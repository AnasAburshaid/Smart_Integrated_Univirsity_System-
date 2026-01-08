#pragma once
#include "clsGrades.h"
#include "clsString.h"
#include "Global.h"
#include "clsDB.h"
#include "sqlite3.h"
#include <vector>
#include <iostream>

using namespace std;

class clsGradeService
{
private:
    static string _CalculateLetter(int total)
    {
        if (total >= 90) return "A";
        if (total >= 85) return "A-";
        if (total >= 80) return "B+";
        if (total >= 75) return "B";
        if (total >= 70) return "B-";
        if (total >= 65) return "C+";
        if (total >= 60) return "C";
        if (total >= 50) return "D";
        return "F";
    }

    static void _RemoveStudentFromEnrollment(const string& studentID, const string& courseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;
        string sql = "DELETE FROM Enrollments WHERE StudentID = '" + studentID + "' AND CourseCode = '" + courseCode + "';";
        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

public:
    enum enMark { Assignment = 1, Midterm = 2, Final = 3 };

    static bool AddOrEditGrade(const string& studentID, const string& courseCode, enMark markType, int grade)
    {
        // 1. LIMITS CHECK
        int maxLimit = 100;
        switch (markType) {
        case Assignment: maxLimit = 20; break;
        case Midterm:    maxLimit = 30; break;
        case Final:      maxLimit = 50; break;
        }
        if (grade < 0 || grade > maxLimit) {
            cout << "\nError: Invalid grade limit.\n";
            return false;
        }

        clsGrade G = clsGrade::Find(studentID, courseCode);

        // 2. SAME VALUE CHECK (Optimization & Protection)
        if (!G.IsEmpty()) {
            int currentVal = 0;
            try {
                if (markType == Assignment) currentVal = stoi(G.Assignment());
                else if (markType == Midterm) currentVal = stoi(G.Midterm());
                else if (markType == Final) currentVal = stoi(G.Final());
            }
            catch (...) { currentVal = 0; }

            if (currentVal == grade) return true;
        }

        int newAssign = 0, newMid = 0, newFinal = 0;

        if (!G.IsEmpty())
        {
            if (G.Semester() != "" && G.Semester() != CurrentSemester)
            {
                return true;
            }

            // 4. LOCK LOGIC 
            int currentFinal = 0;
            try { currentFinal = stoi(G.Final()); }
            catch (...) { currentFinal = 0; }

            if (G.Letter() != "-" && currentFinal > 0)
            {
                // If student is Locked, just return True (Success) to skip.
                return true;
            }

            try { newAssign = stoi(G.Assignment()); }
            catch (...) { newAssign = 0; }
            try { newMid = stoi(G.Midterm()); }
            catch (...) { newMid = 0; }
            try { newFinal = stoi(G.Final()); }
            catch (...) { newFinal = 0; }

            switch (markType) {
            case Assignment: newAssign = grade; break;
            case Midterm:    newMid = grade; break;
            case Final:      newFinal = grade; break;
            }

            if ((newAssign + newMid + newFinal) > 100) return false;

            string letter = "-";
            string semester = G.Semester();
            if (semester == "") semester = CurrentSemester;

            int total = newAssign + newMid + newFinal;

            if (newFinal > 0) {
                letter = _CalculateLetter(total);
                _RemoveStudentFromEnrollment(studentID, courseCode);
            }
            else {
                letter = "-";
            }

            clsGrade Updated(studentID, courseCode, to_string(newAssign), to_string(newMid), to_string(newFinal), letter, G.Semester(), clsGrade::UpdateMode);
            Updated.Save();
            return true;
        }

        switch (markType) {
        case Assignment: newAssign = grade; break;
        case Midterm:    newMid = grade; break;
        case Final:      newFinal = grade; break;
        }

        string initialLetter = "-";
        if (newFinal > 0) initialLetter = _CalculateLetter(newAssign + newMid + newFinal);

        clsGrade newGrade(studentID, courseCode, to_string(newAssign), to_string(newMid), to_string(newFinal), initialLetter, CurrentSemester, clsGrade::AddMode);
        newGrade.Save();
        return true;
    }
};