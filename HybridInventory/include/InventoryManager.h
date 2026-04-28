#pragma once

#include "inventory.h"   /* Item struct + C API */

#include <vector>

/*
 * InventoryManager
 *
 * C++ class that wraps the C backend.
 * Uses std::vector to hold items for listing and std::sort to order them.
 */
class InventoryManager
{
public:
    enum class SortMode { ById, ByName };

    /* Start the interactive menu loop */
    void run();

private:
    void promptAddItem();
    void promptViewItem();
    void promptUpdateItem();
    void promptDeleteItem();
    void listAllItems(SortMode mode = SortMode::ById);
};
