#ifndef DB_H
#define DB_H

#include <string>
#include <vector>

// Item structure shared across the program
struct Item {
    int id;
    std::string name;
    int quantity;
    double price;
    std::string category;
    std::string expiryDate;
};

// Database initialization / teardown
bool connectDatabase(); // connects to MySQL and auto-creates database/tables
void closeDatabase();

// User-related functions
bool insertUser(const std::string& username, const std::string& password);
bool validateUserCredentials(const std::string& username, const std::string& password);

// Item-related functions
bool insertItem(Item& item);
bool loadItemsFromDatabase(std::vector<Item>& inventory, int& nextId);
bool updateItemQuantityInDatabase(int itemID, int quantity);
bool deleteItemFromDatabase(int itemID);

// Sales-related function
bool insertSale(int userID, int itemID, int quantity, const std::string& saleDate);

#endif // DB_H