#include "../include/StockMonitor.h"
#include "../include/db.h"
#include "../include/screen.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static std::vector<Item> loadInventoryItems() {
    std::vector<Item> inventory;
    int nextId = 1;

    if (!loadItemsFromDatabase(inventory, nextId)) {
        std::cout << "Failed to load items from database.\n";
    }

    return inventory;
}

static void printStockHeader() {
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Name"
              << std::setw(16) << "Category"
              << std::setw(10) << "Qty"
              << std::setw(12) << "Price"
              << std::setw(12) << "Value"
              << "Status\n";
    std::cout << "--------------------------------------------------------------------------------\n";
}

static std::string getStockStatus(int quantity) {
    if (quantity == 0) {
        return "Out of stock";
    }

    if (quantity <= 5) {
        return "Low stock";
    }

    return "Available";
}

static void printStockRow(const Item& item) {
    double totalValue = item.quantity * item.price;

    std::cout << std::left
              << std::setw(6)  << item.id
              << std::setw(22) << item.name
              << std::setw(16) << item.category
              << std::setw(10) << item.quantity
              << std::setw(12) << std::fixed << std::setprecision(2) << item.price
              << std::setw(12) << std::fixed << std::setprecision(2) << totalValue
              << getStockStatus(item.quantity)
              << "\n";
}

static void viewAllStock() {
    clearScreen();

    std::cout << "=== All Stock Items ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();

    if (inventory.empty()) {
        std::cout << "No stock items found.\n\n";
        pauseMenu();
        return;
    }

    printStockHeader();

    for (const Item& item : inventory) {
        printStockRow(item);
    }

    std::cout << "\n";
    pauseMenu();
}

static void viewLowStockItems() {
    clearScreen();

    int threshold;

    std::cout << "=== Low Stock Items ===\n\n";
    std::cout << "Enter low stock threshold: ";
    std::cin >> threshold;

    if (std::cin.fail() || threshold < 0) {
        clearInput();
        std::cout << "\nInvalid threshold.\n";
        pauseMenu();
        return;
    }

    clearInput();

    std::vector<Item> inventory = loadInventoryItems();
    bool found = false;

    std::cout << "\nItems with quantity less than or equal to " << threshold << ":\n\n";

    printStockHeader();

    for (const Item& item : inventory) {
        if (item.quantity > 0 && item.quantity <= threshold) {
            printStockRow(item);
            found = true;
        }
    }

    if (!found) {
        std::cout << "No low stock items found.\n";
    }

    std::cout << "\n";
    pauseMenu();
}

static void viewOutOfStockItems() {
    clearScreen();

    std::cout << "=== Out Of Stock Items ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();
    bool found = false;

    printStockHeader();

    for (const Item& item : inventory) {
        if (item.quantity == 0) {
            printStockRow(item);
            found = true;
        }
    }

    if (!found) {
        std::cout << "No out of stock items found.\n";
    }

    std::cout << "\n";
    pauseMenu();
}

static void viewStockValueReport() {
    clearScreen();

    std::cout << "=== Stock Value Report ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();

    if (inventory.empty()) {
        std::cout << "No stock items found.\n\n";
        pauseMenu();
        return;
    }

    double grandTotal = 0.0;

    printStockHeader();

    for (const Item& item : inventory) {
        printStockRow(item);
        grandTotal += item.quantity * item.price;
    }

    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << "Total stock value: RM "
              << std::fixed << std::setprecision(2)
              << grandTotal << "\n\n";

    pauseMenu();
}

static bool itemExists(const std::vector<Item>& inventory, int itemID, Item& foundItem) {
    for (const Item& item : inventory) {
        if (item.id == itemID) {
            foundItem = item;
            return true;
        }
    }

    return false;
}

static void updateStockQuantity() {
    clearScreen();

    std::cout << "=== Update Stock Quantity ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();

    if (inventory.empty()) {
        std::cout << "No stock items found.\n\n";
        pauseMenu();
        return;
    }

    printStockHeader();

    for (const Item& item : inventory) {
        printStockRow(item);
    }

    int itemID;
    int newQuantity;

    std::cout << "\nEnter item ID: ";
    std::cin >> itemID;

    if (std::cin.fail()) {
        clearInput();
        std::cout << "\nInvalid item ID.\n";
        pauseMenu();
        return;
    }

    Item selectedItem;

    if (!itemExists(inventory, itemID, selectedItem)) {
        clearInput();
        std::cout << "\nItem not found.\n";
        pauseMenu();
        return;
    }

    std::cout << "Current quantity: " << selectedItem.quantity << "\n";
    std::cout << "Enter new quantity: ";
    std::cin >> newQuantity;

    if (std::cin.fail() || newQuantity < 0) {
        clearInput();
        std::cout << "\nInvalid quantity.\n";
        pauseMenu();
        return;
    }

    clearInput();

    if (updateItemQuantityInDatabase(itemID, newQuantity)) {
        std::cout << "\nStock quantity updated successfully.\n";
    } else {
        std::cout << "\nFailed to update stock quantity.\n";
    }

    pauseMenu();
}

