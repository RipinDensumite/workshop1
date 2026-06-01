#include "../include/mainmenu.h"
#include "../include/ExpiryTrack.h"
#include "../include/StockMonitor.h"
#include "../include/Report.h"
#include "../include/screen.h"

#include <iostream>
#include <limits>

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int showMainMenu() {
    while (true) {
        clearScreen();
        std::cout << "===FRESH ITEM INVENTORY SYSTEM===\n\n";
        std::cout << "1. Inventory Menu\n";
        std::cout << "2. Expiry tracking\n";
        std::cout << "3. Stock Monitoring\n";
        std::cout << "4. Reports\n";
        std::cout << "5. Exit\n\n";
        std::cout << "Choose an option: ";

        int choice;
        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "Invalid selection. Please enter a number.\n";
            pauseMenu();
            continue;
        }

        clearInput();

        switch (choice) {
            case 1:
                return 1;
            case 2:
                showExpiryTrackingMenu();
                break;
            case 3:
                showStockMonitoringMenu();
                break;
            case 4:
                showReportMenu();
                break;
            case 5:
                return 5;
            default:
                std::cout << "Please choose a valid option (1-5).\n";
                pauseMenu();
                break;
        }
    }
}
