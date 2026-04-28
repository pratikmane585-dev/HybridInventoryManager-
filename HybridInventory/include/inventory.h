#ifndef INVENTORY_H
#define INVENTORY_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_LEN 40
#define INVENTORY_FILE "inventory.dat"

/* ── Core data record ────────────────────────────────────────────── */
typedef struct {
    int   id;
    char  name[MAX_NAME_LEN];
    int   quantity;
    float price;
    int   is_deleted;   /* 0 = active, 1 = soft-deleted */
} Item;

/* ── C backend API ───────────────────────────────────────────────── */
/* All return 1 on success, 0 on failure.                            */
/* list_items returns the number of active items copied to buffer.   */

int add_item    (const Item *item);
int get_item    (int id, Item *out);
int update_item (int id, const Item *updated);
int delete_item (int id);
int list_items  (Item *buffer, int max_items);

#ifdef __cplusplus
}
#endif

#endif /* INVENTORY_H */
