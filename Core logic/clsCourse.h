#pragma once
#include "Global.h"
#include "clsString.h"
#include "clsTeacher.h"
#include "clsDB.h"
#include"sqlite3.h"

#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include "clsSchedule.h"
#include <algorithm>

using namespace std;

class clsCourse
{
private:

    string _CourseCode;
    string _CourseName;
    string _Instructor;
    string _Days;
    string _Time;
    string _Room;
    string _CreditHours;   
    string _Prerequisite;

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

    static string _SafeGetText(sqlite3_stmt* stmt, int col)
    {
        const unsigned char* txt = sqlite3_column_text(stmt, col);
        if (!txt)
            return "";
        return reinterpret_cast<const char*>(txt);
    }

    static clsCourse _GetEmptyCourseObject()
    {
        return clsCourse(EmptyMode, "", "", "", "", "", "", "", "");
    }

    static clsCourse _ConvertRowToCourse(sqlite3_stmt* stmt)
    {
        string code = _SafeGetText(stmt, 0);
        string name = _SafeGetText(stmt, 1);
        string instr = _SafeGetText(stmt, 2);
        string days = _SafeGetText(stmt, 3);
        string time = _SafeGetText(stmt, 4);
        string room = _SafeGetText(stmt, 5);
        int credit = sqlite3_column_int(stmt, 6);
        string prereq = _SafeGetText(stmt, 7);

        return clsCourse(
            UpdateMode,
            code, name, instr, days, time, room,
            to_string(credit), prereq
        );
    }

    // ================== Core DB operations ==================

    static vector<clsCourse> _LoadCoursesFromDB() // name kept, now loads from DB
    {
        vector<clsCourse> vCourses;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db)
            return vCourses;

