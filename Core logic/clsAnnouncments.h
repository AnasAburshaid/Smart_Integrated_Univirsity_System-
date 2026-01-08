#pragma once
#include "Global.h"
#include "clsString.h"
#include "sqlite3.h"
#include "clsDB.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class clsAnnouncement
{
private:

    string _CourseCode;
    string _TeacherName;
    string _Date;
    string _Title;
    string _Message;

    enum enMode;
    enMode _Mode;

  
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

    
    static clsAnnouncement _GetEmptyAnnouncementObject()
    {
        return clsAnnouncement("", "", "", "", "", EmptyMode);
    }

   
    static clsAnnouncement _ConvertRowToAnnouncement(sqlite3_stmt* stmt)
    {
        string course = (const char*)sqlite3_column_text(stmt, 0);
        string teacher = (const char*)sqlite3_column_text(stmt, 1);
        string date = (const char*)sqlite3_column_text(stmt, 2);
        string title = (const char*)sqlite3_column_text(stmt, 3);
        string message = (const char*)sqlite3_column_text(stmt, 4);

        return clsAnnouncement(course, teacher, date, title, message, UpdateMode);
    }

   
    void _AddToDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "INSERT INTO Announcements "
            "(CourseCode, TeacherName, Date, Title, Message) VALUES ('" +
            _EscapeQuotes(_CourseCode) + "', '" +
            _EscapeQuotes(_TeacherName) + "', '" +
            _EscapeQuotes(_Date) + "', '" +
            _EscapeQuotes(_Title) + "', '" +
            _EscapeQuotes(_Message) + "');";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    
    void _UpdateInDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "UPDATE Announcements SET "
            "TeacherName='" + _EscapeQuotes(_TeacherName) + "', "
            "Date='" + _EscapeQuotes(_Date) + "', "
            "Message='" + _EscapeQuotes(_Message) + "' "
            "WHERE CourseCode='" + _EscapeQuotes(_CourseCode) + "' "
            "AND Title='" + _EscapeQuotes(_Title) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    
    void _DeleteFromDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "DELETE FROM Announcements "
            "WHERE CourseCode='" + _EscapeQuotes(_CourseCode) + "' "
            "AND Title='" + _EscapeQuotes(_Title) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

public:

    enum enMode { EmptyMode = 0, AddMode = 1, UpdateMode = 2, DeleteMode = 3 };
    enum enSaveResult { svFaildEmptyObject = 0, svSucceeded = 1, svFaildAlreadyExists = 2 };

    clsAnnouncement(string CourseCode = "", string TeacherName = "", string Date = "",
        string Title = "", string Message = "", enMode Mode = AddMode)
    {
        _CourseCode = CourseCode;
        _TeacherName = TeacherName;
        _Date = Date;
        _Title = Title;
        _Message = Message;
        _Mode = Mode;
    }

    
    string CourseCode() const { return _CourseCode; }
    string TeacherName() const { return _TeacherName; }
    string Date() const { return _Date; }
    string Title() const { return _Title; }
    string Message() const { return _Message; }

    void SetMessage(string NewMsg) { _Message = NewMsg; }
    void SetDate(string NewDate) { _Date = NewDate; }

    bool IsEmpty() const
    {
        return _Mode == EmptyMode;
    }

   
    static clsAnnouncement Find(string CourseCode, string Title)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return _GetEmptyAnnouncementObject();

        string sql =
            "SELECT CourseCode, TeacherName, Date, Title, Message "
            "FROM Announcements WHERE CourseCode='" + _EscapeQuotes(CourseCode) +
            "' AND Title='" + _EscapeQuotes(Title) + "';";

        sqlite3_stmt* stmt = nullptr;
        clsAnnouncement A = _GetEmptyAnnouncementObject();

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
                A = _ConvertRowToAnnouncement(stmt);
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return A;
    }

    static bool IsAnnouncementExist(string CourseCode, string Title)
    {
        return !Find(CourseCode, Title).IsEmpty();
    }

    
    enSaveResult Save()
    {
        if (_Mode == EmptyMode)
            return svFaildEmptyObject;

        switch (_Mode)
        {
        case AddMode:
            if (IsAnnouncementExist(_CourseCode, _Title))
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

   
    static vector<clsAnnouncement> GetAnnouncementsData()
    {
        vector<clsAnnouncement> v;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return v;

        string sql =
            "SELECT CourseCode, TeacherName, Date, Title, Message "
            "FROM Announcements;";
           
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
                v.push_back(_ConvertRowToAnnouncement(stmt));
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return v;
    }
};