static void addStockQuantity() {
    clearScreen();

    std::cout << "=== Add / Restock Quantity ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();

    if (inventory.empty()) {
        std::cout << "No stock items found.\n\n";
        pauseMenu();
        return;
    }

    printStockHeader();

    for (const Item& item : inventory) {
        printStockRow(item);
    }

    int itemID;
    int addQuantity;

    std::cout << "\nEnter item ID: ";
    std::cin >> itemID;

    if (std::cin.fail()) {
        clearInput();
        std::cout << "\nInvalid item ID.\n";
        pauseMenu();
        return;
    }

    Item selectedItem;

    if (!itemExists(inventory, itemID, selectedItem)) {
        clearInput();
        std::cout << "\nItem not found.\n";
        pauseMenu();
        return;
    }

    std::cout << "Current quantity: " << selectedItem.quantity << "\n";
    std::cout << "Enter quantity to add: ";
    std::cin >> addQuantity;

    if (std::cin.fail() || addQuantity <= 0) {
        clearInput();
        std::cout << "\nInvalid quantity.\n";
        pauseMenu();
        return;
    }

    clearInput();

    int newQuantity = selectedItem.quantity + addQuantity;

    if (updateItemQuantityInDatabase(itemID, newQuantity)) {
        std::cout << "\nStock added successfully.\n";
        std::cout << "New quantity: " << newQuantity << "\n";
    } else {
        std::cout << "\nFailed to add stock.\n";
    }

    pauseMenu();
}

static void removeDamagedStock() {
    clearScreen();

    std::cout << "=== Remove Damaged / Lost Stock ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();

    if (inventory.empty()) {
        std::cout << "No stock items found.\n\n";
        pauseMenu();
        return;
    }

    printStockHeader();

    for (const Item& item : inventory) {
        if (item.quantity > 0) {
            printStockRow(item);
        }
    }

    int itemID;
    int removeQuantity;

    std::cout << "\nEnter item ID: ";
    std::cin >> itemID;

    if (std::cin.fail()) {
        clearInput();
        std::cout << "\nInvalid item ID.\n";
        pauseMenu();
        return;
    }

    Item selectedItem;

    if (!itemExists(inventory, itemID, selectedItem)) {
        clearInput();
        std::cout << "\nItem not found.\n";
        pauseMenu();
        return;
    }

    if (selectedItem.quantity <= 0) {
        clearInput();
        std::cout << "\nThis item has no stock to remove.\n";
        pauseMenu();
        return;
    }

    std::cout << "Current quantity: " << selectedItem.quantity << "\n";
    std::cout << "Enter quantity to remove: ";
    std::cin >> removeQuantity;

    if (std::cin.fail() || removeQuantity <= 0) {
        clearInput();
        std::cout << "\nInvalid quantity.\n";
        pauseMenu();
        return;
    }

    clearInput();

    if (removeQuantity > selectedItem.quantity) {
        std::cout << "\nCannot remove more than current stock.\n";
        pauseMenu();
        return;
    }

    if (insertWaste(itemID, removeQuantity, "Damaged or lost stock")) {
        std::cout << "\nStock removed and recorded as waste successfully.\n";
    } else {
        std::cout << "\nFailed to remove stock.\n";
    }

    pauseMenu();
}

void showStockMonitoringMenu() {
    while (true) {
        clearScreen();

        std::cout << "=== Stock Monitoring ===\n\n";
        std::cout << "1. View all stock\n";
        std::cout << "2. View low stock items\n";
        std::cout << "3. View out of stock items\n";
        std::cout << "4. View stock value report\n";
        std::cout << "5. Update stock quantity\n";
        std::cout << "6. Add / restock quantity\n";
        std::cout << "7. Remove damaged / lost stock\n";
        std::cout << "8. Back to main menu\n\n";
        std::cout << "Choose an option: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            clearInput();
            std::cout << "\nInvalid selection. Please enter a number.\n";
            pauseMenu();
            continue;
        }

        clearInput();

        switch (choice) {
            case 1:
                viewAllStock();
                break;

            case 2:
                viewLowStockItems();
                break;

            case 3:
                viewOutOfStockItems();
                break;

            case 4:
                viewStockValueReport();
                break;

            case 5:
                updateStockQuantity();
                break;

            case 6:
                addStockQuantity();
                break;

            case 7:
                removeDamagedStock();
                break;

            case 8:
                return;

            default:
                std::cout << "\nInvalid selection. Please choose between 1 and 8.\n";
                pauseMenu();
                break;
        }
    }
}