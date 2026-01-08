#pragma once
#include "Global.h"
#include "clsPerson.h"
#include "clsString.h"
#include "clsCourse.h"
#include "sqlite3.h"
#include "clsDB.h"
#include <string>
#include <vector>
#include <iostream>
#include "clsGrades.h"

using namespace std;

class clsStudent : public clsPerson
{
private:
    string _ID;
    string _Major;
    string _Password;
    int _CompletedHours = 0;
    bool _MarkForDelete = false;

    enum enMode;
    enMode _Mode;

    // ========= DB helpers =========

    static string _EscapeQuotes(const string& s)
    {
        string r;
        r.reserve(s.size());
        for (char c : s)
        {
            if (c == '\'')
                r += "''";   // SQLite escaping for single quote
            else
                r += c;
        }
        return r;
    }

    static clsStudent _GetEmptyStudentObject()
    {
        return clsStudent(enMode::EmptyMode, "", "", "", "", "", "", "");
    }

    static clsStudent _ConvertRowToStudent(sqlite3_stmt* stmt)
    {
        // columns: 0:ID, 1:FirstName, 2:LastName, 3:Email, 4:Phone, 5:Major, 6:Password
        string id = (const char*)sqlite3_column_text(stmt, 0);
        string fname = (const char*)sqlite3_column_text(stmt, 1);
        string lname = (const char*)sqlite3_column_text(stmt, 2);
        string email = (const char*)sqlite3_column_text(stmt, 3);
        string phone = (const char*)sqlite3_column_text(stmt, 4);
        string major = (const char*)sqlite3_column_text(stmt, 5);
        string pass = (const char*)sqlite3_column_text(stmt, 6);

        return clsStudent(enMode::UpdateMode, id, fname, lname, email, phone, major, pass);
    }

    // ---- low-level DB find helpers (used by public Find) ----

    static clsStudent _FindByIDAndPassword_DB(const string& ID, const string& Password)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db)
            return _GetEmptyStudentObject();

        string sql =
            "SELECT StudentID, FirstName, LastName, Email, Phone, Major, Password "
            "FROM Students WHERE StudentID = '" + _EscapeQuotes(ID) +
            "' AND Password = '" + _EscapeQuotes(Password) + "';";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK)
        {
            if (stmt) sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return _GetEmptyStudentObject();
        }

        clsStudent student = _GetEmptyStudentObject();

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            student = _ConvertRowToStudent(stmt);
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return student;
    }

    static clsStudent _FindByID_DB(const string& ID)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db)
            return _GetEmptyStudentObject();

        string sql =
            "SELECT StudentID, FirstName, LastName, Email, Phone, Major, Password "
            "FROM Students WHERE StudentID = '" + _EscapeQuotes(ID) + "';";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK)
        {
            if (stmt) sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return _GetEmptyStudentObject();
        }

        clsStudent student = _GetEmptyStudentObject();

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            student = _ConvertRowToStudent(stmt);
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return student;
    }

    // ---- low-level DB write helpers (used by Save) ----

    void _AddToDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "INSERT INTO Students "
            "(StudentID, FirstName, LastName, Email, Phone, Major, Password) VALUES ('" +
            _EscapeQuotes(_ID) + "','" +
            _EscapeQuotes(FirstName) + "','" +
            _EscapeQuotes(LastName) + "','" +
            _EscapeQuotes(Email) + "','" +
            _EscapeQuotes(Phone) + "','" +
            _EscapeQuotes(_Major) + "','" +
            _EscapeQuotes(_Password) + "');";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    void _UpdateInDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "UPDATE Students SET "
            "FirstName = '" + _EscapeQuotes(FirstName) + "', "
            "LastName  = '" + _EscapeQuotes(LastName) + "', "
            "Email     = '" + _EscapeQuotes(Email) + "', "
            "Phone     = '" + _EscapeQuotes(Phone) + "', "
            "Major     = '" + _EscapeQuotes(_Major) + "', "
            "Password  = '" + _EscapeQuotes(_Password) + "' "
            "WHERE StudentID = '" + _EscapeQuotes(_ID) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    void _DeleteFromDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "DELETE FROM Students WHERE StudentID = '" + _EscapeQuotes(_ID) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

