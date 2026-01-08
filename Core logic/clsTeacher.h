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

class clsTeacher : public clsPerson
{
private:
    string _ID;
    string _Department;
    string _Password;
    bool _MarkForDelete = false;

    enum enMode;
    enMode _Mode;

    // Escape quotes for SQLite
    static string _EscapeQuotes(const string& s)
    {
        string r;
        r.reserve(s.size());
        for (char c : s)
        {
            if (c == '\'') r += "''";
            else r += c;
        }
        return r;
    }

    static clsTeacher _GetEmptyTeacherObject()
    {
        return clsTeacher(enMode::EmptyMode, "", "", "", "", "", "", "");
    }

    static clsTeacher _ConvertRowToTeacher(sqlite3_stmt* stmt)
    {
        string id = (const char*)sqlite3_column_text(stmt, 0);
        string fname = (const char*)sqlite3_column_text(stmt, 1);
        string lname = (const char*)sqlite3_column_text(stmt, 2);
        string email = (const char*)sqlite3_column_text(stmt, 3);
        string phone = (const char*)sqlite3_column_text(stmt, 4);
        string dept = (const char*)sqlite3_column_text(stmt, 5);
        string pass = (const char*)sqlite3_column_text(stmt, 6);

        return clsTeacher(enMode::UpdateMode, id, fname, lname, email, phone, dept, pass);
    }

    // ========================== FIND DB ===========================

    static clsTeacher _FindByIDAndPassword_DB(const string& ID, const string& Password)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return _GetEmptyTeacherObject();

        string sql =
            "SELECT TeacherID, FirstName, LastName, Email, Phone, Department, Password "
            "FROM Teachers WHERE TeacherID = '" + _EscapeQuotes(ID) +
            "' AND Password = '" + _EscapeQuotes(Password) + "';";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return _GetEmptyTeacherObject();
        }

        clsTeacher teacher = _GetEmptyTeacherObject();

        if (sqlite3_step(stmt) == SQLITE_ROW)
            teacher = _ConvertRowToTeacher(stmt);

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return teacher;
    }

    static clsTeacher _FindByID_DB(const string& ID)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return _GetEmptyTeacherObject();

        string sql =
            "SELECT TeacherID, FirstName, LastName, Email, Phone, Department, Password "
            "FROM Teachers WHERE TeacherID = '" + _EscapeQuotes(ID) + "';";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return _GetEmptyTeacherObject();
        }

        clsTeacher teacher = _GetEmptyTeacherObject();

        if (sqlite3_step(stmt) == SQLITE_ROW)
            teacher = _ConvertRowToTeacher(stmt);

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return teacher;
    }


    // =========================== SAVE DB ==========================

    void _AddToDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "INSERT INTO Teachers "
            "(TeacherID, FirstName, LastName, Email, Phone, Department, Password) VALUES ('" +
            _EscapeQuotes(_ID) + "','" +
            _EscapeQuotes(FirstName) + "','" +
            _EscapeQuotes(LastName) + "','" +
            _EscapeQuotes(Email) + "','" +
            _EscapeQuotes(Phone) + "','" +
            _EscapeQuotes(_Department) + "','" +
            _EscapeQuotes(_Password) + "');";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    void _UpdateInDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "UPDATE Teachers SET "
            "FirstName='" + _EscapeQuotes(FirstName) + "', "
            "LastName='" + _EscapeQuotes(LastName) + "', "
            "Email='" + _EscapeQuotes(Email) + "', "
            "Phone='" + _EscapeQuotes(Phone) + "', "
            "Department='" + _EscapeQuotes(_Department) + "', "
            "Password='" + _EscapeQuotes(_Password) + "' "
            "WHERE TeacherID='" + _EscapeQuotes(_ID) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    void _DeleteFromDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "DELETE FROM Teachers WHERE TeacherID='" + _EscapeQuotes(_ID) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

public:

    enum enMode { EmptyMode = 0, UpdateMode = 1, AddTeacherMode = 2, DeleteTeacherMode = 3 };
    enum enSaveResult { svFaildEmptyObject = 0, svSucceeded = 1, svFaildAlreadyExists = 2 };

    clsTeacher() : clsPerson() {
        _ID = "";
        _Department = "";
        _Password = "";
        _Mode = EmptyMode;
    }

    clsTeacher(enMode Mode, string ID, string First, string Last,
        string Email, string Phone, string Department, string Password)
        : clsPerson(First, Last, Email, Phone)
    {
        _ID = ID;
        _Department = Department;
        _Password = Password;
        _Mode = Mode;
    }

    string ID() const { return _ID; }
    string Department() const { return _Department; }
    string Password() const { return _Password; }

    void SetPassword(string NewPassword)
    {
        _Password = NewPassword;
    }

    // ========================= SAVE PUBLIC =========================

    enSaveResult Save()
    {
        switch (_Mode)
        {
        case EmptyMode:
            return svFaildEmptyObject;

        case UpdateMode:
            _UpdateInDB();
            return svSucceeded;

        case AddTeacherMode:
            if (IsTeacherExist(_ID))
                return svFaildAlreadyExists;
            _AddToDB();
            _Mode = UpdateMode;
            return svSucceeded;

        case DeleteTeacherMode:
            _DeleteFromDB();
            return svSucceeded;
        }

        return svFaildEmptyObject;
    }


    // ========================= FIND PUBLIC =========================

    static clsTeacher Find(string ID, string Password)
    {
        return _FindByIDAndPassword_DB(ID, Password);
    }

    static clsTeacher Find(string ID)
    {
        return _FindByID_DB(ID);
    }

    static bool IsTeacherExist(string ID)
    {
        clsTeacher T = Find(ID);
        return (!T.IsEmpty());
    }

    bool IsEmpty()
    {
        return (_Mode == EmptyMode);
    }


    // ========================= LIST DB =========================

    static vector<clsTeacher> GetTeachersList()
    {
        vector<clsTeacher> list;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return list;

        string sql =
            "SELECT TeacherID, FirstName, LastName, Email, Phone, Department, Password FROM Teachers;";

        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
                list.push_back(_ConvertRowToTeacher(stmt));
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return list;
    }

    // ======================== SetDepartment  ========================

    void SetDepartment(string Department)
    {
        if (Department == "CS" || Department == "COMPUTER SCIENCE")
            _Department = "Computer Science";
        else if (Department == "SE" || Department == "SOFTWARE ENGINEERING")
            _Department = "Software Engineering";
        else if (Department == "CIS" || Department == "COMPUTER INFORMATION SYSTEMS")
            _Department = "Computer Information Systems";
        else if (Department == "CY" || Department == "CYBER SECURITY")
            _Department = "Cyber Security";
        else if (Department == "GS" || Department == "GENERAL STUDIES")
            _Department = "General Studies";
        else if (Department == "MA" || Department == "MATHEMATICS")
            _Department = "Mathematics";
        else if (Department == "PHY" || Department == "PHYSICS")
            _Department = "Physics";
        else
            _Department = "Unknown";
    }
};
