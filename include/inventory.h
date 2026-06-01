#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <vector>
#include "db.h"

void clearInput();
void showMenu();
int findItemIndex(const std::vector<Item>& inventory, int id);
int findItemIndexByName(const std::vector<Item>& inventory, const std::string& name);
void listItems(const std::vector<Item>& inventory);
void addItem(std::vector<Item>& inventory, int& nextId);
void updateQuantity(std::vector<Item>& inventory);
void removeItem(std::vector<Item>& inventory);
void searchItem(const std::vector<Item>& inventory);
void runInventoryModule();

#endif // INVENTORY_H
