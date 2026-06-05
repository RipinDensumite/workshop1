#include "../include/Report.h"
#include "../include/db.h"
#include "../include/screen.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
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

static bool parseDate(const std::string& dateText, std::tm& date) {
    std::istringstream ss(dateText);
    ss >> std::get_time(&date, "%Y-%m-%d");

    if (ss.fail()) {
        return false;
    }

    date.tm_hour = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    date.tm_isdst = -1;

    return true;
}

static std::tm getTodayDate() {
    std::time_t now = std::time(nullptr);
    std::tm today{};

#ifdef _WIN32
    localtime_s(&today, &now);
#else
    localtime_r(&now, &today);
#endif

    today.tm_hour = 0;
    today.tm_min = 0;
    today.tm_sec = 0;
    today.tm_isdst = -1;

    return today;
}

static int getDaysUntilExpiry(const std::string& expiryDate) {
    std::tm expiry{};
    std::tm today = getTodayDate();

    if (!parseDate(expiryDate, expiry)) {
        return 999999;
    }

    std::time_t expiryTime = std::mktime(&expiry);
    std::time_t todayTime = std::mktime(&today);

    double secondsDifference = std::difftime(expiryTime, todayTime);
    return static_cast<int>(secondsDifference / (60 * 60 * 24));
}

static void showInventorySummaryReport() {
    clearScreen();

    std::cout << "=== Inventory Summary Report ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();

    if (inventory.empty()) {
        std::cout << "No inventory records found.\n\n";
        pauseMenu();
        return;
    }

    int totalItems = 0;
    int totalQuantity = 0;
    int lowStockCount = 0;
    int outOfStockCount = 0;
    int expiredCount = 0;
    double totalStockValue = 0.0;

    for (const Item& item : inventory) {
        totalItems++;
        totalQuantity += item.quantity;
        totalStockValue += item.quantity * item.price;

        if (item.quantity == 0) {
            outOfStockCount++;
        } else if (item.quantity <= 5) {
            lowStockCount++;
        }

        if (getDaysUntilExpiry(item.expiryDate) < 0) {
            expiredCount++;
        }
    }

    std::cout << "Total item types      : " << totalItems << "\n";
    std::cout << "Total stock quantity  : " << totalQuantity << "\n";
    std::cout << "Low stock items       : " << lowStockCount << "\n";
    std::cout << "Out of stock items    : " << outOfStockCount << "\n";
    std::cout << "Expired item types    : " << expiredCount << "\n";
    std::cout << "Total stock value     : RM "
              << std::fixed << std::setprecision(2)
              << totalStockValue << "\n\n";

    pauseMenu();
}

static void showStockValueReport() {
    clearScreen();

    std::cout << "=== Stock Value Report ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();

    if (inventory.empty()) {
        std::cout << "No inventory records found.\n\n";
        pauseMenu();
        return;
    }

    double grandTotal = 0.0;

    std::cout << "-------------------------------------------------------------------------------\n";
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Name"
              << std::setw(16) << "Category"
              << std::setw(10) << "Qty"
              << std::setw(12) << "Price"
              << "Value\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    for (const Item& item : inventory) {
        double itemValue = item.quantity * item.price;
        grandTotal += itemValue;

        std::cout << std::left
                  << std::setw(6)  << item.id
                  << std::setw(22) << item.name
                  << std::setw(16) << item.category
                  << std::setw(10) << item.quantity
                  << std::setw(12) << std::fixed << std::setprecision(2) << item.price
                  << std::fixed << std::setprecision(2) << itemValue
                  << "\n";
    }

    std::cout << "-------------------------------------------------------------------------------\n";
    std::cout << "Grand total stock value: RM "
              << std::fixed << std::setprecision(2)
              << grandTotal << "\n\n";

    pauseMenu();
}

static void showLowStockReport() {
    clearScreen();

    std::cout << "=== Low Stock Report ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();
    bool found = false;

    std::cout << "Low stock threshold: quantity <= 5\n\n";

    std::cout << "-------------------------------------------------------------\n";
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Name"
              << std::setw(16) << "Category"
              << "Quantity\n";
    std::cout << "-------------------------------------------------------------\n";

    for (const Item& item : inventory) {
        if (item.quantity > 0 && item.quantity <= 5) {
            std::cout << std::left
                      << std::setw(6)  << item.id
                      << std::setw(22) << item.name
                      << std::setw(16) << item.category
                      << item.quantity << "\n";

            found = true;
        }
    }

    if (!found) {
        std::cout << "No low stock items found.\n";
    }

    std::cout << "\n";
    pauseMenu();
}

