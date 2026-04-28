cmake_minimum_required(VERSION 3.14)
project(HybridInventoryManager LANGUAGES C CXX)

# ── Standards ────────────────────────────────────────────────────────────────
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

# ── Compiler warnings ────────────────────────────────────────────────────────
add_compile_options(-Wall -Wextra)

# ── Include directories ──────────────────────────────────────────────────────
include_directories(include)

# ── C library (compiled as C) ────────────────────────────────────────────────
add_library(inventory_c STATIC
    src/inventory.c
)
set_target_properties(inventory_c PROPERTIES LINKER_LANGUAGE C)

# ── Executable (C++ sources + C library) ─────────────────────────────────────
add_executable(inventory_manager
    src/main.cpp
    src/InventoryManager.cpp
)
set_target_properties(inventory_manager PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(inventory_manager PRIVATE inventory_c)
