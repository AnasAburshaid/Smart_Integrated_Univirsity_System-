#pragma once
#include <iostream>
#include <iomanip>
#include <vector>

#include "clsScreen.h"
#include "clsInputValidate.h"
#include "Global.h"
#include "clsCourse.h"
#include "clsSchedule.h"
#include "clsString.h"
#include "clsDB.h"
#include <sqlite3.h>

class clsRegisterNewCourseScreen : protected clsScreen
{
private:

    // ================================================================
    // CHECK FOR SCHEDULE CONFLICT
    // ================================================================
    static bool _HasScheduleConflict(clsCourse& NewCourse, vector<clsCourse>& vCurrentCourses)
    {
        if (vCurrentCourses.empty())
            return false;

        int newDayMask = clsSchedule::ParseDaysMask(NewCourse.Days());
        clsSchedule::stTimeRange newTime = clsSchedule::ParseTimeRange(NewCourse.Time());

        for (clsCourse& C : vCurrentCourses)
        {
            int curDayMask = clsSchedule::ParseDaysMask(C.Days());
            clsSchedule::stTimeRange curTime = clsSchedule::ParseTimeRange(C.Time());

            if (clsSchedule::HasConflict(newDayMask, newTime, curDayMask, curTime))
            {
                cout << "\nSchedule conflict with: " << C.Name()
                    << " (" << C.Days() << " - " << C.Time() << ")\n";
                return true;
            }
        }
        return false;
    }

    // ================================================================
    // SAVE ENROLLMENT (DB)
    // ================================================================
    static void _SaveEnrollment(string StudentID, string CourseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db)
        {
            cout << "\nFailed to open database for enrollment!\n";
            return;
        }

        string sql =
            "INSERT INTO Enrollments (StudentID, CourseCode) VALUES ('" +
            StudentID + "', '" + CourseCode + "');";

        if (!clsDB::Execute(db, sql))
        {
            cout << "\nError saving enrollment to database.\n";
        }

        clsDB::CloseDB(db);
    }

    // ================================================================
    // PRINT COURSE ROW
    // ================================================================
    static void _PrintEligibleCourse(clsCourse& C)
    {
        cout << left
            << setw(14) << C.Code()
            << setw(40) << C.Name()
            << setw(23) << C.Instructor()
            << setw(15) << C.Days()
            << setw(15) << C.Time()
            << setw(12) << C.CreditHours()
            << endl;
    }

    static void _PrintHeader()
    {
        cout << left
            << setw(14) << "Code"
            << setw(40) << "Course Name"
            << setw(23) << "Instructor"
            << setw(15) << "Days"
            << setw(15) << "Time"
            << setw(12) << "Hours"
            << endl;

        cout << string(115, '-') << endl;
    }

public:

    // ================================================================
    // MAIN SCREEN
    // ================================================================
    static void ShowRegisterNewCourse()
    {
        system("cls");
        _DrawScreenHeader("\tRegister New Course");

        vector<clsCourse> vEligible = clsCourse::GetEligibleCoursesForStudent(CurrentStudent.ID());
        vector<clsCourse> vCurrentCourses = clsCourse::GetStudentCourses(CurrentStudent.ID());

        cout << "\nStudent: " << CurrentStudent.FullName()
            << " (" << CurrentStudent.ID() << ")\n";

        cout << string(100, '=') << "\n";

        if (vEligible.empty())
        {
            cout << "\nThere are no available courses for registration.\n";
            system("pause>0");
            return;
        }

        // ================================
        // Print eligible course list
        // ================================
        cout << "\nAvailable Courses to Register:\n\n";
        _PrintHeader();

        for (clsCourse& C : vEligible)
            _PrintEligibleCourse(C);

        cout << string(115, '-') << endl;

        // ================================
        // Ask for course code
        // ================================
        cout << "\nEnter Course Code to Register: ";
        string courseCode = clsInputValidate::ReadString();
        courseCode = clsString::UpperAllString(clsString::Trim(courseCode));

        clsCourse Selected = clsCourse::FindCourse(courseCode);

        if (Selected.IsEmpty())
        {
            cout << "\nThere is no course with code (" << courseCode << ").\n";
            system("pause>0");
            return;
        }

        // ================================
        // Already enrolled?
        // ================================
        if (clsCourse::IsStudentAlreadyEnrolled(CurrentStudent.ID(), Selected.Code()))
        {
            cout << "\nYou are already enrolled in this course.\n";
            system("pause>0");
            return;
        }

        // ================================
        // Check prerequisite
        // ================================
        string pre = Selected.Prerequisite();

        if (pre != "" && pre != "-")
        {
            if (!clsCourse::HasCompletedPrerequisite(CurrentStudent.ID(), pre))
            {
                cout << "\nYou must complete prerequisite course (" << pre << ") first.\n";
                system("pause>0");
                return;
            }
        }

        // ================================
        // Check for schedule conflict
        // ================================
        if (_HasScheduleConflict(Selected, vCurrentCourses))
        {
            cout << "\nThere is a time conflict with your current schedule.\n";
            system("pause>0");
            return;
        }

        // ================================
        // Check for max hours (18 limit)
        // ================================
        int currentHours = clsCourse::GetStudentTotalHours(CurrentStudent.ID());
        int courseHours = stoi(Selected.CreditHours());

        if (currentHours + courseHours > 18)
        {
            cout << "\nYou cannot exceed 18 credit hours.\n";
            system("pause>0");
            return;
        }

        // ================================
        // Confirm registration
        // ================================
        cout << "\nStudent ID: " << CurrentStudent.ID()
            << "  -->  Course: " << Selected.Code();

        cout << "\nAre you sure you want to enroll in this course? (y/n): ";
        char confirm;
        cin >> confirm;

        if (toupper(confirm) == 'Y')
        {
            // Insert enrollment into DB
            _SaveEnrollment(CurrentStudent.ID(), Selected.Code());

            cout << "\nYou have successfully registered in ("
                << Selected.Name() << ")!\n";
        }

        system("pause>0");
    }
};
