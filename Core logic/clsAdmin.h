#pragma once
#include "Global.h"
#include "clsPerson.h"
#include "clsString.h"
#include "sqlite3.h"
#include "clsDB.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class clsAdmin : public clsPerson
{
private:

    string _Username;
    string _Password;
    string _Department;
    bool _MarkForDelete = false;

    enum enMode;
    enMode _Mode;

    static string _EscapeQuotes(const string& s)
    {
        string r;
        r.reserve(s.size());
        for (char c : s)
        {
            if (c == '\'')
                r += "''";
            else
                r += c;
        }
        return r;
    }

    static clsAdmin _GetEmptyAdminObject()
    {
        return clsAdmin(enMode::EmptyMode, "", "", "", "", "", "", "");
    }

    static clsAdmin _ConvertRowToAdmin(sqlite3_stmt* stmt)
    {
        string uname = (const char*)sqlite3_column_text(stmt, 0);
        string pass = (const char*)sqlite3_column_text(stmt, 1);
        string fname = (const char*)sqlite3_column_text(stmt, 2);
        string lname = (const char*)sqlite3_column_text(stmt, 3);
        string email = (const char*)sqlite3_column_text(stmt, 4);
        string phone = (const char*)sqlite3_column_text(stmt, 5);
        string dept = (const char*)sqlite3_column_text(stmt, 6);

        return clsAdmin(enMode::UpdateMode, uname, pass, fname, lname, email, phone, dept);
    }

    // ========================= DB FIND HELPERS =========================

    static clsAdmin _FindByUserAndPass_DB(const string& Username, const string& Password)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return _GetEmptyAdminObject();

        string sql =
            "SELECT Username, Password, FirstName, LastName, Email, Phone, Department "
            "FROM Admins WHERE Username='" + _EscapeQuotes(Username) +
            "' AND Password='" + _EscapeQuotes(Password) + "';";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return _GetEmptyAdminObject();
        }

        clsAdmin admin = _GetEmptyAdminObject();

        if (sqlite3_step(stmt) == SQLITE_ROW)
            admin = _ConvertRowToAdmin(stmt);

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return admin;
    }

    static clsAdmin _FindByUser_DB(const string& Username)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return _GetEmptyAdminObject();

        string sql =
            "SELECT Username, Password, FirstName, LastName, Email, Phone, Department "
            "FROM Admins WHERE Username='" + _EscapeQuotes(Username) + "';";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return _GetEmptyAdminObject();
        }

        clsAdmin admin = _GetEmptyAdminObject();

        if (sqlite3_step(stmt) == SQLITE_ROW)
            admin = _ConvertRowToAdmin(stmt);

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return admin;
    }


    // ========================= DB WRITE FUNCTIONS =========================

    void _AddToDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "INSERT INTO Admins (Username, Password, FirstName, LastName, Email, Phone, Department) "
            "VALUES ('" +
            _EscapeQuotes(_Username) + "','" +
            _EscapeQuotes(_Password) + "','" +
            _EscapeQuotes(FirstName) + "','" +
            _EscapeQuotes(LastName) + "','" +
            _EscapeQuotes(Email) + "','" +
            _EscapeQuotes(Phone) + "','" +
            _EscapeQuotes(_Department) + "');";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    void _UpdateInDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "UPDATE Admins SET "
            "Password='" + _EscapeQuotes(_Password) + "', "
            "FirstName='" + _EscapeQuotes(FirstName) + "', "
            "LastName='" + _EscapeQuotes(LastName) + "', "
            "Email='" + _EscapeQuotes(Email) + "', "
            "Phone='" + _EscapeQuotes(Phone) + "', "
            "Department='" + _EscapeQuotes(_Department) + "' "
            "WHERE Username='" + _EscapeQuotes(_Username) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    void _DeleteFromDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "DELETE FROM Admins WHERE Username='" + _EscapeQuotes(_Username) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

public:

    enum enMode { EmptyMode = 0, UpdateMode = 1, AddMode = 2, DeleteMode = 3 };
    enum enSaveResult { svFaildEmptyObject = 0, svSucceeded = 1, svFaildAlreadyExists = 2 };

    clsAdmin() : clsPerson() {
        _Username = "";
        _Password = "";
        _Department = "";
        _Mode = EmptyMode;
    }

    clsAdmin(enMode Mode, string Username, string Password, string FirstName,
        string LastName, string Email, string Phone, string Department)
        : clsPerson(FirstName, LastName, Email, Phone)
    {
        _Username = Username;
        _Password = Password;
        _Department = Department;
        _Mode = Mode;
    }

    string Username() const { return _Username; }
    string Password() const { return _Password; }
    string Department() const { return _Department; }

    void SetPassword(string NewPassword) { _Password = NewPassword; }
    void SetDepartment(string NewDepartment) { _Department = NewDepartment; }

    bool IsEmpty() const { return (_Mode == EmptyMode); }


    // ========================= PUBLIC FIND =========================

    static clsAdmin Find(string Username, string Password)
    {
        return _FindByUserAndPass_DB(Username, Password);
    }

    static clsAdmin Find(string Username)
    {
        return _FindByUser_DB(Username);
    }

    static bool IsAdminExist(string Username)
    {
        return !Find(Username).IsEmpty();
    }


    // ========================= SAVE (DB) =========================

    enSaveResult Save()
    {
        if (_Mode == EmptyMode)
            return svFaildEmptyObject;

        switch (_Mode)
        {
        case AddMode:
            if (IsAdminExist(_Username))
                return svFaildAlreadyExists;
            _AddToDB();
            _Mode = UpdateMode;
            return svSucceeded;

        case UpdateMode:
            _UpdateInDB();
            return svSucceeded;

        case DeleteMode:
            _DeleteFromDB();
            return svSucceeded;
        }

        return svFaildEmptyObject;
    }


    // ==================== GetAdminsList (DB VERSION) ====================

    static vector<clsAdmin> GetAdminsList()
    {
        vector<clsAdmin> vAdmins;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return vAdmins;

        string sql =
            "SELECT Username, Password, FirstName, LastName, Email, Phone, Department FROM Admins;";

        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
                vAdmins.push_back(_ConvertRowToAdmin(stmt));
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return vAdmins;
    }

};
