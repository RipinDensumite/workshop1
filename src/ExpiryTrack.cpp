#include "../include/ExpiryTrack.h"
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

static void printItemHeader() {
    std::cout << "-------------------------------------------------------------\n";
    std::cout << "ID\tName\t\tQty\tPrice\tExpiry Date\tStatus\n";
    std::cout << "-------------------------------------------------------------\n";
}

static void printItemRow(const Item& item, int daysUntilExpiry) {
    std::string status;

    if (daysUntilExpiry < 0) {
        status = "Expired";
    } else if (daysUntilExpiry == 0) {
        status = "Expires today";
    } else {
        status = "In " + std::to_string(daysUntilExpiry) + " day(s)";
    }

    std::cout << item.id << "\t"
              << item.name << "\t\t"
              << item.quantity << "\t"
              << item.price << "\t"
              << item.expiryDate << "\t"
              << status << "\n";
}

static std::vector<Item> loadInventoryItems() {
    std::vector<Item> inventory;
    int nextId = 1;

    if (!loadItemsFromDatabase(inventory, nextId)) {
        std::cout << "Failed to load items from database.\n";
    }

    return inventory;
}

static void viewExpiredItems() {
    clearScreen();

    std::cout << "=== Expired Items ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();
    bool found = false;

    printItemHeader();

    for (const Item& item : inventory) {
        int daysUntilExpiry = getDaysUntilExpiry(item.expiryDate);

        if (daysUntilExpiry < 0 && item.quantity > 0) {
            printItemRow(item, daysUntilExpiry);
            found = true;
        }
    }

    if (!found) {
        std::cout << "No expired items found.\n";
    }

    std::cout << "\n";
    pauseMenu();
}

static void viewExpiringSoonItems() {
    clearScreen();

    int days;

    std::cout << "=== Items Expiring Soon ===\n\n";
    std::cout << "Show items expiring within how many days? ";
    std::cin >> days;

    if (std::cin.fail() || days < 0) {
        clearInput();
        std::cout << "\nInvalid number of days.\n";
        pauseMenu();
        return;
    }

    clearInput();

    std::vector<Item> inventory = loadInventoryItems();
    bool found = false;

    std::cout << "\nItems expiring within " << days << " day(s):\n\n";

    printItemHeader();

    for (const Item& item : inventory) {
        int daysUntilExpiry = getDaysUntilExpiry(item.expiryDate);

        if (daysUntilExpiry >= 0 && daysUntilExpiry <= days && item.quantity > 0) {
            printItemRow(item, daysUntilExpiry);
            found = true;
        }
    }

    if (!found) {
        std::cout << "No items expiring within " << days << " day(s).\n";
    }

    std::cout << "\n";
    pauseMenu();
}

static void viewAllExpiryStatus() {
    clearScreen();

    std::cout << "=== All Item Expiry Status ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();

    if (inventory.empty()) {
        std::cout << "No items found.\n\n";
        pauseMenu();
        return;
    }

    printItemHeader();

    for (const Item& item : inventory) {
        int daysUntilExpiry = getDaysUntilExpiry(item.expiryDate);
        printItemRow(item, daysUntilExpiry);
    }

    std::cout << "\n";
    pauseMenu();
}

static void markExpiredItemAsWaste() {
    clearScreen();

    std::cout << "=== Mark Expired Item As Waste ===\n\n";

    std::vector<Item> inventory = loadInventoryItems();
    bool foundExpired = false;

    printItemHeader();

    for (const Item& item : inventory) {
        int daysUntilExpiry = getDaysUntilExpiry(item.expiryDate);

        if (daysUntilExpiry < 0 && item.quantity > 0) {
            printItemRow(item, daysUntilExpiry);
            foundExpired = true;
        }
    }

    if (!foundExpired) {
        std::cout << "No expired items available to mark as waste.\n\n";
        pauseMenu();
        return;
    }

    int itemID;
    int quantity;

    std::cout << "\nEnter item ID to mark as waste: ";
    std::cin >> itemID;

    if (std::cin.fail()) {
        clearInput();
        std::cout << "\nInvalid item ID.\n";
        pauseMenu();
        return;
    }

    std::cout << "Enter waste quantity: ";
    std::cin >> quantity;

    if (std::cin.fail() || quantity <= 0) {
        clearInput();
        std::cout << "\nInvalid quantity.\n";
        pauseMenu();
        return;
    }

    clearInput();

    bool success = insertWaste(itemID, quantity, "Expired item");

    if (success) {
        std::cout << "\nExpired item recorded as waste successfully.\n";
    } else {
        std::cout << "\nFailed to record waste. Please check item ID or quantity.\n";
    }

    pauseMenu();
}

void showExpiryTrackingMenu() {
    while (true) {
        clearScreen();

        std::cout << "=== Expiry Tracking ===\n\n";
        std::cout << "1. View expired items\n";
        std::cout << "2. View items expiring soon\n";
        std::cout << "3. View all item expiry status\n";
        std::cout << "4. Mark expired item as waste\n";
        std::cout << "5. Back to main menu\n\n";
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
                viewExpiredItems();
                break;

            case 2:
                viewExpiringSoonItems();
                break;

            case 3:
                viewAllExpiryStatus();
                break;

            case 4:
                markExpiredItemAsWaste();
                break;

            case 5:
                return;

            default:
                std::cout << "\nInvalid selection. Please choose between 1 and 5.\n";
                pauseMenu();
                break;
        }
    }
}