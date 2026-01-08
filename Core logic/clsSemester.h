#pragma once
#include "Global.h"
#include "clsDB.h"
#include "clsDate.h" 
#include <string>
#include <iostream>

using namespace std;

class clsSemester
{
public:
    string Name;
    // Registration Dates (For enrolling - e.g. 2 weeks)
    string RegStartDate;
    string RegEndDate;

    // Semester Dates (For timeline & exams - e.g. 4 months)
    string SemStartDate;
    string SemEndDate;

    string WithdrawDeadline;

    // Flags
    bool GradingOpen = false;
    bool AllowAssignment = false;
    bool AllowMidterm = false;
    bool AllowFinal = false;

    // 1. Update Semester Info (Updates SEMESTER timeline, NOT Registration)
    static void Update(string NewName, string SemStart, string SemEnd, string Withdraw)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        // We update SemStartDate/SemEndDate here.
        string sql = "UPDATE Semester SET "
            "CurrentSemester = '" + NewName + "', "
            "SemStartDate = '" + SemStart + "', "
            "SemEndDate = '" + SemEnd + "', "
            "WithdrawDeadline = '" + Withdraw + "' "
            "WHERE ID = 1;";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);

        CurrentSemester = NewName;
    }

    // 2. Update Registration Dates (Separate Function)
    static void UpdateRegistrationDates(string RegStart, string RegEnd)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql = "UPDATE Semester SET "
            "RegStartDate = '" + RegStart + "', "
            "RegEndDate = '" + RegEnd + "' "
            "WHERE ID = 1;";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    // 3. Get Current Semester Info (Fetches ALL dates)
    static clsSemester GetCurrent()
    {
        clsSemester sem;
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return sem;

        // Fetch Old Columns + NEW Columns (SemStartDate, SemEndDate)
        string sql = "SELECT CurrentSemester, RegStartDate, RegEndDate, WithdrawDeadline, "
            "GradingOpen, AllowAssignment, AllowMidterm, AllowFinal, "
            "SemStartDate, SemEndDate FROM Semester WHERE ID = 1;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                auto txt = [&](int i) { const char* c = (const char*)sqlite3_column_text(stmt, i); return c ? string(c) : ""; };

                sem.Name = txt(0);
                sem.RegStartDate = txt(1);
                sem.RegEndDate = txt(2);
                sem.WithdrawDeadline = txt(3);

                sem.GradingOpen = (sqlite3_column_int(stmt, 4) == 1);
                sem.AllowAssignment = (sqlite3_column_int(stmt, 5) == 1);
                sem.AllowMidterm = (sqlite3_column_int(stmt, 6) == 1);
                sem.AllowFinal = (sqlite3_column_int(stmt, 7) == 1);

                sem.SemStartDate = txt(8);
                sem.SemEndDate = txt(9);

                if (sem.SemStartDate == "") sem.SemStartDate = sem.RegStartDate;
                if (sem.SemEndDate == "") sem.SemEndDate = sem.RegEndDate;
            }
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return sem;
    }

    // 4. Check Registration (Uses REG dates)
    static bool IsRegistrationOpen()
    {
        clsSemester sem = GetCurrent();
        if (sem.RegStartDate == "" || sem.RegEndDate == "") return true;

        clsDate today = clsDate::GetSystemDate();
        clsDate start(sem.RegStartDate);
        clsDate end(sem.RegEndDate);

        return (!clsDate::IsDateBeforeDate2(today, start) && !clsDate::IsDateAfterDate2(today, end));
    }

    // 5. Check Withdrawal (Uses WithdrawDeadline)
    static bool IsWithdrawAllowed()
    {
        clsSemester sem = GetCurrent();
        if (sem.WithdrawDeadline == "") return true;
        clsDate today = clsDate::GetSystemDate();
        clsDate deadline(sem.WithdrawDeadline);
        return (clsDate::IsDateBeforeDate2(today, deadline) || clsDate::IsDateEqualsDate2(today, deadline));
    }

    static void UpdateGradingRules(bool isOpen, bool allowAss, bool allowMid, bool allowFin)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;
        string sql = "UPDATE Semester SET GradingOpen = " + to_string(isOpen ? 1 : 0) + ", AllowAssignment = " + to_string(allowAss ? 1 : 0) + ", AllowMidterm = " + to_string(allowMid ? 1 : 0) + ", AllowFinal = " + to_string(allowFin ? 1 : 0) + " WHERE ID = 1;";
        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    static int GetSemesterOrder(string sem)
    {
        if (sem.length() < 5) return 0;
        string yearStr = sem.substr(sem.length() - 4);
        string season = sem.substr(0, sem.length() - 4);
        int year = 0;
        try { year = stoi(yearStr); }
        catch (...) { return 0; }
        int seasonOrder = 0;
        if (season == "Spring") seasonOrder = 1;
        else if (season == "Summer") seasonOrder = 2;
        else if (season == "Fall") seasonOrder = 3;
        return (year * 10) + seasonOrder;
    }
};