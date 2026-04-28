# Hybrid Inventory Manager

A console-based inventory system demonstrating C/C++ interop:

- **C backend** (`inventory.c`) – binary file I/O with `fread`/`fwrite`/`fseek`
- **C++ frontend** (`InventoryManager.cpp`) – menu, input validation, `std::vector`, `std::sort`

Data persists across restarts in `inventory.dat` (binary).

---

## File Structure

```
HybridInventory/
├── include/
│   ├── inventory.h          # C struct + extern "C" API
│   └── InventoryManager.h   # C++ class declaration
├── src/
│   ├── inventory.c          # C backend (file I/O)
│   ├── InventoryManager.cpp # C++ class implementation
│   └── main.cpp             # Entry point
├── Makefile
├── CMakeLists.txt
└── README.md
```

---

## Build & Run

### Option A – Make (recommended)

```bash
# Build
make

# Run
./inventory_manager

# Or build + run in one step
make run

# Clean all build artifacts and inventory.dat
make clean
```

### Option B – CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
./inventory_manager
```

> **Requirements:** `gcc` ≥ 10 and `g++` ≥ 10 (or `clang`/`clang++` with same flags).

---

## Menu Options

| Key | Action |
|-----|--------|
| 1 | Add item |
| 2 | View item by ID |
| 3 | Update item |
| 4 | Delete item (soft) |
| 5 | List all – sorted by ID |
| 6 | List all – sorted by name |
| 7 | Exit |

---

## Input Validation Rules

| Field | Rule |
|-------|------|
| ID | Positive integer; duplicate IDs rejected |
| Name | Non-empty string (max 39 chars) |
| Quantity | Integer ≥ 0 |
| Price | Float ≥ 0.00 |

Invalid input shows a clear error message and re-prompts — no crash.

---

## Test Cases

- **TC1 – Persistence across restarts**  
  Added items with IDs 1, 2, 3. Exited. Re-ran the program. Selected "List all" → all three items appeared with their original data intact.

- **TC2 – Duplicate ID rejection**  
  Tried to add a second item with ID 1 while ID 1 already existed. Program printed `[FAIL] Could not add item (duplicate ID or file error)` and left the original record untouched.

- **TC3 – Update persists after restart**  
  Updated item ID 2's price from $5.00 to $99.99. Exited. Re-launched. Used "View item" on ID 2 → price showed $99.99.

- **TC4 – Soft delete hides item**  
  Deleted item ID 3. Selected "List all" → ID 3 did not appear. Used "View item" on ID 3 → showed `[FAIL] Item not found or has been deleted`. Re-ran program → item still absent.

- **TC5 – Invalid input recovery**  
  Entered `-5` for ID → error message appeared, re-prompted. Entered `0` for quantity → accepted. Entered `-1.5` for price → error message appeared, re-prompted. Entered empty string for name → error message appeared, re-prompted. Program never crashed.

---

## Design Notes

- **Soft delete**: `is_deleted = 1` keeps the binary file layout fixed (no shifting records). `get_item` and `list_items` filter these out automatically.
- **fseek indexing**: Record `i` is always at byte offset `i × sizeof(Item)`, so update and delete are O(1) file seeks.
- **`extern "C"`**: The header wraps all declarations in `extern "C"` so the C++ linker resolves the C symbols without name-mangling.
- **STL usage**: `std::vector<Item>` holds the active snapshot for display; `std::sort` with a lambda provides dual sort modes (by ID or by name).
