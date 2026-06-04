#include "../include/db.h"

#include <iostream>
#include <memory>
#include <mysql/jdbc.h>

using namespace std;

static const string DB_HOST = "tcp://127.0.0.1:3306";
static const string DB_USER = "root";
static const string DB_PASS = "";
static const string DB_NAME = "workshop";

static const string DEFAULT_ADMIN_USERNAME = "admin";
static const string DEFAULT_ADMIN_PASSWORD = "arif";

static void printDatabaseError(const sql::SQLException& e) {
    cerr << "Database error: " << e.what() << endl;
    cerr << "MySQL error code: " << e.getErrorCode() << endl;
    cerr << "SQL state: " << e.getSQLState() << endl;
}

static unique_ptr<sql::Connection> openConnection(bool useDatabase = true) {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

    unique_ptr<sql::Connection> conn(
        driver->connect(DB_HOST, DB_USER, DB_PASS)
    );

    if (useDatabase) {
        conn->setSchema(DB_NAME);
    }

    return conn;
}

bool connectDatabase() {
    try {
        unique_ptr<sql::Connection> conn = openConnection(false);
        unique_ptr<sql::Statement> stmt(conn->createStatement());

        stmt->execute("CREATE DATABASE IF NOT EXISTS " + DB_NAME);
        stmt->execute("USE " + DB_NAME);

        stmt->execute(R"(
            CREATE TABLE IF NOT EXISTS users (
                userID INT AUTO_INCREMENT PRIMARY KEY,
                username VARCHAR(50) NOT NULL UNIQUE,
                password VARCHAR(255) NOT NULL
            ) ENGINE=InnoDB
        )");

        stmt->execute(R"(
            CREATE TABLE IF NOT EXISTS items (
                itemID INT AUTO_INCREMENT PRIMARY KEY,
                name VARCHAR(100) NOT NULL,
                category VARCHAR(50) NOT NULL,
                price DECIMAL(10,2) NOT NULL,
                expiry_date DATE NOT NULL,
                quantity INT NOT NULL DEFAULT 0
            ) ENGINE=InnoDB
        )");

        stmt->execute(R"(
            CREATE TABLE IF NOT EXISTS sales (
                saleID INT AUTO_INCREMENT PRIMARY KEY,
                userID INT NOT NULL,
                itemID INT NOT NULL,
                quantity_sold INT NOT NULL,
                sale_date DATE NOT NULL,
                sale_time TIME NOT NULL,

                FOREIGN KEY (userID) REFERENCES users(userID)
                    ON UPDATE CASCADE
                    ON DELETE RESTRICT,

                FOREIGN KEY (itemID) REFERENCES items(itemID)
                    ON UPDATE CASCADE
                    ON DELETE RESTRICT
            ) ENGINE=InnoDB
        )");

        stmt->execute(R"(
            CREATE TABLE IF NOT EXISTS waste (
                wasteID INT AUTO_INCREMENT PRIMARY KEY,
                itemID INT NOT NULL,
                quantity INT NOT NULL,
                waste_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
                reason VARCHAR(100) NOT NULL,

                FOREIGN KEY (itemID) REFERENCES items(itemID)
                    ON UPDATE CASCADE
                    ON DELETE RESTRICT
            ) ENGINE=InnoDB
        )");

        unique_ptr<sql::PreparedStatement> insertAdmin(
            conn->prepareStatement(
                "INSERT IGNORE INTO users (username, password) VALUES (?, ?)"
            )
        );

        insertAdmin->setString(1, DEFAULT_ADMIN_USERNAME);
        insertAdmin->setString(2, DEFAULT_ADMIN_PASSWORD);
        insertAdmin->executeUpdate();

        cout << "Database connected and tables checked successfully.\n";
        return true;

    } catch (sql::SQLException& e) {
        printDatabaseError(e);
        return false;
    }
}

void closeDatabase() {
    // Nothing needed here because every function opens/closes its own connection safely.
}

bool insertUser(const string& username, const string& password) {
    try {
        unique_ptr<sql::Connection> conn = openConnection();

        unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "INSERT INTO users (username, password) VALUES (?, ?)"
            )
        );

        stmt->setString(1, username);
        stmt->setString(2, password);

        return stmt->executeUpdate() > 0;

    } catch (sql::SQLException& e) {
        // Usually duplicate username
        return false;
    }
}

bool validateUserCredentials(const string& username, const string& password) {
    try {
        unique_ptr<sql::Connection> conn = openConnection();

        unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "SELECT userID FROM users WHERE username = ? AND password = ? LIMIT 1"
            )
        );

        stmt->setString(1, username);
        stmt->setString(2, password);

        unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        return res->next();

    } catch (sql::SQLException& e) {
        printDatabaseError(e);
        return false;
    }
}

bool insertItem(Item& item) {
    try {
        unique_ptr<sql::Connection> conn = openConnection();

        unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "INSERT INTO items (name, category, price, expiry_date, quantity) "
                "VALUES (?, ?, ?, ?, ?)"
            )
        );

        stmt->setString(1, item.name);
        stmt->setString(2, item.category);
        stmt->setDouble(3, item.price);
        stmt->setString(4, item.expiryDate);
        stmt->setInt(5, item.quantity);

        stmt->executeUpdate();

        unique_ptr<sql::Statement> idStmt(conn->createStatement());
        unique_ptr<sql::ResultSet> res(
            idStmt->executeQuery("SELECT LAST_INSERT_ID() AS itemID")
        );

        if (res->next()) {
            item.id = res->getInt("itemID");
        }

        return true;

    } catch (sql::SQLException& e) {
        printDatabaseError(e);
        return false;
    }
}

