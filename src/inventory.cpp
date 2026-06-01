#include "../include/screen.h"
#include "../include/inventory.h"
#include "../include/db.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <thread>

using namespace std;

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool isValidDate(const string& date) {
    if (date.length() != 10 || date[4] != '-' || date[7] != '-') return false;
    int year, month, day;
    char dash1, dash2;
    stringstream ss(date);
    ss >> year >> dash1 >> month >> dash2 >> day;
    if (ss.fail() || dash1 != '-' || dash2 != '-') return false;
    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) daysInMonth[1] = 29;
    if (day < 1 || day > daysInMonth[month - 1]) return false;
    return true;
}

string getCategory() {
    while (true) {
        cout << "Category:\n";
        cout << "a. Produce (fruits & vegies)\n";
        cout << "b. Meat & Poultry\n";
        cout << "c. Fish & Seafood\n";
        cout << "d. Dairy & Eggs\n";
        cout << "e. Bakery\n";
        cout << "Choose category (a-e): ";
        string choice;
        getline(cin, choice);
        if (choice == "a") return "Produce (fruits & vegies)";
        if (choice == "b") return "Meat & Poultry";
        if (choice == "c") return "Fish & Seafood";
        if (choice == "d") return "Dairy & Eggs";
        if (choice == "e") return "Bakery";
        cout << "Invalid choice. Please select a, b, c, d, or e.\n";
    }
}

void showMenu() {
    clearScreen();
    cout << "=== Inventory Management Menu ===\n";
    cout << "1. Add item\n";
    cout << "2. List items\n";
    cout << "3. Update quantity\n";
    cout << "4. Remove item\n";
    cout << "5. Search item\n";
    cout << "6. Back to main menu\n";
    cout << "7. Exit\n";
    cout << "Choose an option: ";
}

