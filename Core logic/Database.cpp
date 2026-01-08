#include"Database.h"
#include"sqlite3.h"
#include <iostream>

bool Database::TestConnection()
{
    sqlite3* db;
    int rc = sqlite3_open("University.db", &db);

    if (rc)
    {
        std::cout << "Failed to open database: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    else
    {
        std::cout << "Database opened successfully!\n";
        sqlite3_close(db);
        return true;
    }
}