        string sql =
            "SELECT CourseCode, CourseName, Instructor, Days, Time, Room, "
            "       CreditHours, Prerequisite "
            "FROM Courses;";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            if (stmt) sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return vCourses;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            vCourses.push_back(_ConvertRowToCourse(stmt));
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return vCourses;
    }

    void _Add() 
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        int credit = 0;
        try { credit = stoi(_CreditHours); }
        catch (...) { credit = 0; }

        string sql =
            "INSERT INTO Courses "
            "(CourseCode, CourseName, Instructor, Days, Time, Room, CreditHours, Prerequisite) "
            "VALUES ('" + _EscapeQuotes(_CourseCode) + "', '" +
            _EscapeQuotes(_CourseName) + "', '" +
            _EscapeQuotes(_Instructor) + "', '" +
            _EscapeQuotes(_Days) + "', '" +
            _EscapeQuotes(_Time) + "', '" +
            _EscapeQuotes(_Room) + "', " +
            to_string(credit) + ", '" +
            _EscapeQuotes(_Prerequisite) + "');";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    void _Update() // name kept, now updates DB
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        int credit = 0;
        try { credit = stoi(_CreditHours); }
        catch (...) { credit = 0; }

        string sql =
            "UPDATE Courses SET "
            "CourseName  = '" + _EscapeQuotes(_CourseName) + "', "
            "Instructor  = '" + _EscapeQuotes(_Instructor) + "', "
            "Days        = '" + _EscapeQuotes(_Days) + "', "
            "Time        = '" + _EscapeQuotes(_Time) + "', "
            "Room        = '" + _EscapeQuotes(_Room) + "', "
            "CreditHours = " + to_string(credit) + ", "
            "Prerequisite= '" + _EscapeQuotes(_Prerequisite) + "' "
            "WHERE CourseCode = '" + _EscapeQuotes(_CourseCode) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    void _Delete() // name kept, now deletes from DB
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return;

        string sql =
            "DELETE FROM Courses WHERE CourseCode = '" +
            _EscapeQuotes(_CourseCode) + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    // ---- helpers for validation ----

    static bool _IsDigits(const string& s)
    {
        for (int i = 0; i < (int)s.length(); i++)
        {
            if (!isdigit(s[i]))
                return false;
        }
        return true;
    }

    static string _NormalizeInstructorName(const string& Name)
    {
        string result = clsString::LowerAllString(clsString::Trim(Name));

        if (result.length() >= 2 &&
            result[0] == 'd' &&
            result[1] == 'r')
        {
            if (result.length() >= 3 && (result[2] == '.' || result[2] == ' '))
            {
                result = result.substr(3);
            }
            else
            {
                result = result.substr(2);
            }
            result = clsString::Trim(result);
        }

        return result;
    }

    static bool _IsValidDayInternal(const string& dayUpper)
    {
        string ValidDays[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

        for (int i = 0; i < 7; i++)
        {
            if (dayUpper == ValidDays[i])
                return true;
        }

        return false;
    }

    static int _GetDaySortValue(string Days)
    {
        if (Days.length() < 3) return 99;

        string d = clsString::UpperAllString(Days.substr(0, 3));

        if (d == "SUN") return 1;
        if (d == "MON") return 2;
        if (d == "TUE") return 3;
        if (d == "WED") return 4;
        if (d == "THU") return 5;
        if (d == "FRI") return 6;
        if (d == "SAT") return 7;
        return 99;
    }

    static int _GetTimeSortValue(string Time)
    {
        string clean = "";
        for (char c : Time) {
            if (c == ':') continue; // Skip colon
            if (c == '-') break;    // Stop at dash
            if (isdigit(c)) clean += c;
        }
        if (clean == "") return 9999;
        try { return stoi(clean); }
        catch (...) { return 9999; }
    }

public:

    enum enMode { EmptyMode = 0, UpdateMode = 1, AddMode = 2, DeleteMode = 3 };
    enum enSaveResult { svFaildEmptyObject = 0, svSucceeded = 1, svFailedAlreadyExists = 2 };


    clsCourse(string Code, string Name, string Instructor,
        string Days, string Time, string Room,
        string Credit, string Prereq)
    {
        _CourseCode = Code;
        _CourseName = Name;
        _Instructor = Instructor;
        _Days = Days;
        _Time = Time;
        _Room = Room;
        _CreditHours = Credit;
        _Prerequisite = Prereq;

        _Mode = UpdateMode; // treat loaded courses as updatable
    }

    clsCourse()
    {
        _CourseCode = "";
        _Mode = EmptyMode;
    }

    clsCourse(enMode Mode,
        string Code, string Name, string Instructor,
        string Days, string Time, string Room,
        string Credit, string Prereq)
    {
        _CourseCode = Code;
        _CourseName = Name;
        _Instructor = Instructor;
        _Days = Days;
        _Time = Time;
        _Room = Room;
        _CreditHours = Credit;
        _Prerequisite = Prereq;

        _Mode = Mode;
    }

    string Code() const { return _CourseCode; }
    string Name() const { return _CourseName; }
    string Instructor() const { return _Instructor; }
    string Days() const { return _Days; }
    string Time() const { return _Time; }
    string Room() const { return _Room; }
    string CreditHours() const { return _CreditHours; }
    string Prerequisite() const { return _Prerequisite; }

    void SetName(string v) { _CourseName = v; }
    void SetInstructor(string v) { _Instructor = v; }
    void SetDays(string v) { _Days = v; }
    void SetTime(string v) { _Time = v; }
    void SetRoom(string v) { _Room = v; }
    void SetCreditHours(string v) { _CreditHours = v; }
    void SetPrerequisite(string v) { _Prerequisite = v; }

    // ========== LOAD / FIND  ==========

    static vector<clsCourse> LoadAllCourses()
    {
        return _LoadCoursesFromDB(); 
    }

    static clsCourse FindCourse(string CourseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db)
            return _GetEmptyCourseObject();

        string sql =
            "SELECT CourseCode, CourseName, Instructor, Days, Time, Room, "
            "       CreditHours, Prerequisite "
            "FROM Courses "
            "WHERE UPPER(CourseCode) = UPPER('" + _EscapeQuotes(CourseCode) + "');";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            if (stmt) sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return _GetEmptyCourseObject();
        }

        clsCourse C = _GetEmptyCourseObject();

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            C = _ConvertRowToCourse(stmt);
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return C;
    }

    static bool IsCourseExist(string Code)
    {
        return !FindCourse(Code).IsEmpty();
    }

    static vector<clsCourse> GetCoursesList()
    {
        return LoadAllCourses();
    }

    bool IsEmpty() const { return (_Mode == EmptyMode || _CourseCode == ""); }

    // ========== SAVE() ==========

    enSaveResult Save()
    {
        if (_Mode == EmptyMode)
            return svFaildEmptyObject;

        switch (_Mode)
        {
        case AddMode:
            if (IsCourseExist(_CourseCode))
                return svFailedAlreadyExists;
            _Add();
            _Mode = UpdateMode;
            return svSucceeded;

        case UpdateMode:
            _Update();
            return svSucceeded;

        case DeleteMode:
            _Delete();
            return svSucceeded;
        }

        return svFaildEmptyObject;
    }

    // ================== ENROLLMENTS / GRADES (DB) ==================

    static vector<clsCourse> GetStudentCourses(string StudentID)
    {
        vector<clsCourse> vCourses;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db)
            return vCourses;

        string sql =
            "SELECT c.CourseCode, c.CourseName, c.Instructor, c.Days, c.Time, c.Room, "
            "       c.CreditHours, c.Prerequisite "
            "FROM Courses c "
            "INNER JOIN Enrollments e ON c.CourseCode = e.CourseCode "
            "WHERE e.StudentID = '" + _EscapeQuotes(StudentID) + "';";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            if (stmt) sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return vCourses;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            vCourses.push_back(_ConvertRowToCourse(stmt));
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return vCourses;
    }

    static bool IsStudentAlreadyEnrolled(string StudentID, string Code)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return false;

        string sql =
            "SELECT 1 FROM Enrollments "
            "WHERE StudentID = '" + _EscapeQuotes(StudentID) +
            "' AND UPPER(CourseCode) = UPPER('" + _EscapeQuotes(Code) + "') "
            "LIMIT 1;";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            if (stmt) sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return false;
        }

        bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return exists;
    }

    static int GetStudentTotalHours(string StudentID)
    {
        int total = 0;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db)
            return 0;

        string sql =
            "SELECT SUM(c.CreditHours) "
            "FROM Courses c "
            "INNER JOIN Enrollments e ON c.CourseCode = e.CourseCode "
            "WHERE e.StudentID = '" + _EscapeQuotes(StudentID) + "';";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            if (stmt) sqlite3_finalize(stmt);
            clsDB::CloseDB(db);
            return 0;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL)
                total = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return total;
    }

    static bool HasCompletedPrerequisite(string StudentID, string Pre)
    {
        if (Pre == "-" || Pre == "")
            return true;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return false;

        string sql =
            "SELECT 1 FROM Grades "
            "WHERE StudentID = '" + _EscapeQuotes(StudentID) + "' "
            "  AND CourseCode = '" + _EscapeQuotes(Pre) + "' "
            "  AND UPPER(Letter) NOT IN ('F', 'W', 'I', '-', '') "
            "LIMIT 1;";

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

        bool completed = false;
        if (rc == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
                completed = true;
        }

        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return completed;
    }

    static vector<clsCourse> GetEligibleCoursesForStudent(string StudentID)
    {
        vector<clsCourse> All = LoadAllCourses();
        vector<clsCourse> Eligible;

        for (clsCourse& Course : All)
        {
            if (!IsStudentAlreadyEnrolled(StudentID, Course.Code()) &&
                HasCompletedPrerequisite(StudentID, Course.Prerequisite()))
            {
                Eligible.push_back(Course);
            }
        }
        return Eligible;
    }

    // 1. Get Category Name
    string GetCategory()
    {
        string upperCode = clsString::UpperAllString(_CourseCode);

        if (upperCode.rfind("UNI", 0) == 0) return "University Mandatory";
        if (upperCode.rfind("ELECT", 0) == 0) return "University Elective";
        if (upperCode.rfind("SCI", 0) == 0) return "College Mandatory";

        return "Major Mandatory"; 
    }

    // 2. Boolean Helpers for Logic
    bool IsUniversityElective() { return (_CourseCode.find("ELECT") == 0); }
    bool IsUniversityMandatory() { return (_CourseCode.find("UNI") == 0); }

    // 3. Get University Mandatory (UNI%)
    static vector<clsCourse> GetUniversityMandatory()
    {
        vector<clsCourse> vList;
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return vList;
        string sql = "SELECT * FROM Courses WHERE CourseCode LIKE 'UNI%';";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) vList.push_back(_ConvertRowToCourse(stmt));
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return vList;
    }

    // 4. Get University Elective (ELECT%)
    static vector<clsCourse> GetUniversityElective()
    {
        vector<clsCourse> vList;
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return vList;
        string sql = "SELECT * FROM Courses WHERE CourseCode LIKE 'ELECT%';";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) vList.push_back(_ConvertRowToCourse(stmt));
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return vList;
    }

    // 5. Get College Mandatory (SCI%)
    static vector<clsCourse> GetCollegeRequirements()
    {
        vector<clsCourse> vList;
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return vList;
        string sql = "SELECT * FROM Courses WHERE CourseCode LIKE 'SCI%';";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) vList.push_back(_ConvertRowToCourse(stmt));
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return vList;
    }

    // 6. Get Major Mandatory (Smart Filter: SE, CS, CY, CIS)
    static vector<clsCourse> GetMajorRequirements(string StudentMajor)
    {
        vector<clsCourse> vList;
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return vList;

        string Prefix = "SE%"; // Default
        string UpperMajor = clsString::UpperAllString(StudentMajor);

        if (UpperMajor == "COMPUTER SCIENCE" || UpperMajor == "CS") Prefix = "CS%";
        else if (UpperMajor == "COMPUTER INFORMATION SYSTEMS" || UpperMajor == "CIS") Prefix = "CIS%";
        else if (UpperMajor == "CYBER SECURITY" || UpperMajor == "CY") Prefix = "CY%";
        else if (UpperMajor == "SOFTWARE ENGINEERING" || UpperMajor == "SE") Prefix = "SE%";

        string sql = "SELECT * FROM Courses WHERE CourseCode LIKE '" + Prefix + "';";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) vList.push_back(_ConvertRowToCourse(stmt));
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return vList;
    }

    static vector<clsCourse> GetCoursesByInstructor(string InstructorName)
    {
        vector<clsCourse> vCourses = LoadAllCourses();
        vector<clsCourse> InstructorCourses;

        string teacherName = clsString::LowerAllString(InstructorName);
        teacherName = clsString::Trim(teacherName);

        if (teacherName.rfind("dr.", 0) == 0)
            teacherName = clsString::Trim(teacherName.substr(3));

        vector<string> AddedCodes;

        for (clsCourse& C : vCourses)
        {
            string instructor = clsString::LowerAllString(C.Instructor());
            instructor = clsString::Trim(instructor);

            if (instructor.rfind("dr.", 0) == 0)
                instructor = clsString::Trim(instructor.substr(3));

            if (instructor == teacherName)
            {
                bool exists = false;
                for (string s : AddedCodes)
                {
                    if (clsString::UpperAllString(s) ==
                        clsString::UpperAllString(C.Code()))
                    {
                        exists = true;
                        break;
                    }
                }

                if (!exists)
                {
                    InstructorCourses.push_back(C);
                    AddedCodes.push_back(C.Code());
                }
            }
        }

        return InstructorCourses;
    }

    static vector<string> GetMyCourseCodes(string StudentID)
    {
        vector<string> codes;
        vector<clsCourse> My = GetStudentCourses(StudentID);

        for (clsCourse& C : My)
        {
            bool exists = false;

            for (string& s : codes)
            {
                if (clsString::UpperAllString(s) ==
                    clsString::UpperAllString(C.Code()))
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                codes.push_back(C.Code());
        }

        return codes;
    }

    static vector<string> GetAllRooms()
    {
        vector<string> rooms;
        vector<clsCourse> vCourses = LoadAllCourses();

        for (clsCourse& C : vCourses)
        {
            string room = clsString::Trim(C.Room());

            bool exists = false;
            for (string& r : rooms)
            {
                if (clsString::UpperAllString(r) == clsString::UpperAllString(room))
                {
                    exists = true;
                    break;
                }
            }

            if (!exists && room != "")
                rooms.push_back(room);
        }

        return rooms;
    }

    //VALIDATION HELPERS 

    static bool IsValidCourseCodeFormat(string Code)
    {
        const regex pattern(R"(^[A-Za-z]{2,4}\d{3}$)");
        return regex_match(Code, pattern);
    }

    static bool TryGetInstructorFormattedName(string InputName, string& OutFormattedName)
    {
        vector<clsTeacher> Teachers = clsTeacher::GetTeachersList();

        string normalizedInput = _NormalizeInstructorName(InputName);

        for (int i = 0; i < (int)Teachers.size(); i++)
        {
            clsTeacher T = Teachers[i];

            string fullNameNoPrefix = T.FirstName + " " + T.LastName;
            string normalizedTeacher = _NormalizeInstructorName(fullNameNoPrefix);

            if (normalizedTeacher == normalizedInput)
            {
                OutFormattedName = "Dr. " + T.FirstName + " " + T.LastName;
                return true;
            }
        }

        return false;
    }

    static bool IsValidCreditHours(string Hours)
    {
        return (Hours == "1" || Hours == "3");
    }

    static bool IsValidPrerequisiteCode(string Code)
    {
        if (Code == "-" || Code == "")
            return true;

        if (!IsCourseExist(Code))
            return false;

        return true;
    }

    static bool IsValidDaysString(string DaysInput)
    {
        string Input = clsString::UpperAllString(DaysInput);
        vector<string> vDays = clsString::Split(Input, ",");

        if (vDays.size() == 0)
            return false;

        for (int i = 0; i < (int)vDays.size(); i++)
        {
            string d = clsString::Trim(vDays[i]);
            if (!_IsValidDayInternal(d))
                return false;
        }

        return true;
    }

    static bool IsValidTimeFormat(string time)
    {
        size_t dashPos = time.find('-');
        if (dashPos == string::npos)
            return false;

        string from = time.substr(0, dashPos);
        string to = time.substr(dashPos + 1);

        size_t colon1 = from.find(':');
        size_t colon2 = to.find(':');

        if (colon1 == string::npos || colon2 == string::npos)
            return false;

        string h1 = from.substr(0, colon1);
        string m1 = from.substr(colon1 + 1);

        string h2 = to.substr(0, colon2);
        string m2 = to.substr(colon2 + 1);

        if (!_IsDigits(h1) || !_IsDigits(m1) || !_IsDigits(h2) || !_IsDigits(m2))
            return false;

        int hour1 = stoi(h1);
        int min1 = stoi(m1);
        int hour2 = stoi(h2);
        int min2 = stoi(m2);

        if (hour1 < 0 || hour1 > 23) return false;
        if (hour2 < 0 || hour2 > 23) return false;
        if (min1 < 0 || min1  > 59) return false;
        if (min2 < 0 || min2  > 59) return false;

        return true;
    }

    static bool IsValidRoomFormat(string room)
    {
        regex roomPattern(R"(^(Room|ROOM|Lab|LAB)-\d{1,3}$)");
        return regex_match(room, roomPattern);
    }

    static bool IsRoomExistsInSystem(string room)
    {
        vector<string> systemRooms = GetAllRooms();

        for (const string& R : systemRooms)
        {
            if (clsString::UpperAllString(R) == clsString::UpperAllString(room))
            {
                return true;
            }
        }

        return false;
    }

    static bool IsScheduleConflict(string StudentID, clsCourse& NewCourse)
    {
        // 1. Get current courses for this student
        vector<clsCourse> vCurrentCourses = GetStudentCourses(StudentID);

        if (vCurrentCourses.empty())
            return false;

        // 2. Calculate masks for the new course
        int newDayMask = clsSchedule::ParseDaysMask(NewCourse.Days());
        clsSchedule::stTimeRange newTime = clsSchedule::ParseTimeRange(NewCourse.Time());

        // 3. Compare with every existing course
        for (clsCourse& C : vCurrentCourses)
        {
            int curDayMask = clsSchedule::ParseDaysMask(C.Days());
            clsSchedule::stTimeRange curTime = clsSchedule::ParseTimeRange(C.Time());

            if (clsSchedule::HasConflict(newDayMask, newTime, curDayMask, curTime))
            {
                return true; // Conflict found!
            }
        }
        return false; // No conflict
    }

    static bool Register(string StudentID, string CourseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return false;

        // Standard Default Values
        int Assign = 0, Mid = 0, Fin = 0;
        string Letter = "-";
        string Sem = CurrentSemester;

        // 1. Insert into Grades (Transcript History)
        string sqlGrade = "INSERT INTO Grades (StudentID, CourseCode, Assignment, Midterm, Final, Letter, Semester) "
            "VALUES ('" + StudentID + "', '" + CourseCode + "', "
            + to_string(Assign) + ", " + to_string(Mid) + ", " + to_string(Fin) + ", '"
            + Letter + "', '" + Sem + "');";

        clsDB::Execute(db, sqlGrade);

        // 2. Insert into Enrollments 
        string sqlEnroll = "INSERT INTO Enrollments (StudentID, CourseCode) VALUES ('" + StudentID + "', '" + CourseCode + "');";

        bool result = clsDB::Execute(db, sqlEnroll);

        clsDB::CloseDB(db);

        return result;
    }

    static bool Unregister(string StudentID, string CourseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return false;

        // 1. Remove from Enrollments (Active List)
        string sqlEnroll = "DELETE FROM Enrollments WHERE StudentID = '" +
            _EscapeQuotes(StudentID) + "' AND CourseCode = '" + _EscapeQuotes(CourseCode) + "';";
        clsDB::Execute(db, sqlEnroll);

        string sqlGrade = "DELETE FROM Grades WHERE StudentID = '" + _EscapeQuotes(StudentID) +
            "' AND CourseCode = '" + _EscapeQuotes(CourseCode) +
            "' AND Semester = '" + CurrentSemester + "';";

        bool result = clsDB::Execute(db, sqlGrade);

        clsDB::CloseDB(db);
        return result;
    }
    static vector<string> GetStudentsInCourse(string CourseCode)
    {
        vector<string> vStudentIDs;

        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return vStudentIDs;

        string sql = "SELECT StudentID FROM Enrollments WHERE UPPER(CourseCode) = UPPER('" + _EscapeQuotes(CourseCode) + "');";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                string id = (const char*)sqlite3_column_text(stmt, 0);
                vStudentIDs.push_back(id);
            }
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);

        return vStudentIDs;
    }

    static bool IsCourseFull(string CourseCode)
    {
        int maxCapacity = GetMaxCapacity(CourseCode);

        if (maxCapacity == 0) return false;

        int currentCount = GetEnrolledCount(CourseCode);

        return (currentCount >= maxCapacity);
    }

    static int GetMaxCapacity(string CourseCode)
    {
        clsCourse Course = clsCourse::FindCourse(CourseCode);
        if (Course.IsEmpty()) return 0;

        if (Course.Prerequisite() == "" || Course.Prerequisite() == "-") {
            return 60;
        }
        return 30;
    }

    static int GetEnrolledCount(string CourseCode)
    {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (!db) return 0;

        string sql = "SELECT COUNT(*) FROM Enrollments WHERE UPPER(CourseCode) = UPPER('" + _EscapeQuotes(CourseCode) + "');";

        sqlite3_stmt* stmt = nullptr;
        int count = 0;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
        clsDB::CloseDB(db);
        return count;
    }

    static void SortCoursesBySchedule(vector<clsCourse>& vCourses)
    {
        std::sort(vCourses.begin(), vCourses.end(), [](clsCourse& A, clsCourse& B) {

            int dayA = _GetDaySortValue(A.Days());
            int dayB = _GetDaySortValue(B.Days());

            if (dayA != dayB) {
                return dayA < dayB; // Sort by Day first (Sun < Mon)
            }

            // If Days are same (e.g. both Mon), sort by Time
            return _GetTimeSortValue(A.Time()) < _GetTimeSortValue(B.Time());
            });
    }

    static bool IsTeacherHasScheduleConflict(string InstructorName, string NewDays, string NewTime, string IgnoreCourseCode = "")
    {
        vector<clsCourse> vTeacherCourses = GetCoursesByInstructor(InstructorName);

        int newDayMask = clsSchedule::ParseDaysMask(NewDays);
        clsSchedule::stTimeRange newTimeRange = clsSchedule::ParseTimeRange(NewTime);

        for (clsCourse& C : vTeacherCourses)
        {
            if (IgnoreCourseCode != "" && C.Code() == IgnoreCourseCode)
                continue;

            int curDayMask = clsSchedule::ParseDaysMask(C.Days());
            clsSchedule::stTimeRange curTimeRange = clsSchedule::ParseTimeRange(C.Time());

            if (clsSchedule::HasConflict(newDayMask, newTimeRange, curDayMask, curTimeRange))
            {
                return true; 
            }
        }
        return false; 
    }

    static int CountFutureUnlocks(string CourseCode)
    {
        vector<clsCourse> allCourses = LoadAllCourses();
        int unlockCount = 0;

        for (clsCourse& C : allCourses)
        {
            if (clsString::UpperAllString(C.Prerequisite()) == clsString::UpperAllString(CourseCode))
            {
                unlockCount++;
            }
        }
        return unlockCount;
    }

};

