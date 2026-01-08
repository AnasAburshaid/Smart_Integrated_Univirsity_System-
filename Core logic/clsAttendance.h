#pragma once
#include "Global.h"
#include "clsDB.h"
#include "clsStudent.h"
#include "clsCourse.h"
#include "clsSystemAnnouncement.h"
#include "clsGrades.h" 
#include <string>
#include <vector>

using namespace std;

class clsAttendance
{
private:
    static bool _HasAnnouncement(string StudentID, string TitleToCheck)
    {
        vector<clsSystemAnnouncement> msgs = clsSystemAnnouncement::GetListFor(StudentID);
        for (auto& m : msgs)
        {
            if (m.Title() == TitleToCheck)
                return true;
        }
        return false;
    }

public:
    string CourseCode;
    string StudentID;
    string Date;
    string Status; 

    clsAttendance(string c, string s, string d, string st)
    {
        CourseCode = c;
        StudentID = s;
        Date = d;
        Status = st;
    }

    void Save()
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        // Insert Record
        string sql = "INSERT INTO Attendance (CourseCode, StudentID, Date, Status) VALUES ('" +
            CourseCode + "', '" + StudentID + "', '" + Date + "', '" + Status + "');";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);

        if (Status == "Absent")
        {
            CheckAbsenceRules(StudentID, CourseCode);
        }
    }

    // 2. Count Absences
    static int GetAbsenceCount(string StudentID, string CourseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return 0;

        string sql = "SELECT COUNT(*) FROM Attendance WHERE StudentID='" + StudentID +
            "' AND CourseCode='" + CourseCode + "' AND Status='Absent';";

        int count = 0;
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return count;
    }

    static void CheckAbsenceRules(string StudentID, string CourseCode)
    {
        int count = GetAbsenceCount(StudentID, CourseCode);

        if (count == 7)
        {
            string warnTitle = "Attendance Warning [" + CourseCode + "]";

            if (!_HasAnnouncement(StudentID, warnTitle))
            {
                clsSystemAnnouncement msg(
                    warnTitle,
                    "You have reached 7 absences in " + CourseCode + ". One more and you will FAIL (F).",
                    StudentID
                );
                msg.Save();
            }
        }

        if (count >= 8)
        {
            UpdateGradeToFail(StudentID, CourseCode);

            string failTitle = "Course Failed [" + CourseCode + "]";

            if (!_HasAnnouncement(StudentID, failTitle))
            {
                clsSystemAnnouncement msg(
                    failTitle,
                    "You have exceeded 8 absences in " + CourseCode + ". You have received an automatic 'F'.",
                    StudentID
                );
                msg.Save();
            }
        }
    }

    // Helper: Update Grade to 'F'
    static void UpdateGradeToFail(string StudentID, string CourseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql = "UPDATE Grades SET Letter='F' WHERE StudentID='" + StudentID +
            "' AND CourseCode='" + CourseCode + "' AND Letter='-';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    struct stRecord {
        string Date;
        string Status;
    };

    static vector<stRecord> GetHistory(string StudentID, string CourseCode)
    {
        vector<stRecord> vHistory;
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return vHistory;

        string sql = "SELECT Date, Status FROM Attendance WHERE StudentID='" + StudentID +
            "' AND CourseCode='" + CourseCode + "' ORDER BY rowid DESC;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                stRecord rec;
                rec.Date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                rec.Status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                vHistory.push_back(rec);
            }
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return vHistory;
    }
};