int findItemIndex(const vector<Item>& inventory, int id) {
    for (size_t i = 0; i < inventory.size(); ++i) {
        if (inventory[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int findItemIndexByName(const vector<Item>& inventory, const string& name) {
    for (size_t i = 0; i < inventory.size(); ++i) {
        if (inventory[i].name == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void listItems(const vector<Item>& inventory) {
    clearScreen();
    cout << "=== List of Items ===\n\n";
    
    if (inventory.empty()) {
        cout << "Inventory is empty.\n";
    } else {
        cout << left
            << setw(5)  << "ID"
            << setw(20) << "Name"
            << setw(8)  << "Qty"
            << setw(12) << "Price (RM)"
            << setw(20) << "Category"
            << "Expiry Date\n";

        cout << string(75, '-') << "\n";

        for (const auto& item : inventory) {
            stringstream ss;
            ss << "RM " << fixed << setprecision(2) << item.price;

            cout << left
                << setw(5)  << item.id
                << setw(20) << item.name
                << setw(8)  << item.quantity
                << setw(12) << ss.str()
                << setw(20) << item.category
                << item.expiryDate
                << "\n";
        }
    }
    
    pauseMenu();
}

void addItem(vector<Item>& inventory, int& nextId) {
    clearScreen();
    cout << "=== Add New Item ===\n\n";
    
    string name;
    int quantity;
    double price;

    cout << "Enter item name: ";
    getline(cin, name);

    if (name.empty()) {
        cout << "Name cannot be empty.\n";
        pauseMenu();
        return;
    }

    if (findItemIndexByName(inventory, name) != -1) {
        cout << "Item already exists in inventory.\n";
        pauseMenu();
        return;
    }

    cout << "Enter quantity: ";
    if (!(cin >> quantity) || quantity < 0) {
        cout << "Invalid quantity.\n";
        clearInput();
        pauseMenu();
        return;
    }

    cout << "Enter price: ";
    if (!(cin >> price) || price < 0) {
        cout << "Invalid price.\n";
        clearInput();
        pauseMenu();
        return;
    }

    clearInput();

    string category = getCategory();

    string expiryDate;
    while (true) {
        cout << "Enter expiry date (YYYY-MM-DD): ";
        getline(cin, expiryDate);
        if (isValidDate(expiryDate)) break;
        cout << "Invalid date format. Please use YYYY-MM-DD.\n";
    }

    Item item{0, name, quantity, price, category, expiryDate};

    if (insertItem(item)) {
        nextId = max(nextId, item.id + 1);
        cout << "\nItem added to storage successfully.\n";
    } else {
        item.id = nextId++;
        cout << "\nItem added locally (storage insert failed).\n";
    }
    inventory.push_back(item);
    pauseMenu();
}

void updateQuantity(vector<Item>& inventory) {
    clearScreen();
    cout << "=== Update Quantity ===\n\n";
    
    if (inventory.empty()) {
        cout << "Inventory is empty.\n";
        pauseMenu();
        return;
    }

    int id;
    cout << "Enter item ID to update: ";

    if (!(cin >> id)) {
        cout << "Invalid ID.\n";
        clearInput();
        pauseMenu();
        return;
    }

    int index = findItemIndex(inventory, id);

    if (index == -1) {
        cout << "Item not found.\n";
        clearInput();
        pauseMenu();
        return;
    }

    int newQuantity;
    cout << "Enter new quantity: ";

    if (!(cin >> newQuantity) || newQuantity < 0) {
        cout << "Invalid quantity.\n";
        clearInput();
        pauseMenu();
        return;
    }

    inventory[index].quantity = newQuantity;
    if (updateItemQuantityInDatabase(inventory[index].id, newQuantity)) {
        cout << "\nQuantity updated in storage successfully.\n";
    } else {
        cout << "\nQuantity updated locally (storage update failed).\n";
    }

    clearInput();
    pauseMenu();
}

void removeItem(vector<Item>& inventory) {
    clearScreen();
    cout << "=== Remove Item ===\n\n";
    
    if (inventory.empty()) {
        cout << "Inventory is empty.\n";
        pauseMenu();
        return;
    }

    int id;
    cout << "Enter item ID to remove: ";

    if (!(cin >> id)) {
        cout << "Invalid ID.\n";
        clearInput();
        pauseMenu();
        return;
    }

    int index = findItemIndex(inventory, id);

    if (index == -1) {
        cout << "Item not found.\n";
        clearInput();
        pauseMenu();
        return;
    }

    int itemId = inventory[index].id;
    inventory.erase(inventory.begin() + index);

    if (deleteItemFromDatabase(itemId)) {
        cout << "\nItem removed from storage successfully.\n";
    } else {
        cout << "\nItem removed locally (storage delete failed).\n";
    }

    clearInput();
    pauseMenu();
}

void searchItem(const vector<Item>& inventory) {
    clearScreen();
    cout << "=== Search Item ===\n\n";
    
    if (inventory.empty()) {
        cout << "Inventory is empty.\n";
    } else {
        string name;
        cout << "Enter item name to search: ";
        getline(cin, name);

        int index = findItemIndexByName(inventory, name);

        if (index == -1) {
            cout << "\nItem not found.\n";
        } else {
            const Item& item = inventory[index];

            cout << "\nFound item:\n";
            cout << "ID: " << item.id << "\n";
            cout << "Name: " << item.name << "\n";
            cout << "Quantity: " << item.quantity << "\n";
            cout << "Price: RM " << fixed << setprecision(2) << item.price << "\n";
            cout << "Category: " << item.category << "\n";
            cout << "Expiry Date: " << item.expiryDate << "\n";
        }
    }
    
    pauseMenu();
}

static void showTemporaryMessage(const std::string& message) {
    std::cout << message << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void runInventoryModule() {
    std::vector<Item> inventory;
    int nextId = 1;
    int choice = 0;

    if (connectDatabase()) {
        if (!loadItemsFromDatabase(inventory, nextId)) {
            std::cout << "Warning: Could not load items from storage. Inventory will start empty.\n";
            pauseMenu();
        }
    } else {
        std::cout << "Warning: Storage initialization failed. Inventory will start empty.\n";
        pauseMenu();
    }

    while (true) {
        showMenu();

        if (!(std::cin >> choice)) {
            clearInput();
            showTemporaryMessage("Invalid selection. Please enter a number.");
            continue;
        }

        clearInput();

        switch (choice) {
            case 1:
                addItem(inventory, nextId);
                break;
            case 2:
                listItems(inventory);
                break;
            case 3:
                updateQuantity(inventory);
                break;
            case 4:
                removeItem(inventory);
                break;
            case 5:
                searchItem(inventory);
                break;
            case 6:
                closeDatabase();
                return; // Back to main menu
            case 7:
                closeDatabase();
                std::cout << "Goodbye!\n";
                std::exit(0);
            default:
                showTemporaryMessage("Please choose a valid option (1-7). Please wait...");
                break;
        }
    }
}
