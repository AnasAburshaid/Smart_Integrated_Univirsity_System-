#include <sstream>   
#include "StudentController.h"
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <json/json.h> 
#include <set>
#include <cmath>
#include <iomanip>
#include <algorithm> 
#include "../core_logic/clsStudent.h" 
#include "../core_logic/clsCourse.h" 
#include "../core_logic/clsInputValidate.h" 
#include "../core_logic/clsGrades.h"
#include "../core_logic/clsAnnouncments.h"
#include "../core_logic/Global.h"
#include "../core_logic/clsDate.h" 
#include "../core_logic/clsSystemAnnouncement.h" 
#include "../core_logic/clsSemester.h"
#include "../core_logic/clsExam.h"
#include "../core_logic/clsAttendance.h"

// ======================================================================
// CORS HELPER (Paste this at the top of StudentController.cc)
// ======================================================================
static void addCors(const HttpResponsePtr& resp) {
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");
}

// ======================================================================
#pragma region Authentication_and_Profile
// ======================================================================

void StudentController::login(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    if (req->method() == Options) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k200OK);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");
        callback(resp);
        return;
    }

    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr) {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid or missing JSON input.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    std::string id = (*jsonPtr)["id"].asString();
    std::string password = (*jsonPtr)["password"].asString();

    clsStudent student = clsStudent::Find(id, password);

    if (student.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid student ID or password.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k401Unauthorized);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
    }
    else {
        responseJson["status"] = "success";

        // --- Student Data ---
        responseJson["data"]["id"] = student.ID();
        responseJson["data"]["fullName"] = student.FullName();
        responseJson["data"]["major"] = student.Major();
        responseJson["data"]["email"] = student.Email;
        responseJson["data"]["role"] = "Student";

        responseJson["data"]["token"] = "std_token_" + student.ID();

        auto resp = HttpResponse::newHttpJsonResponse(responseJson);

        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");

        callback(resp);
    }
}
// 2. GET PROFILE
void StudentController::getStudent(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId)
{
    clsStudent student = clsStudent::Find(studentId);
    Json::Value responseJson;

    if (student.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student profile not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        callback(resp);
        return;
    }
    vector<clsGrade> grades = clsGrade::GetStudentGrades(studentId);
    int finishedCredits = 0;

    for (clsGrade& G : grades) {
        if (G.Grade() >= 50) {
            clsCourse course = clsCourse::FindCourse(G.CourseCode());
            if (!course.IsEmpty()) {
                finishedCredits += stoi(course.CreditHours());
            }
        }
    }

    int totalRequired = 132; // Standard CS Degree
    int remaining = totalRequired - finishedCredits;
    if (remaining < 0) remaining = 0;
    double cgpa = clsGrade::CalculateCumulativeGPA(studentId);

    responseJson["status"] = "success";
    responseJson["data"]["id"] = student.ID();
    responseJson["data"]["fullName"] = student.FullName();
    responseJson["data"]["major"] = student.Major();
    responseJson["data"]["email"] = student.Email;
    responseJson["data"]["creditsRequired"] = totalRequired;
    responseJson["data"]["creditsFinished"] = finishedCredits;
    responseJson["data"]["creditsRemaining"] = remaining;
    responseJson["data"]["cgpa"] = cgpa;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    callback(resp);
}

