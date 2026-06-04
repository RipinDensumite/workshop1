#include "../include/inventory.h"
#include "../include/login.h"
#include "../include/mainmenu.h"
#include "../include/screen.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>
using namespace std;

void showTemporaryMessage(const std::string& message) {
    cout << message << std::endl;
    this_thread::sleep_for(std::chrono::seconds(3));
}

void clearScreen() {
    system("cls");
}

void pauseMenu() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
    showLoginScreen();
    while (true) {
        int choice = showMainMenu();
        if (choice == 1) {
            runInventoryModule();
        } else if (choice == 5) {
            break;
        }
    }
    return 0;
}
