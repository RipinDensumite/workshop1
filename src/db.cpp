#include "../include/db.h"
#include <algorithm>
#include <iostream>
#include <mutex>
#include <vector>
#include <string>

static std::vector<Item> g_inventory;
static int g_nextId = 1;
static std::vector<std::pair<std::string, std::string>> g_users;
static std::mutex g_mutex;

bool connectDatabase() {
    std::lock_guard<std::mutex> lk(g_mutex);
    if (g_users.empty()) {
        // default admin user
        g_users.emplace_back("admin", "arif");
    }
    return true;
}

void closeDatabase() {
    // nothing to free for in-memory storage
}

bool insertUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lk(g_mutex);
    for (auto &u : g_users) {
        if (u.first == username) return false;
    }
    g_users.emplace_back(username, password);
    return true;
}

bool validateUserCredentials(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lk(g_mutex);
    for (auto &u : g_users) {
        if (u.first == username && u.second == password) return true;
    }
    return false;
}

bool insertItem(Item& item) {
    std::lock_guard<std::mutex> lk(g_mutex);
    item.id = g_nextId++;
    g_inventory.push_back(item);
    return true;
}

bool loadItemsFromDatabase(std::vector<Item>& inventory, int& nextId) {
    std::lock_guard<std::mutex> lk(g_mutex);
    inventory = g_inventory;
    nextId = g_nextId;
    return true;
}

bool updateItemQuantityInDatabase(int itemID, int quantity) {
    std::lock_guard<std::mutex> lk(g_mutex);
    auto it = std::find_if(g_inventory.begin(), g_inventory.end(), [&](const Item& it){ return it.id == itemID; });
    if (it == g_inventory.end()) return false;
    it->quantity = quantity;
    return true;
}

bool deleteItemFromDatabase(int itemID) {
    std::lock_guard<std::mutex> lk(g_mutex);
    auto it = std::find_if(g_inventory.begin(), g_inventory.end(), [&](const Item& it){ return it.id == itemID; });
    if (it == g_inventory.end()) return false;
    g_inventory.erase(it);
    return true;
}

bool insertSale(int /*userID*/, int /*itemID*/, int /*quantity*/, const std::string& /*saleDate*/) {
    // For now, sales are not persisted separately in this in-memory version.
    return true;
}