// 3. UPDATE PROFILE INFO
void StudentController::updateInfo(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr) {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid JSON.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    if (!(*jsonPtr).isMember("studentId")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Student ID is required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    string id = (*jsonPtr)["studentId"].asString();
    clsStudent student = clsStudent::Find(id);

    if (student.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        callback(resp);
        return;
    }

    // --- VALIDATION CHECKS ---

    if ((*jsonPtr).isMember("email"))
    {
        string email = (*jsonPtr)["email"].asString();

        if (!clsInputValidate::IsValidEmail(email)) 
        {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Email Format.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        student.SetEmail(email);
    }

    if ((*jsonPtr).isMember("phone")) {
        string phone = (*jsonPtr)["phone"].asString();
        if (!clsInputValidate::IsValidJordanianPhone(phone)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Phone Format. Must be 07######## ";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        student.SetPhone(phone);
    }

    if (student.Save() == clsStudent::svSucceeded) 
    {
        responseJson["status"] = "success";
        responseJson["message"] = "Profile updated successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Database error.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    callback(resp);
}

// 4. CHANGE PASSWORD
void StudentController::updatePassword(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("studentId") ||
        !(*jsonPtr).isMember("oldPassword") ||
        !(*jsonPtr).isMember("newPassword") ||
        !(*jsonPtr).isMember("confirmPassword"))
    {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing fields. Required: studentId, oldPassword, newPassword, confirmPassword.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    string id = (*jsonPtr)["studentId"].asString();
    string oldPass = (*jsonPtr)["oldPassword"].asString();
    string newPass = (*jsonPtr)["newPassword"].asString();

    if (newPass.length() < 4) {
        responseJson["status"] = "fail";
        responseJson["message"] = "New Password too short. Must be at least 4 characters.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    string confirmPass = (*jsonPtr)["confirmPassword"].asString();

    clsStudent student = clsStudent::Find(id);

    if (student.IsEmpty() || student.Password() != oldPass) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Student ID or Old Password.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
        return;
    }

    if (newPass != confirmPass) {
        responseJson["status"] = "fail";
        responseJson["message"] = "New Password and Confirm Password do not match.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    student.SetPassword(newPass);

    if (student.Save() == clsStudent::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Password changed successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Database error.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    callback(resp);
}

#pragma endregion

// ======================================================================
#pragma region Academic_Records
// ======================================================================

// 5. GET GRADES (Clean Controller: Logic Moved to clsSemester)
void StudentController::getGrades(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId)
{
    Json::Value responseJson;

    // 1. Validate Student
    if (!clsStudent::IsStudentExist(studentId)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    // 2. Fetch Data
    vector<clsGrade> allGrades = clsGrade::GetStudentGrades(studentId);

    // 3. Identify and Sort Semesters Chronologically
    std::set<string> uniqueSemestersSet;
    for (clsGrade& G : allGrades) {
        if (G.Semester() != "") uniqueSemestersSet.insert(G.Semester());
    }

    if (!clsCourse::GetStudentCourses(studentId).empty()) {
        uniqueSemestersSet.insert(CurrentSemester);
    }

    vector<string> sortedSemesters(uniqueSemestersSet.begin(), uniqueSemestersSet.end());

    std::sort(sortedSemesters.begin(), sortedSemesters.end(), [](const string& a, const string& b) {
        return clsSemester::GetSemesterOrder(a) < clsSemester::GetSemesterOrder(b);
        });

    Json::Value semestersArray(Json::arrayValue);

    // 4. Running Variables for Cumulative Calculation
    std::map<string, clsGrade> runningCourseHistory;

    for (const string& semName : sortedSemesters) {
        Json::Value semesterObj;
        semesterObj["semesterName"] = semName;

        vector<clsGrade> semGrades;
        bool isSemesterComplete = true;

        for (clsGrade& G : allGrades) {
            if (G.Semester() == semName) {
                semGrades.push_back(G);
                if (G.Letter() == "-" || G.Letter() == "") isSemesterComplete = false;

                if (G.IsIncludedInGPA()) {
                    runningCourseHistory[G.CourseCode()] = G;
                }
            }
        }

        // --- A. CALCULATE SEMESTER GPA ---
        double semPoints = 0;
        double semHours = 0;

        Json::Value coursesInSem(Json::arrayValue);
        for (clsGrade& G : semGrades) {
            Json::Value gradeObj;
            gradeObj["courseCode"] = G.CourseCode();
            clsCourse C = clsCourse::FindCourse(G.CourseCode());
            gradeObj["courseName"] = C.Name();
            gradeObj["creditHours"] = C.CreditHours();

            int assignVal = 0, midVal = 0, finalVal = 0;
            try { assignVal = stoi(G.Assignment()); }
            catch (...) {}
            try { midVal = stoi(G.Midterm()); }
            catch (...) {}
            try { finalVal = stoi(G.Final()); }
            catch (...) {}

            gradeObj["assignment"] = assignVal;
            gradeObj["midterm"] = midVal;
            gradeObj["final"] = finalVal;
            gradeObj["practical"] = assignVal;
            gradeObj["total"] = assignVal + midVal + finalVal;
            gradeObj["grade"] = G.Letter();
            gradeObj["letter"] = G.Letter();

            coursesInSem.append(gradeObj);

            if (G.IsIncludedInGPA()) {
                semPoints += clsGrade::GradeToPoints(G.Letter()) * stod(C.CreditHours());
                semHours += stod(C.CreditHours());
            }
        }
        semesterObj["courses"] = coursesInSem;

        // --- B. CALCULATE RUNNING CUMULATIVE GPA ---
        double cumPoints = 0;
        double cumHours = 0;

        for (auto const& [code, G] : runningCourseHistory) {
            clsCourse C = clsCourse::FindCourse(code);
            cumPoints += clsGrade::GradeToPoints(G.Letter()) * stod(C.CreditHours());
            cumHours += stod(C.CreditHours());
        }

        // --- C. ASSIGN GPA VALUES ---
        if (isSemesterComplete && semHours > 0) {
            std::stringstream ssSem, ssCum;
            ssSem << std::fixed << std::setprecision(2) << (semPoints / semHours);
            ssCum << std::fixed << std::setprecision(2) << (cumPoints / cumHours);

            semesterObj["semesterGPA"] = ssSem.str();
            semesterObj["cumulativeGPA"] = ssCum.str();
        }
        else {
            semesterObj["semesterGPA"] = "-";
            semesterObj["cumulativeGPA"] = "-";
        }

        semestersArray.append(semesterObj);
    }

    // Smart Warning Logic 
    double finalCGPA = clsGrade::CalculateCumulativeGPA(studentId);
    if (finalCGPA > 0 && finalCGPA < 2.0) {
        responseJson["academicStatus"] = "Probation";
        responseJson["hasWarning"] = true;
    }
    else {
        responseJson["academicStatus"] = "Good Standing";
        responseJson["hasWarning"] = false;
    }

    responseJson["status"] = "success";
    responseJson["data"] = semestersArray;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}
#pragma endregion


// ======================================================================
#pragma region Course_Registration
// ======================================================================

// 6. GET MY COURSES
void StudentController::getMyCourses(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId)
{
    vector<clsCourse> courses = clsCourse::GetStudentCourses(studentId);
    Json::Value responseJson;

    if (courses.empty()) {
        responseJson["status"] = "success";
        responseJson["message"] = "No courses found for this student.";
        responseJson["data"] = Json::arrayValue;
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
        return;
    }

    Json::Value coursesArray(Json::arrayValue);
    for (clsCourse& C : courses) {
        Json::Value courseObj;
        courseObj["code"] = C.Code();
        courseObj["name"] = C.Name();
        courseObj["instructor"] = C.Instructor();
        courseObj["room"] = C.Room();
        courseObj["days"] = C.Days();
        courseObj["time"] = C.Time();
        courseObj["creditHours"] = C.CreditHours();

        // --- NEW: Add Absence Count ---
        int abs = clsAttendance::GetAbsenceCount(studentId, C.Code());
        courseObj["absences"] = abs;
        // ------------------------------

        coursesArray.append(courseObj);
    }

    responseJson["status"] = "success";
    responseJson["data"] = coursesArray;
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    callback(resp);
}

// GET AVAILABLE COURSES (Filtered: Hides Passed & Active Courses)
void StudentController::getAvailableCourses(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId)
{
    Json::Value responseJson;

    if (!clsSemester::IsRegistrationOpen()) {
        responseJson["status"] = "fail"; 
        responseJson["code"] = "REG_CLOSED";
        responseJson["message"] = "Registration is closed.";
        responseJson["data"] = Json::arrayValue; 

        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp);
        callback(resp);
        return;
    }

    if (!clsStudent::IsStudentExist(studentId)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    // 1. Get all courses the student is eligible for (Prerequisites met)
    vector<clsCourse> eligibleCourses = clsCourse::GetEligibleCoursesForStudent(studentId);

    vector<clsGrade> myGrades = clsGrade::GetStudentGrades(studentId);
    std::set<string> hiddenCourses;

    for (clsGrade& G : myGrades) {

        if (G.Letter() == "-" || G.Letter() == "" || G.Semester() == CurrentSemester) {
            hiddenCourses.insert(G.CourseCode());
            continue;
        }

        string L = G.Letter();
        if (L == "A+" || L == "A" || L == "A-" ||
            L == "B+" || L == "B" || L == "B-" ||
            L == "C+")
        {
            hiddenCourses.insert(G.CourseCode());
        }
    }

    Json::Value coursesArray(Json::arrayValue);

    for (clsCourse& C : eligibleCourses) {

        if (hiddenCourses.count(C.Code())) continue;

        int current = clsCourse::GetEnrolledCount(C.Code());
        int max = clsCourse::GetMaxCapacity(C.Code());
        bool isFull = (current >= max);

        bool recommended = false;
        int unlocks = clsCourse::CountFutureUnlocks(C.Code());

        if (unlocks > 0) recommended = true;
        else if (C.GetCategory() == "Major Mandatory") recommended = true;

        Json::Value obj;
        obj["code"] = C.Code();
        obj["name"] = C.Name();
        obj["creditHours"] = C.CreditHours();
        obj["instructor"] = C.Instructor();
        obj["category"] = C.GetCategory();
        obj["days"] = C.Days();
        obj["time"] = C.Time();
        obj["enrolledCount"] = current;
        obj["maxCapacity"] = max;
        obj["isFull"] = isFull;
        obj["isRecommended"] = recommended;

        coursesArray.append(obj);
    }

    responseJson["status"] = "success";
    responseJson["count"] = coursesArray.size();
    responseJson["data"] = coursesArray;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");
    callback(resp);
}


// 8. GET RECOMMENDED COURSES (Strict: New Courses Only + Unlockers First)
void StudentController::getRecommendedCourses(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId)
{
    Json::Value responseJson;

    if (!clsSemester::IsRegistrationOpen()) {
        responseJson["status"] = "fail";
        responseJson["code"] = "REG_CLOSED";
        responseJson["message"] = "Registration is closed.";
        responseJson["data"] = Json::arrayValue;

        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp);
        callback(resp);
        return;
    }

    if (!clsStudent::IsStudentExist(studentId)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
        return;
    }

    vector<clsGrade> myGrades = clsGrade::GetStudentGrades(studentId);

    int completedElectiveHours = 0;
    std::set<string> hiddenCourses;

    for (clsGrade& G : myGrades) {
        string L = G.Letter();
        if (L != "W") {
            hiddenCourses.insert(G.CourseCode());
        }

        // 2. Count Elective Hours (to cap them at 6)
        if (G.Grade() >= 50 || L == "A" || L == "B" || L == "C" || L == "D") {
            clsCourse c = clsCourse::FindCourse(G.CourseCode());
            if (c.IsUniversityElective()) {
                completedElectiveHours += stoi(c.CreditHours());
            }
        }
    }

    vector<clsCourse> eligibleCourses = clsCourse::GetEligibleCoursesForStudent(studentId);

    struct WeightedCourse {
        clsCourse C;
        int Score;
        string Reason;
        string PriorityLabel;
    };

    vector<WeightedCourse> prioritizedList;

    for (clsCourse& C : eligibleCourses) {

        if (hiddenCourses.count(C.Code())) continue;

        if (clsCourse::IsCourseFull(C.Code())) continue;

        if (C.IsUniversityElective() && completedElectiveHours >= 6) continue;

        int score = 0;
        string reason = "";
        string priority = "Normal";

        int unlocks = clsCourse::CountFutureUnlocks(C.Code());
        string cat = C.GetCategory();

        if (unlocks > 0) {
            score += (unlocks * 100);
            reason = "High Priority: Unlocks " + to_string(unlocks) + " future courses.";
            priority = "Critical";
        }


        if (cat == "College Mandatory") {
            score += 40;
            if (reason == "") reason = "Foundational Course (College Requirement).";
            if (priority == "Normal") priority = "High";
        }

        // Major Mandatory (Core IT/CS subjects) 
        else if (cat == "Major Mandatory") {
            score += 30;
            if (reason == "") reason = "Core Major Requirement.";
        }

        // University Mandatory (General stuff) 
        else if (cat == "University Mandatory") {
            score += 20;
            if (reason == "") reason = "General University Requirement.";
        }

        // Electives (Fillers) -> Score +10
        else if (C.IsUniversityElective()) {
            score += 10;
            if (reason == "") reason = "Elective: Good option to fill credit hours.";
            priority = "Low";
        }

        prioritizedList.push_back({ C, score, reason, priority });
    }

    std::sort(prioritizedList.begin(), prioritizedList.end(), [](const WeightedCourse& a, const WeightedCourse& b) {
        return a.Score > b.Score;
        });

    Json::Value recArray(Json::arrayValue);
    int currentHours = 0;
    int MAX_HOURS = 18;

    for (auto& item : prioritizedList) {
        int hours = 3;
        try { hours = stoi(item.C.CreditHours()); }
        catch (...) {}

        if (currentHours + hours <= MAX_HOURS) {
            currentHours += hours;

            Json::Value obj;
            obj["code"] = item.C.Code();
            obj["name"] = item.C.Name();
            obj["creditHours"] = item.C.CreditHours();
            obj["priority"] = item.PriorityLabel;
            obj["reason"] = item.Reason;
            obj["category"] = item.C.GetCategory();

            recArray.append(obj);
        }
    }

    responseJson["status"] = "success";
    responseJson["totalRecommendedHours"] = currentHours;
    responseJson["data"] = recArray;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}
// 9. REGISTER COURSE 
void StudentController::registerCourse(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!clsSemester::IsRegistrationOpen()) {
        responseJson["status"] = "fail";
        responseJson["code"] = "REG_CLOSED"; 
        responseJson["message"] = "Registration is CLOSED.";

        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }

    if (!jsonPtr) {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (!(*jsonPtr).isMember("studentId") || !(*jsonPtr).isMember("courseCode")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing studentId or courseCode";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string studentId = (*jsonPtr)["studentId"].asString();
    string courseCode = (*jsonPtr)["courseCode"].asString();

    // --- 1. TIMELINE CHECK ---
    if (!clsSemester::IsRegistrationOpen()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Registration is CLOSED. Please check academic calendar.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }

    clsStudent student = clsStudent::Find(studentId);
    clsCourse course = clsCourse::FindCourse(courseCode);

    if (student.IsEmpty() || course.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Student ID or Course Code.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    vector<clsCourse> myCurrentCourses = clsCourse::GetStudentCourses(studentId);
    int currentTotalHours = 0;

    for (auto& c : myCurrentCourses) {
        currentTotalHours += stoi(c.CreditHours());
    }

    int newCourseHours = stoi(course.CreditHours());

    if (currentTotalHours + newCourseHours > 18) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Registration Failed: You cannot exceed 18 Credit Hours. (Current: " + to_string(currentTotalHours) + ")";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict); 
        addCors(resp); callback(resp); return;
    }

    // --- 3. SMART RETAKE CHECK ---
    clsGrade lastAttempt = clsGrade::Find(studentId, courseCode);

    if (!lastAttempt.IsEmpty())
    {
        if (lastAttempt.Semester() == CurrentSemester)
        {
            responseJson["status"] = "fail";
            responseJson["message"] = "You are already registered for this course this semester.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            addCors(resp); callback(resp); return;
        }

        string L = lastAttempt.Letter();
        if (L == "A+" || L == "A" || L == "A-" ||
            L == "B+" || L == "B" || L == "B-" ||
            L == "C+")
        {
            responseJson["status"] = "fail";
            responseJson["message"] = "You cannot retake this course because you passed with a good grade (" + L + ").";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            addCors(resp); callback(resp); return;
        }
    }

    // --- 4. CONFLICT & CAPACITY CHECKS ---
    if (clsCourse::IsScheduleConflict(studentId, course)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Schedule Conflict: Overlaps with another course.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict);
        addCors(resp); callback(resp); return;
    }

    if (clsCourse::IsCourseFull(courseCode))
    {
        vector<clsGrade> myGrades = clsGrade::GetStudentGrades(studentId);
        int finishedCredits = 0;

        for (clsGrade& G : myGrades) {
            int gVal = 0;
            try { gVal = stoi(G.Final()) + stoi(G.Midterm()) + stoi(G.Assignment()); }
            catch (...) { gVal = 0; }

            if (gVal >= 50) {
                clsCourse c = clsCourse::FindCourse(G.CourseCode());
                finishedCredits += stoi(c.CreditHours());
            }
        }

        if (finishedCredits < 112)
        {
            responseJson["status"] = "fail";
            responseJson["message"] = "Registration Failed: Course is Full. (Only graduating students can override).";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k409Conflict);
            addCors(resp); callback(resp); return;
        }
    }

    // --- 5. ELECTIVE LIMIT (Max 6 Hours) ---
    if (course.IsUniversityElective())
    {
        int currentElectiveHours = 0;
        for (auto& myC : myCurrentCourses) { 
            if (myC.IsUniversityElective()) {
                currentElectiveHours += stoi(myC.CreditHours());
            }
        }

        if (currentElectiveHours + newCourseHours > 6)
        {
            responseJson["status"] = "fail";
            responseJson["message"] = "Registration Failed: You can only register 6 hours of University Electives.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k403Forbidden);
            addCors(resp); callback(resp); return;
        }
    }

    if (student.CanRegisterCourse(course))
    {
        if (clsCourse::Register(studentId, courseCode)) {
            responseJson["status"] = "success";
            responseJson["message"] = "Successfully registered course " + course.Name();
        }
        else {
            responseJson["status"] = "error";
            responseJson["message"] = "Database error.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k500InternalServerError);
            addCors(resp); callback(resp); return;
        }
    }
    else
    {
        responseJson["status"] = "fail";
        responseJson["message"] = "Cannot register: Prerequisite missing or max hours exceeded.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}
// 10. DROP COURSE
void StudentController::dropCourse(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr) {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (!(*jsonPtr).isMember("studentId") || !(*jsonPtr).isMember("courseCode")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing studentId or courseCode";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string studentId = (*jsonPtr)["studentId"].asString();
    string courseCode = (*jsonPtr)["courseCode"].asString();

    // 1. Check Withdrawal Period
    if (!clsSemester::IsWithdrawAllowed()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Drop/Withdrawal period is CLOSED.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }

    // 2. Check Enrollment
    if (!clsCourse::IsStudentAlreadyEnrolled(studentId, courseCode)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student is not enrolled in this course.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict);
        addCors(resp); callback(resp); return;
    }

    // 3. Clean Delete (Enrollments + Grades)
    sqlite3* db = clsDB::OpenDB("University.db");
    if (!db) {
        responseJson["status"] = "error";
        responseJson["message"] = "Database connection error.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    // A. Remove from Active Enrollments
    string sqlEnroll = "DELETE FROM Enrollments WHERE StudentID = '" + studentId + "' AND CourseCode = '" + courseCode + "';";
    clsDB::Execute(db, sqlEnroll);

    string sqlGrade = "DELETE FROM Grades WHERE StudentID = '" + studentId + "' AND CourseCode = '" + courseCode + "' AND Letter = '-';";
    clsDB::Execute(db, sqlGrade);

    clsDB::CloseDB(db);

    responseJson["status"] = "success";
    responseJson["message"] = "Successfully dropped course.";

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}
// ======================================================================
// 13. WITHDRAW COURSE (Records a 'W')
// ======================================================================
void StudentController::withdrawCourse(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("studentId") || !(*jsonPtr).isMember("courseCode")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing fields: studentId, courseCode.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    string studentId = (*jsonPtr)["studentId"].asString();
    string courseCode = (*jsonPtr)["courseCode"].asString();

    // 1. Check if student is actually enrolled
    // (We reuse Find to check the LATEST enrollment)
    clsGrade currentGrade = clsGrade::Find(studentId, courseCode);

    if (currentGrade.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "You are not enrolled in this course.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
        return;
    }

    if (clsSemester::IsRegistrationOpen()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Registration is still OPEN. Use 'Drop' to remove the course.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict);
        callback(resp);
        return;
    }

    if (!clsSemester::IsWithdrawAllowed()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Withdrawal period is CLOSED. The deadline has passed.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);
        callback(resp);
        return;
    }

    // 2. Prevent double withdraw
    if (currentGrade.Letter() == "W") {
        responseJson["status"] = "fail";
        responseJson["message"] = "You have already withdrawn from this course.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        callback(resp);
        return;
    }

    // 3. Perform Withdraw: Set Marks to 0 and Letter to 'W'
    sqlite3* db = clsDB::OpenDB("University.db");
    if (db) {
        // Update the SPECIFIC row for this semester
        string sql = "UPDATE Grades SET "
            "Assignment=0, Midterm=0, Final=0, Letter='W' "
            "WHERE StudentID = '" + studentId + "' "
            "AND CourseCode = '" + courseCode + "' "
            "AND Semester = '" + currentGrade.Semester() + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    responseJson["status"] = "success";
    responseJson["message"] = "Successfully withdrawn. Grade recorded as 'W'.";

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    callback(resp);
}

void StudentController::getRegistrationStatus(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    clsSemester sem = clsSemester::GetCurrent();
    bool isOpen = clsSemester::IsRegistrationOpen();

    responseJson["status"] = "success";
    responseJson["data"]["isOpen"] = isOpen; 
    responseJson["data"]["startDate"] = sem.RegStartDate;
    responseJson["data"]["endDate"] = sem.RegEndDate;

    if (isOpen) {
        responseJson["data"]["message"] = "Registration is currently OPEN.";
    }
    else {
        responseJson["data"]["message"] = "Registration is CLOSED.";
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp);
    callback(resp);
}
#pragma endregion

// ======================================================================
#pragma region Announcements_and_System
// ======================================================================

// 11. GET ANNOUNCEMENTS (Course + System)
void StudentController::getAnnouncements(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId)
{
    Json::Value announcementsArray(Json::arrayValue);

    // Get Student's Courses
    vector<clsCourse> myCourses = clsCourse::GetStudentCourses(studentId);
    // Get All Course Announcements
    vector<clsAnnouncement> allAnnouncements = clsAnnouncement::GetAnnouncementsData();

    for (clsAnnouncement& A : allAnnouncements) {
        for (clsCourse& C : myCourses) {
            string annCode = A.CourseCode();
            string myCode = C.Code();
            for (char& c : annCode) c = toupper(c);
            for (char& c : myCode) c = toupper(c);

            if (annCode == myCode) {
                Json::Value obj;
                obj["courseCode"] = A.CourseCode();
                obj["teacher"] = A.TeacherName();
                obj["title"] = A.Title();
                obj["message"] = A.Message();
                obj["date"] = A.Date();
                obj["type"] = "Course";
                announcementsArray.append(obj);
                break;
            }
        }
    }

    Json::Value responseJson;
    responseJson["status"] = "success";
    responseJson["data"] = announcementsArray;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}

// 12. GET SYSTEM ANNOUNCEMENTS 

void StudentController::getSystemAnnouncements(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId)
{
    Json::Value responseJson;
    Json::Value messagesArray(Json::arrayValue);

    if (!clsStudent::IsStudentExist(studentId)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    // ---------------------------------------------------------
    // PART A: SEMESTER REWARDS 
    // ---------------------------------------------------------
    vector<clsGrade> allGrades = clsGrade::GetStudentGrades(studentId);
    std::set<string> uniqueSemesters;

    for (clsGrade& G : allGrades) {
        if (G.Semester() != "") uniqueSemesters.insert(G.Semester());
    }

    for (const string& semName : uniqueSemesters) {
        if (semName == CurrentSemester) continue;

        double semGPA = clsGrade::CalculateGPA(studentId, semName);

        if (semGPA >= 3.75) {
            Json::Value msg;
            msg["type"] = "Reward";
            msg["title"] = "Academic Excellence Award";
            msg["date"] = semName;
            std::stringstream ss; ss << std::fixed << std::setprecision(2) << semGPA;
            msg["message"] = "Outstanding! You achieved a GPA of " + ss.str() + " in " + semName +
                ". You have earned a 100% Discount on an external course! Contact Admin to claim.";
            messagesArray.append(msg);
        }
        else if (semGPA >= 3.50) {
            Json::Value msg;
            msg["type"] = "Reward";
            msg["title"] = "Honor Roll Reward";
            msg["date"] = semName;
            std::stringstream ss; ss << std::fixed << std::setprecision(2) << semGPA;
            msg["message"] = "Great job! You achieved a GPA of " + ss.str() + " in " + semName +
                ". You have earned a 50% Discount on an external course! Contact Admin to claim.";
            messagesArray.append(msg);
        }
    }

    // ---------------------------------------------------------
    // PART B: ACADEMIC WARNINGS (Calculated on the Fly) - [KEPT INTACT]
    // ---------------------------------------------------------
    double cgpa = clsGrade::CalculateCumulativeGPA(studentId);
    if (cgpa > 0.0 && cgpa < 2.0) {
        Json::Value msg;
        msg["type"] = "Alert";
        msg["title"] = "Academic Warning";
        msg["date"] = "Urgent Action";
        msg["message"] = "Your Cumulative GPA is " + to_string(cgpa).substr(0, 4) + ". Please see your advisor immediately.";
        messagesArray.append(msg);
    }

    // ---------------------------------------------------------
    // PART C: GLOBAL ADMIN ANNOUNCEMENTS (Target = "Student") - [KEPT INTACT]
    // ---------------------------------------------------------
    vector<clsSystemAnnouncement> adminMsgs = clsSystemAnnouncement::GetListFor("Student");
    for (clsSystemAnnouncement& A : adminMsgs) {
        Json::Value msg;
        msg["type"] = "Admin Announcement";
        msg["title"] = A.Title();
        msg["date"] = A.Date();
        msg["message"] = A.Message();
        messagesArray.append(msg);
    }

    // ---------------------------------------------------------
    // PART D: PERSONAL NOTIFICATIONS (Target = "3220xxxx") - [KEPT INTACT]
    // ---------------------------------------------------------
    vector<clsSystemAnnouncement> personalMsgs = clsSystemAnnouncement::GetListFor(studentId);
    for (clsSystemAnnouncement& A : personalMsgs) {
        Json::Value msg;
        msg["type"] = "Personal Notification";
        msg["title"] = A.Title();
        msg["date"] = A.Date();
        msg["message"] = A.Message();
        messagesArray.append(msg);
    }

    // ---------------------------------------------------------
    // PART E: GRADUATION BLOCK CHECK 
    // ---------------------------------------------------------
    int finishedCredits = 0;
    for (clsGrade& G : allGrades) {
        int total = 0;
        try { total = stoi(G.Final()) + stoi(G.Midterm()) + stoi(G.Assignment()); }
        catch (...) {}

        if (total >= 50) {
            clsCourse c = clsCourse::FindCourse(G.CourseCode());
            if (!c.IsEmpty()) finishedCredits += stoi(c.CreditHours());
        }
    }

    if (finishedCredits >= 132 && cgpa < 2.0) {
        Json::Value msg;
        msg["type"] = "Alert";
        msg["title"] = "Graduation Blocked";
        msg["date"] = "Urgent";
        msg["message"] = "CRITICAL: You have finished 132 credit hours, but your CGPA is " + to_string(cgpa).substr(0, 4) +
            ". You CANNOT graduate with a GPA below 2.0. You must retake courses to raise your average.";
        messagesArray.append(msg);
    }

    responseJson["status"] = "success";
    responseJson["data"] = messagesArray;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}
// 14. GET MY EXAMS
void StudentController::getMyExams(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId) 
{
    // 1. Get Courses
    vector<clsCourse> myCourses = clsCourse::GetStudentCourses(studentId);
    Json::Value examList(Json::arrayValue);

    // 2. Loop and find exam info
    for (clsCourse& C : myCourses) {
        clsExam E = clsExam::Find(C.Code());

        Json::Value obj;
        obj["courseCode"] = C.Code();
        obj["courseName"] = C.Name();

        // Midterm Info
        obj["midDate"] = E.MidDate;
        obj["midTime"] = E.MidTime;
        obj["midRoom"] = E.MidRoom;

        // Final Info
        obj["finalDate"] = E.FinalDate;
        obj["finalTime"] = E.FinalTime;
        obj["finalRoom"] = E.FinalRoom;

        examList.append(obj);
    }

    Json::Value responseJson;
    responseJson["status"] = "success";
    responseJson["data"] = examList;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    callback(resp);
}

// ==========================================================
// SCHOLARSHIP & INTELLIGENT SUGGESTIONS
// ==========================================================
void StudentController::getScholarshipStatus(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId)
{
    Json::Value responseJson;

    // 1. Get Student Info
    clsStudent student = clsStudent::Find(studentId);
    if (student.IsEmpty()) {
        responseJson["status"] = "error";
        responseJson["message"] = "Student not found";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    // 2. Parse GPA & ROUND IT
    double rawGpa = clsGrade::CalculateCumulativeGPA(studentId);
    double gpa = std::round(rawGpa * 100.0) / 100.0;

    // 3. Status Logic
    string status = "";
    string message = "";
    string color = "";
    string emailSubject = "";

    if (gpa > 3.60) {
        status = "Safe";
        message = "Excellent! Your scholarship is secure.";
        color = "green";
        emailSubject = "Scholarship Status: Excellent Standing";
    }
    else if (gpa >= 3.50 && gpa <= 3.60) {
        status = "Warning";
        message = "Careful! You are in the Danger Zone (3.50 - 3.60).";
        color = "orange";
        emailSubject = "Scholarship Warning: Action Required";
    }
    else {
        status = "Not Qualified";
        message = "You don't have a scholarship.";
        color = "red";
        emailSubject = "Scholarship Status: Not Qualified";
    }

    responseJson["gpa"] = gpa;
    responseJson["status"] = status;
    responseJson["message"] = message;
    responseJson["badgeColor"] = color;

    // 4. Analyze Grades for Suggestions 
    vector<clsGrade> allGrades = clsGrade::GetStudentGrades(studentId);

    // Helper to calc total (for display)
    auto calcTotal = [](clsGrade& g) -> int {
        try {
            int m = (g.Midterm() == "-" || g.Midterm() == "") ? 0 : stoi(g.Midterm());
            int p = (g.Assignment() == "-" || g.Assignment() == "") ? 0 : stoi(g.Assignment());
            int f = (g.Final() == "-" || g.Final() == "") ? 0 : stoi(g.Final());
            return m + p + f;
        }
        catch (...) { return 0; }
        };

    std::sort(allGrades.begin(), allGrades.end(), [&](clsGrade a, clsGrade b) {
        double pA = clsGrade::GradeToPoints(a.Letter());
        double pB = clsGrade::GradeToPoints(b.Letter());

        // If the GPA points are different, prioritize the lower one (Worse grade)
        if (abs(pA - pB) > 0.001) {
            return pA < pB;
        }
        return calcTotal(a) < calcTotal(b);
        });

    // Build the suggestion list
    stringstream adviceBody;
    adviceBody << "Current GPA: " << gpa << "\n\n";
    adviceBody << message << "\n\n";

    Json::Value suggestions(Json::arrayValue);
    int count = 0;
    std::set<string> addedCourses;

    for (clsGrade& G : allGrades) {
        if (G.Letter() == "-" || G.Letter() == "") continue;
        if (addedCourses.count(G.CourseCode())) continue;

        int total = calcTotal(G);
        // Skip empty or very high grades (unless F)
        if (total == 0 && G.Letter() != "F") continue;

        if (clsGrade::GradeToPoints(G.Letter()) < 3.0 || total < 80) {
            Json::Value course;
            course["courseCode"] = G.CourseCode();
            clsCourse courseObj = clsCourse::FindCourse(G.CourseCode());
            string cName = courseObj.Name();
            course["courseName"] = cName;
            course["grade"] = total;

            if (G.Letter() == "F" || G.Letter() == "D-" || G.Letter() == "D")
                course["reason"] = "Critical: Low Grade (" + G.Letter() + "). Retake highly recommended.";
            else if (G.Letter() == "D+" || G.Letter() == "C-" || G.Letter() == "C")
                course["reason"] = "Warning: Grade (" + G.Letter() + ") is pulling down your GPA.";
            else
                course["reason"] = "Improvement Opportunity (Grade: " + G.Letter() + ")";

            suggestions.append(course);
            addedCourses.insert(G.CourseCode());

            if (count < 3 && status != "Safe") {
                if (count == 0) adviceBody << "Recommended courses to retake:\n";
                adviceBody << "- " << G.CourseCode() << " (" << cName << "): " << G.Letter() << " (" << total << ")\n";
            }
            count++;
        }
        if (count >= 3) break;
    }

    if (status == "Safe") {
        responseJson["suggestions"] = Json::arrayValue;
    }
    else {
        responseJson["suggestions"] = suggestions;
    }

    vector<clsSystemAnnouncement> myMsgs = clsSystemAnnouncement::GetListFor(studentId);
    bool alreadyNotified = false;
    for (auto& msg : myMsgs) {
        if (msg.Title() == emailSubject) {
            alreadyNotified = true;
            break;
        }
    }

    if (!alreadyNotified) {
        string finalMessage = adviceBody.str();
        if (status == "Safe") {
            finalMessage = "Congratulations! Your GPA is " + to_string(gpa) + ". You are maintaining an excellent academic standing. Keep it up!";
        }
        clsSystemAnnouncement newMsg(emailSubject, finalMessage, studentId);
        newMsg.Save();
        responseJson["notificationSent"] = true;
    }
    else {
        responseJson["notificationSent"] = false;
    }

    if (suggestions.size() > 0 && status != "Safe") {
        responseJson["suggestionTitle"] = "Recommendations to Boost Your GPA:";
    }
    else {
        responseJson["suggestionTitle"] = "";
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");

    callback(resp);
}
#pragma endregion