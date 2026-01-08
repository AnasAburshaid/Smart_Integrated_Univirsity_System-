#pragma once
#include "Global.h"
#include "clsDB.h"
#include "sqlite3.h"
#include <string>
#include <vector>

using namespace std;

class clsExam
{
public:
    string CourseCode;

    // Midterm Details
    string MidDate = "-";
    string MidTime = "-";
    string MidRoom = "-";

    // Final Details
    string FinalDate = "-";
    string FinalTime = "-";
    string FinalRoom = "-";


    clsExam(string code)
    { 
        CourseCode = code; 
    }

    clsExam(string code, string md, string mt, string mr, string fd, string ft, string fr)
    {
        CourseCode = code;
        MidDate = md; MidTime = mt; MidRoom = mr;
        FinalDate = fd; FinalTime = ft; FinalRoom = fr;
    }

    // 1. Find Exam Info for a Course

    static clsExam Find(string Code)
    {
        sqlite3* db = clsDB::OpenDB("University.db");

        clsExam exam(Code, "-", "-", "-", "-", "-", "-");

        if (!db) return exam;

        string sql = "SELECT MidDate, MidTime, MidRoom, FinalDate, FinalTime, FinalRoom FROM Exams WHERE CourseCode = '" + Code + "';";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                exam.MidDate = (const char*)sqlite3_column_text(stmt, 0);
                exam.MidTime = (const char*)sqlite3_column_text(stmt, 1);
                exam.MidRoom = (const char*)sqlite3_column_text(stmt, 2);
                exam.FinalDate = (const char*)sqlite3_column_text(stmt, 3);
                exam.FinalTime = (const char*)sqlite3_column_text(stmt, 4);
                exam.FinalRoom = (const char*)sqlite3_column_text(stmt, 5);
            }
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return exam;
    }

    // 2. Save Exam Info (Upsert: Insert or Update)
    void Save()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        // Try to update first
        string updateSQL = "UPDATE Exams SET "
            "MidDate='" + MidDate + "', MidTime='" + MidTime + "', MidRoom='" + MidRoom + "', "
            "FinalDate='" + FinalDate + "', FinalTime='" + FinalTime + "', FinalRoom='" + FinalRoom + "' "
            "WHERE CourseCode='" + CourseCode + "';";

        clsDB::Execute(db, updateSQL);

        if (sqlite3_changes(db) == 0)
        {
            string insertSQL = "INSERT INTO Exams (CourseCode, MidDate, MidTime, MidRoom, FinalDate, FinalTime, FinalRoom) "
                "VALUES ('" + CourseCode + "', '" + MidDate + "', '" + MidTime + "', '" + MidRoom + "', "
                "'" + FinalDate + "', '" + FinalTime + "', '" + FinalRoom + "');";
            clsDB::Execute(db, insertSQL);
        }

        clsDB::CloseDB(db);
    }
};