static void showExpiredItemsReport() {
    clearScreen();

    std::cout << "=== Expired Items Report ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();
    bool found = false;

    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Name"
              << std::setw(16) << "Category"
              << std::setw(10) << "Qty"
              << "Expiry Date\n";
    std::cout << "-----------------------------------------------------------------------\n";

    for (const Item& item : inventory) {
        if (getDaysUntilExpiry(item.expiryDate) < 0) {
            std::cout << std::left
                      << std::setw(6)  << item.id
                      << std::setw(22) << item.name
                      << std::setw(16) << item.category
                      << std::setw(10) << item.quantity
                      << item.expiryDate << "\n";

            found = true;
        }
    }

    if (!found) {
        std::cout << "No expired items found.\n";
    }

    std::cout << "\n";
    pauseMenu();
}

static void showSalesReport() {
    clearScreen();

    std::cout << "=== Sales Report ===\n\n";

    std::vector<SalesReport> sales;

    if (!loadSalesReportFromDatabase(sales)) {
        std::cout << "Failed to load sales report.\n\n";
        pauseMenu();
        return;
    }

    if (sales.empty()) {
        std::cout << "No sales records found.\n\n";
        pauseMenu();
        return;
    }

    double totalSalesAmount = 0.0;
    int totalQuantitySold = 0;

    std::cout << "----------------------------------------------------------------------------------------------------------\n";
    std::cout << std::left
              << std::setw(8)  << "SaleID"
              << std::setw(16) << "User"
              << std::setw(22) << "Item"
              << std::setw(8)  << "Qty"
              << std::setw(12) << "Price"
              << std::setw(12) << "Total"
              << std::setw(14) << "Date"
              << "Time\n";
    std::cout << "----------------------------------------------------------------------------------------------------------\n";

    for (const SalesReport& sale : sales) {
        totalSalesAmount += sale.totalAmount;
        totalQuantitySold += sale.quantitySold;

        std::cout << std::left
                  << std::setw(8)  << sale.saleID
                  << std::setw(16) << sale.username
                  << std::setw(22) << sale.itemName
                  << std::setw(8)  << sale.quantitySold
                  << std::setw(12) << std::fixed << std::setprecision(2) << sale.price
                  << std::setw(12) << std::fixed << std::setprecision(2) << sale.totalAmount
                  << std::setw(14) << sale.saleDate
                  << sale.saleTime << "\n";
    }

    std::cout << "----------------------------------------------------------------------------------------------------------\n";
    std::cout << "Total quantity sold : " << totalQuantitySold << "\n";
    std::cout << "Total sales amount  : RM "
              << std::fixed << std::setprecision(2)
              << totalSalesAmount << "\n\n";

    pauseMenu();
}

static void showWasteReport() {
    clearScreen();

    std::cout << "=== Waste Report ===\n\n";

    std::vector<WasteReport> wastes;

    if (!loadWasteReportFromDatabase(wastes)) {
        std::cout << "Failed to load waste report.\n\n";
        pauseMenu();
        return;
    }

    if (wastes.empty()) {
        std::cout << "No waste records found.\n\n";
        pauseMenu();
        return;
    }

    int totalWasteQuantity = 0;

    std::cout << "------------------------------------------------------------------------------------------\n";
    std::cout << std::left
              << std::setw(9)  << "WasteID"
              << std::setw(24) << "Item"
              << std::setw(10) << "Qty"
              << std::setw(24) << "Reason"
              << "Date\n";
    std::cout << "------------------------------------------------------------------------------------------\n";

    for (const WasteReport& waste : wastes) {
        totalWasteQuantity += waste.quantity;

        std::cout << std::left
                  << std::setw(9)  << waste.wasteID
                  << std::setw(24) << waste.itemName
                  << std::setw(10) << waste.quantity
                  << std::setw(24) << waste.reason
                  << waste.wasteDate << "\n";
    }

    std::cout << "------------------------------------------------------------------------------------------\n";
    std::cout << "Total wasted quantity: " << totalWasteQuantity << "\n\n";

    pauseMenu();
}

void showReportMenu() {
    while (true) {
        clearScreen();

        std::cout << "=== Reports ===\n\n";
        std::cout << "1. Inventory summary report\n";
        std::cout << "2. Stock value report\n";
        std::cout << "3. Low stock report\n";
        std::cout << "4. Expired items report\n";
        std::cout << "5. Sales report\n";
        std::cout << "6. Waste report\n";
        std::cout << "7. Back to main menu\n\n";
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
                showInventorySummaryReport();
                break;

            case 2:
                showStockValueReport();
                break;

            case 3:
                showLowStockReport();
                break;

            case 4:
                showExpiredItemsReport();
                break;

            case 5:
                showSalesReport();
                break;

            case 6:
                showWasteReport();
                break;

            case 7:
                return;

            default:
                std::cout << "\nInvalid selection. Please choose between 1 and 7.\n";
                pauseMenu();
                break;
        }
    }
}