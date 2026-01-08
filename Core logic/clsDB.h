#pragma once
#include <iostream>
#include <string>       
#include "sqlite3.h"

using namespace std;

class clsDB
{

public:
    static sqlite3* OpenDB(string path)
    {
        sqlite3* db;

        string fullPath = "C:/dev/UniversityAPIServer/core_logic/University.db";

        int rc = sqlite3_open(fullPath.c_str(), &db);
        if (rc) {
            return nullptr;
        }
        return db;
    }

    static bool Execute(sqlite3* db, string Query)
    {
        char* errMsg = 0;
        int rc = sqlite3_exec(db, Query.c_str(), nullptr, 0, &errMsg);

        if (rc != SQLITE_OK) {
            cout << "SQL Error: " << errMsg << endl;
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    static void CloseDB(sqlite3* db)
    {
        if (db) {
            sqlite3_close(db);
        }
    }
};