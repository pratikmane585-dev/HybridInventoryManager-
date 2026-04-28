/*
 * InventoryManager.cpp  –  C++ layer for the Hybrid Inventory Manager
 *
 * Wraps the C backend with a class, uses std::vector + std::sort,
 * and provides validated console I/O helpers.
 */

#include "InventoryManager.h"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

/* ════════════════════════════════════════════════════════════════════
   Input helpers
   ════════════════════════════════════════════════════════════════════ */

namespace {

/* Flush bad input and show message */
void clear_cin()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int read_positive_int(const std::string &prompt)
{
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v > 0) { clear_cin(); return v; }
        std::cout << "  [!] Must be a positive integer. Try again.\n";
        clear_cin();
    }
}

int read_non_negative_int(const std::string &prompt)
{
    int v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0) { clear_cin(); return v; }
        std::cout << "  [!] Must be 0 or greater. Try again.\n";
        clear_cin();
    }
}

float read_non_negative_float(const std::string &prompt)
{
    float v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0.0f) { clear_cin(); return v; }
        std::cout << "  [!] Must be 0.00 or greater. Try again.\n";
        clear_cin();
    }
}

std::string read_non_empty_string(const std::string &prompt)
{
    std::string s;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, s);
        if (!s.empty()) return s;
        std::cout << "  [!] Name cannot be empty. Try again.\n";
    }
}

/* Pretty table row */
void print_header()
{
    std::cout << "\n"
              << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Name"
              << std::setw(10) << "Qty"
              << std::setw(10) << "Price"
              << "\n"
              << std::string(48, '-') << "\n";
}

void print_item(const Item &it)
{
    std::cout << std::left
              << std::setw(6)  << it.id
              << std::setw(22) << it.name
              << std::setw(10) << it.quantity
              << "$" << std::fixed << std::setprecision(2) << it.price
              << "\n";
}

} // anonymous namespace

/* ════════════════════════════════════════════════════════════════════
   InventoryManager  –  public methods
   ════════════════════════════════════════════════════════════════════ */

void InventoryManager::promptAddItem()
{
    std::cout << "\n── Add Item ──\n";
    Item it{};

    it.id       = read_positive_int("  ID        : ");
    std::string nm = read_non_empty_string("  Name      : ");
    std::strncpy(it.name, nm.c_str(), MAX_NAME_LEN - 1);
    it.name[MAX_NAME_LEN - 1] = '\0';
    it.quantity = read_non_negative_int("  Quantity  : ");
    it.price    = read_non_negative_float("  Price     : ");
    it.is_deleted = 0;

    if (add_item(&it))
        std::cout << "  [OK] Item " << it.id << " added.\n";
    else
        std::cout << "  [FAIL] Could not add item (duplicate ID or file error).\n";
}

void InventoryManager::promptViewItem()
{
    std::cout << "\n── View Item ──\n";
    int id = read_positive_int("  Enter ID  : ");

    Item it{};
    if (get_item(id, &it)) {
        print_header();
        print_item(it);
    } else {
        std::cout << "  [FAIL] Item " << id << " not found or has been deleted.\n";
    }
}

void InventoryManager::promptUpdateItem()
{
    std::cout << "\n── Update Item ──\n";
    int id = read_positive_int("  Enter ID to update : ");

    Item existing{};
    if (!get_item(id, &existing)) {
        std::cout << "  [FAIL] Item " << id << " not found or deleted.\n";
        return;
    }

    std::cout << "  Current → name=\"" << existing.name
              << "\"  qty=" << existing.quantity
              << "  price=$" << std::fixed << std::setprecision(2) << existing.price
              << "\n  (Press Enter to keep current value)\n";

    /* Name */
    std::cout << "  New name [" << existing.name << "] : ";
    std::string nm;
    std::getline(std::cin, nm);
    if (!nm.empty())
        std::strncpy(existing.name, nm.c_str(), MAX_NAME_LEN - 1);

    /* Quantity – allow empty to keep */
    std::cout << "  New qty  [" << existing.quantity << "] : ";
    std::string qstr;
    std::getline(std::cin, qstr);
    if (!qstr.empty()) {
        try {
            int q = std::stoi(qstr);
            if (q < 0) std::cout << "  [!] Qty cannot be negative, keeping old.\n";
            else existing.quantity = q;
        } catch (...) {
            std::cout << "  [!] Invalid qty, keeping old.\n";
        }
    }

    /* Price – allow empty to keep */
    std::cout << "  New price [" << std::fixed << std::setprecision(2)
              << existing.price << "] : ";
    std::string pstr;
    std::getline(std::cin, pstr);
    if (!pstr.empty()) {
        try {
            float p = std::stof(pstr);
            if (p < 0.0f) std::cout << "  [!] Price cannot be negative, keeping old.\n";
            else existing.price = p;
        } catch (...) {
            std::cout << "  [!] Invalid price, keeping old.\n";
        }
    }

    if (update_item(id, &existing))
        std::cout << "  [OK] Item " << id << " updated.\n";
    else
        std::cout << "  [FAIL] Update failed.\n";
}

void InventoryManager::promptDeleteItem()
{
    std::cout << "\n── Delete Item ──\n";
    int id = read_positive_int("  Enter ID to delete : ");

    if (delete_item(id))
        std::cout << "  [OK] Item " << id << " deleted.\n";
    else
        std::cout << "  [FAIL] Item " << id << " not found, already deleted, or file error.\n";
}

void InventoryManager::listAllItems(SortMode mode)
{
    static const int MAX = 1024;
    Item buf[MAX];
    int  count = list_items(buf, MAX);

    if (count == 0) {
        std::cout << "\n  (No active items in inventory.)\n";
        return;
    }

    /* STL 1: load into vector */
    std::vector<Item> items(buf, buf + count);

    /* STL 2: sort */
    if (mode == SortMode::ByName) {
        std::sort(items.begin(), items.end(),
                  [](const Item &a, const Item &b) {
                      return std::strcmp(a.name, b.name) < 0;
                  });
    } else {
        std::sort(items.begin(), items.end(),
                  [](const Item &a, const Item &b) { return a.id < b.id; });
    }

    std::cout << "\n── All Items (" << count << ") ──";
    print_header();
    for (const auto &it : items)
        print_item(it);
}

/* ════════════════════════════════════════════════════════════════════
   Menu
   ════════════════════════════════════════════════════════════════════ */

void InventoryManager::run()
{
    std::cout << "╔══════════════════════════════════╗\n"
              << "║   Hybrid Inventory Manager v1.0  ║\n"
              << "╚══════════════════════════════════╝\n";

    int choice = 0;
    while (true) {
        std::cout << "\n  1) Add item\n"
                  << "  2) View item\n"
                  << "  3) Update item\n"
                  << "  4) Delete item\n"
                  << "  5) List all (sort by ID)\n"
                  << "  6) List all (sort by name)\n"
                  << "  7) Exit\n"
                  << "  > ";

        if (!(std::cin >> choice)) { clear_cin(); continue; }
        clear_cin();

        switch (choice) {
            case 1: promptAddItem();                  break;
            case 2: promptViewItem();                 break;
            case 3: promptUpdateItem();               break;
            case 4: promptDeleteItem();               break;
            case 5: listAllItems(SortMode::ById);     break;
            case 6: listAllItems(SortMode::ByName);   break;
            case 7:
                std::cout << "\n  Goodbye!\n";
                return;
            default:
                std::cout << "  [!] Invalid choice. Enter 1-7.\n";
        }
    }
}
