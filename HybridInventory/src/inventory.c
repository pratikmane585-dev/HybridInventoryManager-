/*
 * inventory.c  –  C backend for the Hybrid Inventory Manager
 *
 * Storage layout: flat binary file of Item records.
 * Each record sits at offset  (index * sizeof(Item)).
 * fseek lets us jump straight to any record for O(1) update/delete.
 */

#include "inventory.h"

#include <stdio.h>
#include <string.h>

/* ── helpers ─────────────────────────────────────────────────────── */

/* Returns the 0-based index of the record with the given id,
   or -1 if not found (including soft-deleted).                      */
static int find_index(FILE *fp, int id)
{
    Item tmp;
    int  idx = 0;

    rewind(fp);
    while (fread(&tmp, sizeof(Item), 1, fp) == 1) {
        if (tmp.id == id)
            return idx;
        ++idx;
    }
    return -1;
}

/* ── public API ──────────────────────────────────────────────────── */

/*
 * add_item – append a new record.
 * Rejects: non-positive id, empty name, duplicate id.
 * Returns 1 on success, 0 on failure.
 */
int add_item(const Item *item)
{
    if (!item || item->id <= 0 || item->name[0] == '\0')
        return 0;

    /* Check for duplicate id (ignore deleted records too – reuse not
       allowed; keeps file indexing simple).                          */
    FILE *fp = fopen(INVENTORY_FILE, "rb");
    if (fp) {
        if (find_index(fp, item->id) >= 0) {
            fclose(fp);
            return 0;   /* duplicate */
        }
        fclose(fp);
    }

    /* Append */
    fp = fopen(INVENTORY_FILE, "ab");
    if (!fp) return 0;

    int ok = (fwrite(item, sizeof(Item), 1, fp) == 1);
    fclose(fp);
    return ok;
}

/*
 * get_item – read one active record by id into *out.
 * Returns 1 on success, 0 if not found or soft-deleted.
 */
int get_item(int id, Item *out)
{
    if (!out || id <= 0) return 0;

    FILE *fp = fopen(INVENTORY_FILE, "rb");
    if (!fp) return 0;

    int idx = find_index(fp, id);
    if (idx < 0) { fclose(fp); return 0; }

    /* Seek to that record */
    fseek(fp, (long)idx * (long)sizeof(Item), SEEK_SET);
    fread(out, sizeof(Item), 1, fp);
    fclose(fp);

    if (out->is_deleted) return 0;   /* treat as not found */
    return 1;
}

/*
 * update_item – overwrite the record in-place.
 * The id in *updated must match the target id.
 * Returns 1 on success, 0 on failure.
 */
int update_item(int id, const Item *updated)
{
    if (!updated || id <= 0) return 0;

    FILE *fp = fopen(INVENTORY_FILE, "r+b");
    if (!fp) return 0;

    int idx = find_index(fp, id);
    if (idx < 0) { fclose(fp); return 0; }

    /* Seek and overwrite */
    fseek(fp, (long)idx * (long)sizeof(Item), SEEK_SET);
    Item rec = *updated;
    rec.id   = id;          /* keep original id */
    int ok   = (fwrite(&rec, sizeof(Item), 1, fp) == 1);
    fclose(fp);
    return ok;
}

/*
 * delete_item – soft-delete: set is_deleted = 1.
 * Returns 1 on success, 0 on failure.
 */
int delete_item(int id)
{
    if (id <= 0) return 0;

    FILE *fp = fopen(INVENTORY_FILE, "r+b");
    if (!fp) return 0;

    int idx = find_index(fp, id);
    if (idx < 0) { fclose(fp); return 0; }

    fseek(fp, (long)idx * (long)sizeof(Item), SEEK_SET);
    Item rec;
    if (fread(&rec, sizeof(Item), 1, fp) != 1) { fclose(fp); return 0; }
    if (rec.is_deleted) { fclose(fp); return 0; } /* already gone */

    rec.is_deleted = 1;
    fseek(fp, (long)idx * (long)sizeof(Item), SEEK_SET);
    int ok = (fwrite(&rec, sizeof(Item), 1, fp) == 1);
    fclose(fp);
    return ok;
}

/*
 * list_items – copy up to max_items active records into buffer.
 * Returns count of items copied.
 */
int list_items(Item *buffer, int max_items)
{
    if (!buffer || max_items <= 0) return 0;

    FILE *fp = fopen(INVENTORY_FILE, "rb");
    if (!fp) return 0;

    int count = 0;
    Item tmp;
    while (count < max_items && fread(&tmp, sizeof(Item), 1, fp) == 1) {
        if (!tmp.is_deleted)
            buffer[count++] = tmp;
    }
    fclose(fp);
    return count;
}