bool loadItemsFromDatabase(vector<Item>& inventory, int& nextId) {
    try {
        unique_ptr<sql::Connection> conn = openConnection();

        unique_ptr<sql::Statement> stmt(conn->createStatement());

        unique_ptr<sql::ResultSet> res(
            stmt->executeQuery(
                "SELECT itemID, name, category, price, "
                "DATE_FORMAT(expiry_date, '%Y-%m-%d') AS expiryDate, quantity "
                "FROM items "
                "ORDER BY itemID"
            )
        );

        inventory.clear();

        int maxId = 0;

        while (res->next()) {
            Item item;

            item.id = res->getInt("itemID");
            item.name = res->getString("name");
            item.category = res->getString("category");
            item.price = res->getDouble("price");
            item.expiryDate = res->getString("expiryDate");
            item.quantity = res->getInt("quantity");

            inventory.push_back(item);

            if (item.id > maxId) {
                maxId = item.id;
            }
        }

        nextId = maxId + 1;
        return true;

    } catch (sql::SQLException& e) {
        printDatabaseError(e);
        return false;
    }
}

bool updateItemQuantityInDatabase(int itemID, int quantity) {
    if (quantity < 0) {
        return false;
    }

    try {
        unique_ptr<sql::Connection> conn = openConnection();

        unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "UPDATE items SET quantity = ? WHERE itemID = ?"
            )
        );

        stmt->setInt(1, quantity);
        stmt->setInt(2, itemID);

        return stmt->executeUpdate() > 0;

    } catch (sql::SQLException& e) {
        printDatabaseError(e);
        return false;
    }
}

bool deleteItemFromDatabase(int itemID) {
    try {
        unique_ptr<sql::Connection> conn = openConnection();

        unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "DELETE FROM items WHERE itemID = ?"
            )
        );

        stmt->setInt(1, itemID);

        return stmt->executeUpdate() > 0;

    } catch (sql::SQLException& e) {
        printDatabaseError(e);
        return false;
    }
}

static int getDefaultAdminUserID(sql::Connection* conn) {
    unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "SELECT userID FROM users WHERE username = ? LIMIT 1"
        )
    );

    stmt->setString(1, DEFAULT_ADMIN_USERNAME);

    unique_ptr<sql::ResultSet> res(stmt->executeQuery());

    if (res->next()) {
        return res->getInt("userID");
    }

    return 1;
}

bool insertSale(int userID, int itemID, int quantity, const string& saleDate) {
    if (quantity <= 0) {
        return false;
    }

    unique_ptr<sql::Connection> conn;

    try {
        conn = openConnection();
        conn->setAutoCommit(false);

        unique_ptr<sql::PreparedStatement> checkStock(
            conn->prepareStatement(
                "SELECT quantity FROM items WHERE itemID = ? FOR UPDATE"
            )
        );

        checkStock->setInt(1, itemID);

        unique_ptr<sql::ResultSet> stockResult(checkStock->executeQuery());

        if (!stockResult->next()) {
            conn->rollback();
            return false;
        }

        int currentQuantity = stockResult->getInt("quantity");

        if (quantity > currentQuantity) {
            conn->rollback();
            return false;
        }

        int actualUserID = userID;

        // Keeps compatibility if your old code passed 0 because userID was ignored before.
        if (actualUserID <= 0) {
            actualUserID = getDefaultAdminUserID(conn.get());
        }

        if (saleDate.empty()) {
            unique_ptr<sql::PreparedStatement> insertSaleStmt(
                conn->prepareStatement(
                    "INSERT INTO sales (userID, itemID, quantity_sold, sale_date, sale_time) "
                    "VALUES (?, ?, ?, CURDATE(), CURTIME())"
                )
            );

            insertSaleStmt->setInt(1, actualUserID);
            insertSaleStmt->setInt(2, itemID);
            insertSaleStmt->setInt(3, quantity);
            insertSaleStmt->executeUpdate();

        } else {
            unique_ptr<sql::PreparedStatement> insertSaleStmt(
                conn->prepareStatement(
                    "INSERT INTO sales (userID, itemID, quantity_sold, sale_date, sale_time) "
                    "VALUES (?, ?, ?, ?, CURTIME())"
                )
            );

            insertSaleStmt->setInt(1, actualUserID);
            insertSaleStmt->setInt(2, itemID);
            insertSaleStmt->setInt(3, quantity);
            insertSaleStmt->setString(4, saleDate);
            insertSaleStmt->executeUpdate();
        }

        unique_ptr<sql::PreparedStatement> updateStock(
            conn->prepareStatement(
                "UPDATE items SET quantity = quantity - ? WHERE itemID = ?"
            )
        );

        updateStock->setInt(1, quantity);
        updateStock->setInt(2, itemID);
        updateStock->executeUpdate();

        conn->commit();
        conn->setAutoCommit(true);

        return true;

    } catch (sql::SQLException& e) {
        if (conn) {
            try {
                conn->rollback();
            } catch (...) {
                // Ignore rollback error
            }
        }

        printDatabaseError(e);
        return false;
    }
}