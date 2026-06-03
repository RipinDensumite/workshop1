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

#include <mysql/jdbc.h>

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

void setupAndDisplayUsers() {
    try {
        sql::mysql::MySQL_Driver* driver;
        sql::Connection* conn;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = sql::mysql::get_mysql_driver_instance();

        conn = driver->connect(
            "tcp://127.0.0.1:3306",
            "root",
            ""
        );

        stmt = conn->createStatement();

        stmt->execute("CREATE DATABASE IF NOT EXISTS workshop");
        stmt->execute("USE workshop");

        stmt->execute(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "name VARCHAR(100) NOT NULL,"
            "password VARCHAR(100) NOT NULL"
            ")"
        );

        res = stmt->executeQuery("SELECT id, name, password FROM users");

        cout << "\nAll Users\n";
        cout << "----------------------\n";

        bool hasUsers = false;

        while (res->next()) {
            hasUsers = true;

            cout << "ID: " << res->getInt("id") << endl;
            cout << "Name: " << res->getString("name") << endl;
            cout << "Password: " << res->getString("password") << endl;
            cout << "----------------------\n";
        }

        if (!hasUsers) {
            cout << "No users found.\n";
        }

        delete res;
        delete stmt;
        delete conn;
    }
    catch (sql::SQLException& e) {
        cout << "\nMySQL connection failed.\n";
        cout << "Error: " << e.what() << endl;
    }
}

int main() {
    cout << "Hello world" << endl;

    setupAndDisplayUsers();

    return 0;
}