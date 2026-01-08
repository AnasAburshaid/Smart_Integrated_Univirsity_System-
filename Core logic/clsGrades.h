#pragma once
#include "Global.h"
#include "clsString.h"
#include "clsCourse.h"
#include "clsDB.h"
#include <sqlite3.h>

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class clsGrade
{
public:

    enum enMode { EmptyMode = 0, AddMode = 1, UpdateMode = 2, DeleteMode = 3 };

private:

    string _StudentID;
    string _CourseCode;
    string _Assignment;  
    string _Midterm;     
    string _Final;       
    string _Letter;      
    string _Semester;

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

    // ============================================================
    // Convert DB Row -> Object
    // ============================================================
    static clsGrade _ConvertRowToGrade(sqlite3_stmt* stmt)
    {
        string sid = (const char*)sqlite3_column_text(stmt, 0);
        string code = (const char*)sqlite3_column_text(stmt, 1);
        int assign = sqlite3_column_int(stmt, 2);
        int mid = sqlite3_column_int(stmt, 3);
        int fin = sqlite3_column_int(stmt, 4);
        string letter = (const char*)sqlite3_column_text(stmt, 5);
        string sem = (const char*)sqlite3_column_text(stmt, 6);

        return clsGrade(
            sid, code,
            to_string(assign),
            to_string(mid),
            to_string(fin),
            letter,
            sem,
            UpdateMode
        );
    }

    static clsGrade _GetEmptyGrade()
    {
        return clsGrade("", "", "0", "0", "0", "-", "", EmptyMode);
    }

    // ============================================================
    // DB INSERT
    // ============================================================
    void _AddToDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        int A = stoi(_Assignment);
        int M = stoi(_Midterm);
        int F = stoi(_Final);

        string sql =
            "INSERT INTO Grades "
            "(StudentID, CourseCode, Assignment, Midterm, Final, Letter, Semester) "
            "VALUES ('" + _EscapeQuotes(_StudentID) + "', '"
            + _EscapeQuotes(_CourseCode) + "', "
            + to_string(A) + ", "
            + to_string(M) + ", "
            + to_string(F) + ", '"
            + _EscapeQuotes(_Letter) + "', '"
            + _EscapeQuotes(_Semester) + "');";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    // ============================================================
    // DB UPDATE
    // ============================================================
    void _UpdateInDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        int A = stoi(_Assignment);
        int M = stoi(_Midterm);
        int F = stoi(_Final);

        string sql =
            "UPDATE Grades SET "
            "Assignment=" + to_string(A) + ", "
            "Midterm=" + to_string(M) + ", "
            "Final=" + to_string(F) + ", "
            "Letter='" + _EscapeQuotes(_Letter) + "', "
            "Semester='" + _EscapeQuotes(_Semester) + "' "
            "WHERE StudentID='" + _EscapeQuotes(_StudentID) + "' "
            "AND CourseCode='" + _EscapeQuotes(_CourseCode) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    // ============================================================
    // DB DELETE
    // ============================================================
    void _DeleteFromDB()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "DELETE FROM Grades "
            "WHERE StudentID='" + _EscapeQuotes(_StudentID) + "' "
            "AND CourseCode='" + _EscapeQuotes(_CourseCode) + "' "
            "AND Semester='" + _EscapeQuotes(_Semester) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

public:

    // Constructors
    clsGrade(string studentID = "", string courseCode = "",
        string assignment = "-", string midterm = "-", string finalMark = "-",
        string letter = "-", string semester = "", enMode mode = AddMode)
    {
        _StudentID = studentID;
        _CourseCode = courseCode;
        _Assignment = (assignment == "-" ? "0" : assignment);
        _Midterm = (midterm == "-" ? "0" : midterm);
        _Final = (finalMark == "-" ? "0" : finalMark);
        _Letter = letter;
        _Semester = semester;
        _Mode = mode;
    }

    // ============================================================
    // Getters
    // ============================================================
    string StudentID() const { return _StudentID; }
    string CourseCode() const { return _CourseCode; }
    string Assignment() const { return _Assignment; }
    string Midterm() const { return _Midterm; }
    string Final() const { return _Final; }
    string Letter() const { return _Letter; }
    string Semester() const { return _Semester; }

    // ============================================================
    // Setters (These were missing!)
    // ============================================================
    void SetAssignment(int val) { _Assignment = to_string(val); }
    void SetMidterm(int val) { _Midterm = to_string(val); }
    void SetFinal(int val) { _Final = to_string(val); }
    void SetLetter(string val) { _Letter = val; }

    // ============================================================
    // Helper: Is this grade included in GPA?
    // ============================================================

    bool IsIncludedInGPA() const
    {
        if (_Letter == "W" || _Letter == "I" || _Letter == "-" || _Letter == "")
            return false;

        return true;
    }


    // ============================================================
    // Load / Save Helpers
    // ============================================================

    static vector<clsGrade> LoadDataToVector()
    {
        vector<clsGrade> v;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return v;

        string sql =
            "SELECT StudentID, CourseCode, Assignment, Midterm, Final, Letter, Semester "
            "FROM Grades;";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
                v.push_back(_ConvertRowToGrade(stmt));
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return v;
    }

    static vector<clsGrade> GetStudentGrades(string StudentID)
    {
        vector<clsGrade> v;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return v;

        string sql =
            "SELECT StudentID, CourseCode, Assignment, Midterm, Final, Letter, Semester "
            "FROM Grades WHERE StudentID='" + _EscapeQuotes(StudentID) + "';";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
                v.push_back(_ConvertRowToGrade(stmt));
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return v;
    }

    static vector<clsGrade> GetCourseGrades(string CourseCode)
    {
        vector<clsGrade> v;
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return v;

        string sql = "SELECT StudentID, CourseCode, Assignment, Midterm, Final, Letter, Semester "
            "FROM Grades WHERE CourseCode='" + _EscapeQuotes(CourseCode) + "';";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW)
                v.push_back(_ConvertRowToGrade(stmt));
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return v;
    }

    // ============================================================
    // FIND
    // ============================================================

    // Find by student + course + semester
    static clsGrade Find(string StudentID, string CourseCode, string Semester)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return _GetEmptyGrade();

        string sql =
            "SELECT StudentID, CourseCode, Assignment, Midterm, Final, Letter, Semester "
            "FROM Grades WHERE StudentID='" + _EscapeQuotes(StudentID) + "' "
            "AND CourseCode='" + _EscapeQuotes(CourseCode) + "' "
            "AND Semester='" + _EscapeQuotes(Semester) + "';";

        sqlite3_stmt* stmt = nullptr;
        clsGrade G = _GetEmptyGrade();

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
                G = _ConvertRowToGrade(stmt);
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return G;
    }

    // Finds latest attempt (retake-safe)
    static clsGrade Find(string StudentID, string CourseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return _GetEmptyGrade();

        string sql =
            "SELECT StudentID, CourseCode, Assignment, Midterm, Final, Letter, Semester "
            "FROM Grades WHERE StudentID='" + _EscapeQuotes(StudentID) + "' "
            "AND CourseCode='" + _EscapeQuotes(CourseCode) + "' "
            "ORDER BY Semester DESC LIMIT 1;";

        sqlite3_stmt* stmt = nullptr;
        clsGrade G = _GetEmptyGrade();

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
                G = _ConvertRowToGrade(stmt);
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return G;
    }

    // ============================================================
    // GPA Calculation
    // ============================================================

    static double GradeToPoints(string Grade)
    {
        if (Grade == "A" || Grade == "A+") return 4.0;
        if (Grade == "A-") return 3.7;
        if (Grade == "B+") return 3.3;
        if (Grade == "B")  return 3.0;
        if (Grade == "B-") return 2.7;
        if (Grade == "C+") return 2.3;
        if (Grade == "C")  return 2.0;
        if (Grade == "C-") return 1.7;
        if (Grade == "D+") return 1.3;
        if (Grade == "D")  return 1.0;
        return 0.0;
    }

    // Helper: Convert Score to Letter (Matches Frontend Scale)
    static string GetLetterFromScore(int score)
    {
        if (score >= 90) return "A";
        if (score >= 85) return "A-";
        if (score >= 80) return "B+";
        if (score >= 76) return "B";
        if (score >= 72) return "B-";
        if (score >= 68) return "C+";
        if (score >= 64) return "C";
        if (score >= 60) return "C-";
        if (score >= 55) return "D+";
        if (score >= 50) return "D";
        return "F"; // Changed from F to return actual fail
    }


    static double CalculateGPA(string StudentID, string Semester)
    {
        vector<clsGrade> v = GetStudentGrades(StudentID);

        double TotalPoints = 0.0;
        double TotalHours = 0.0;

        for (clsGrade& G : v)
        {
            // 1. Must be the correct semester
            if (G.Semester() != Semester) continue;

            // 2. Must be a valid grade (Not W, Not I, Not -)
            if (!G.IsIncludedInGPA()) continue;

            clsCourse C = clsCourse::FindCourse(G.CourseCode());
            if (C.IsEmpty()) continue;

            double points = GradeToPoints(G.Letter());
            double hours = stod(C.CreditHours());

            TotalPoints += points * hours;
            TotalHours += hours;
        }

        if (TotalHours == 0.0) return 0.0;

        return TotalPoints / TotalHours;
    }

    static double CalculateCumulativeGPA(string StudentID)
    {
        vector<clsGrade> v = GetUniqueStudentGrades(StudentID);

        double TotalPoints = 0.0;
        double TotalHours = 0.0;

        for (clsGrade& G : v)
        {
            if (!G.IsIncludedInGPA()) continue;

            clsCourse C = clsCourse::FindCourse(G.CourseCode());
            if (C.IsEmpty()) continue;

            double points = GradeToPoints(G.Letter());
            double hours = stod(C.CreditHours());

            TotalPoints += points * hours;
            TotalHours += hours;
        }

        if (TotalHours == 0.0) return 0.0;

        return TotalPoints / TotalHours;
    }

    static vector<clsGrade> GetUniqueStudentGrades(string StudentID)
    {
        vector<clsGrade> v;
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return v;

        string sql =
            "SELECT StudentID, CourseCode, Assignment, Midterm, Final, Letter, Semester "
            "FROM Grades "
            "WHERE StudentID='" + _EscapeQuotes(StudentID) + "' "
            "GROUP BY CourseCode "
            "HAVING rowid = MAX(rowid);";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
                v.push_back(_ConvertRowToGrade(stmt));
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return v;
    }

    // ============================================================
    // Save 
    // ============================================================
    void Save()
    {
        if (_Letter != "W" && _Letter != "I") {
            int total = Grade();
            _Letter = GetLetterFromScore(total);
        }

        // 2. Perform DB Operation
        switch (_Mode)
        {
        case AddMode:
            _AddToDB();
            _Mode = UpdateMode;
            break;

        case UpdateMode:
            _UpdateInDB();
            break;

        case DeleteMode:
            _DeleteFromDB();
            break;
        }
    }

    bool IsEmpty() const
    {
        return (_Mode == EmptyMode || _StudentID == "");
    }

    int Grade() const
    {
        try {
            return std::stoi(_Assignment) + std::stoi(_Midterm) + std::stoi(_Final);
        }
        catch (...) {
            return 0;
        }
    }

};