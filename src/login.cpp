#include "../include/login.h"
#include "../include/screen.h"
#include "../include/db.h"

#include <conio.h>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

static std::string readPassword() {
    std::string password;
    int ch;

    while (true) {
        ch = _getch();
        if (ch == '\r' || ch == '\n') {
            std::cout << '\n';
            break;
        }
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password.push_back(static_cast<char>(ch));
            std::cout << '*';
        }
    }

    return password;
}

static void showLoginMenu() {
    clearScreen();
    std::cout << "**************************************************\n";
    std::cout << "*                                                *\n";
    std::cout << "*          WELCOME TO FRESH ITEM INVENTORY        *\n";
    std::cout << "*                    SYSTEM                      *\n";
    std::cout << "*                                                *\n";
    std::cout << "**************************************************\n\n";
    std::cout << "1. Login\n";
    std::cout << "2. Exit\n";
    std::cout << "Choose an option: ";
}

static bool loginWithDatabase() {
    clearScreen();
    std::cout << "--- Login ---\n";
    std::cout << "Please enter your credentials\n\n";

    std::cout << "Username: ";
    std::string username;
    std::getline(std::cin, username);

    std::cout << "Password: ";
    std::string password = readPassword();

    if (validateUserCredentials(username, password)) {
        std::cout << "Login successful.\n";
        pauseMenu();
        return true;
    }

    std::cout << "Wrong username or password entered.\n";
    pauseMenu();
    return false;
}

void showLoginScreen() {
    clearScreen();

    // Initialize in-memory storage (creates a default user if needed)
    connectDatabase();

    while (true) {
        showLoginMenu();
        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid selection.\n";
            pauseMenu();
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            if (loginWithDatabase()) {
                closeDatabase();
                return;
            }
        } else if (choice == 2) {
            closeDatabase();
            std::exit(0);
        } else {
            std::cout << "Please enter 1 or 2.\n";
            pauseMenu();
        }
    }
}