public:
    enum enMode { EmptyMode = 0, UpdateMode = 1, AddStudentMode = 2, DeleteStudentMode = 3 };
    enum enSaveResult { svFaildEmptyObject = 0, svSucceeded = 1, svFaildAlreadyExists = 2 };

    clsStudent() : clsPerson() {
        _ID = "";
        _Major = "";
        _Password = "";
        _Mode = EmptyMode;
    }

    clsStudent(enMode Mode, string ID, string FirstName, string LastName, string Email,
        string Phone, string Major, string Password)
        : clsPerson(FirstName, LastName, Email, Phone)
    {
        _ID = ID;
        _Major = Major;
        _Password = Password;
        _Mode = Mode;
    }

    // Getters
    string ID() const { return _ID; }
    string Major() const { return _Major; }
    string Password() const { return _Password; }
    int CompletedHours() const { return _CompletedHours; }
    void SetCompletedHours(int Hours) { _CompletedHours = Hours; }
    string Department() const { return _Major; }
    string phone() 
    {
        return Phone; 
    }
    bool MarkForDelete() const { return _MarkForDelete; }

    // Setters
    void SetPassword(string NewPassword)
    {
        _Password = NewPassword;
    }

    void SetMajor(string NewMajor)
    {
        _Major = NewMajor;
    }


    // ========== Save (now DB only) ==========

    enSaveResult Save()
    {
        switch (_Mode)
        {
        case enMode::EmptyMode:
            return enSaveResult::svFaildEmptyObject;

        case enMode::UpdateMode:
            _UpdateInDB();
            return enSaveResult::svSucceeded;

        case enMode::AddStudentMode:
            if (IsStudentExist(_ID))
                return enSaveResult::svFaildAlreadyExists;
            else
            {
                _AddToDB();
                _Mode = enMode::UpdateMode;
                return enSaveResult::svSucceeded;
            }

        case enMode::DeleteStudentMode:
            _DeleteFromDB();
            return enSaveResult::svSucceeded;
        }

        return enSaveResult::svFaildEmptyObject;
    }

    // ========== Find () ==========

    static clsStudent Find(string ID, string Password)
    {
        return _FindByIDAndPassword_DB(ID, Password);
    }

    static clsStudent Find(string ID)
    {
        return _FindByID_DB(ID);
    }

    static bool IsStudentExist(string ID)
    {
        clsStudent S = clsStudent::Find(ID);
        return (!S.IsEmpty());
    }

    bool IsEmpty()
    {
        return (_Mode == enMode::EmptyMode);
    }

    // ========== GetStudentsList ==========

    static vector<clsStudent> GetStudentsList()
    {
        vector<clsStudent> vStudents;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db)
            return vStudents;

        string sql =
            "SELECT StudentID, FirstName, LastName, Email, Phone, Major, Password "
            "FROM Students;";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK)
        {
            if (stmt) sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return vStudents;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            clsStudent S = _ConvertRowToStudent(stmt);
            vStudents.push_back(S);
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return vStudents;
    }


    bool CanRegisterCourse(clsCourse& Course)
    {
        if (clsCourse::IsStudentAlreadyEnrolled(ID(), Course.Code()))
        {
            return false;
        }

        if (!clsCourse::HasCompletedPrerequisite(ID(), Course.Prerequisite()))
        {
            return false;
        }

        int CurrentHours = clsCourse::GetStudentTotalHours(ID());
        if (CurrentHours + stoi(Course.CreditHours()) > 18)
        {
            return false;
        }
        clsGrade G = clsGrade::Find(ID(), Course.Code());

        if (!G.IsEmpty())
        {
            if (G.Grade() >= 60)
            {
                return false;
            }
        }

        return true;
    }
};
