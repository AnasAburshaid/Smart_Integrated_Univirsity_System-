#include "AdminController.h"
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <json/json.h> 
#include <regex> 
#include <map> 

#include "../core_logic/clsStudent.h" 
#include "../core_logic/clsTeacher.h" 
#include "../core_logic/clsInputValidate.h" 
#include "../core_logic/clsCourse.h"
#include "../core_logic/clsGrades.h" 
#include "../core_logic/clsSemester.h"
#include "../core_logic/clsExam.h"

// ======================================================================
// CORS HELPER (Crucial for Frontend)
// ======================================================================
static void addCors(const HttpResponsePtr& resp) {
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");
}

// ======================================================================
#pragma region Helper_Functions
// ======================================================================

std::string _ResolveDepartment(std::string input)
{
    std::string upper = input;
    for (char& c : upper) c = toupper(c);

    if (upper == "CS" || upper == "COMPUTER SCIENCE") return "Computer Science";
    if (upper == "SE" || upper == "SOFTWARE ENGINEERING") return "Software Engineering";
    if (upper == "CIS" || upper == "COMPUTER INFORMATION SYSTEMS") return "Computer Information Systems";
    if (upper == "CY" || upper == "CYBER SECURITY") return "Cyber Security";
    if (upper == "GS" || upper == "GENERAL STUDIES") return "General Studies";
    if (upper == "MA" || upper == "MATHEMATICS") return "Mathematics";
    if (upper == "PHY" || upper == "PHYSICS") return "Physics";

    return "";
}

bool _IsValidTeacherID(const std::string& id)
{
    const std::regex pattern(R"(^T\d{4}$)");
    return std::regex_match(id, pattern);
}

std::string _ResolveMajor(std::string input)
{
    std::string upper = input;
    for (char& c : upper) c = toupper(c);

    if (upper == "CS" || upper == "COMPUTER SCIENCE") return "Computer Science";
    if (upper == "SE" || upper == "SOFTWARE ENGINEERING") return "Software Engineering";
    if (upper == "CIS" || upper == "COMPUTER INFORMATION SYSTEMS") return "Computer Information Systems";
    if (upper == "CY" || upper == "CYBER SECURITY") return "Cyber Security";

    return "";
}

bool _IsValidStudentID(const std::string& id)
{
    const std::regex pattern(R"(^3220\d{4}$)");
    return std::regex_match(id, pattern);
}

#pragma endregion

// ================== CONTROLLER ACTIONS ==================

// ======================================================================
#pragma region Admin_Authentication
// ======================================================================

void AdminController::login(const HttpRequestPtr& req,
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

    string username = (*jsonPtr)["username"].asString();
    string password = (*jsonPtr)["password"].asString();

    if (password.length() < 4) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Password too short.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    clsAdmin admin = clsAdmin::Find(username, password);

    if (admin.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Admin Username or Password.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k401Unauthorized);
        addCors(resp); callback(resp);
    }
    else {
        responseJson["status"] = "success";
        responseJson["data"]["username"] = admin.Username();
        responseJson["data"]["fullName"] = admin.FullName();
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp);
    }

}

// GET ADMIN PROFILE

void AdminController::getProfile(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string username)
{
    Json::Value responseJson;

    clsAdmin admin = clsAdmin::Find(username);

    if (admin.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Admin not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp);
        callback(resp);
        return;
    }

    responseJson["status"] = "success";

    // --- Data Object ---
    responseJson["data"]["username"] = admin.Username();
    responseJson["data"]["fullName"] = admin.FullName();
    responseJson["data"]["email"] = admin.Email;
    responseJson["data"]["phone"] = admin.Phone;
    responseJson["data"]["department"] = "Administration";

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp);
    callback(resp);
}
#pragma endregion

// ======================================================================
#pragma region Student_Management
// ======================================================================

void AdminController::addStudent(const HttpRequestPtr& req,
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

    string id = (*jsonPtr)["id"].asString();
    string firstName = clsString::UpperFirstLetterOfEachWord((*jsonPtr)["firstName"].asString());
    string lastName = clsString::UpperFirstLetterOfEachWord((*jsonPtr)["lastName"].asString());
    string email = clsString::LowerAllString((*jsonPtr)["email"].asString());
    string phone = (*jsonPtr)["phone"].asString();
    string majorInput = (*jsonPtr)["major"].asString();
    string password = (*jsonPtr)["password"].asString();

    if (password.length() < 4) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Password too short.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (!_IsValidStudentID(id)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid ID Format! Must start with 3220 and be 8 digits.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (clsStudent::IsStudentExist(id)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student ID already exists.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict);
        addCors(resp); callback(resp); return;
    }

    if (!email.empty() && !clsInputValidate::IsValidEmail(email)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Email Format.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (!phone.empty() && !clsInputValidate::IsValidJordanianPhone(phone)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Phone Format. Must be 07xxxxxxxx.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string fullMajor = "";
    if (!majorInput.empty()) {
        fullMajor = _ResolveMajor(majorInput);
        if (fullMajor == "") {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Major.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
    }

    clsStudent newStudent(clsStudent::enMode::AddStudentMode,
        id, firstName, lastName, email, phone, fullMajor, password);

    if (newStudent.Save() == clsStudent::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Student added successfully.";
        responseJson["data"]["id"] = id;
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Failed to save student.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::getAllStudents(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    vector<clsStudent> vStudents = clsStudent::GetStudentsList();
    Json::Value responseJson;
    Json::Value studentsArray(Json::arrayValue);

    string searchName = req->getParameter("name");
    string searchMajor = req->getParameter("major");

    searchName = clsString::UpperAllString(searchName);
    searchMajor = clsString::UpperAllString(searchMajor);

    for (clsStudent& S : vStudents) {
        bool include = true;
        if (!searchName.empty()) {
            string sName = clsString::UpperAllString(S.FullName());
            if (sName.find(searchName) == string::npos) include = false;
        }
        if (include && !searchMajor.empty()) {
            string sMajor = clsString::UpperAllString(S.Major());
            if (sMajor.find(searchMajor) == string::npos) include = false;
        }
        if (include) {
            Json::Value obj;
            obj["id"] = S.ID();
            obj["name"] = S.FullName();
            obj["email"] = S.Email;
            obj["major"] = S.Major();
            studentsArray.append(obj);
        }
    }

    responseJson["status"] = "success";
    responseJson["count"] = studentsArray.size();
    responseJson["data"] = studentsArray;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::updateStudent(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("id")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Student ID is required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string id = (*jsonPtr)["id"].asString();
    clsStudent student = clsStudent::Find(id);

    if (student.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    if ((*jsonPtr).isMember("email")) {
        string email = (*jsonPtr)["email"].asString();
        if (!clsInputValidate::IsValidEmail(email)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Email Format.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
        student.SetEmail(email);
    }
    if ((*jsonPtr).isMember("phone")) {
        string phone = (*jsonPtr)["phone"].asString();
        if (!clsInputValidate::IsValidJordanianPhone(phone)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Phone Format.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
        student.SetPhone(phone);
    }
    if ((*jsonPtr).isMember("major")) {
        string m = _ResolveMajor((*jsonPtr)["major"].asString());
        if (m != "") student.SetMajor(m);
    }
    if ((*jsonPtr).isMember("firstName")) student.SetFirstName((*jsonPtr)["firstName"].asString());
    if ((*jsonPtr).isMember("lastName"))  student.SetLastName((*jsonPtr)["lastName"].asString());
    if ((*jsonPtr).isMember("password"))  student.SetPassword((*jsonPtr)["password"].asString());

    if (student.Save() == clsStudent::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Student updated successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Failed to update student.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::deleteStudent(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("id")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid JSON. Student ID required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string id = (*jsonPtr)["id"].asString();
    clsStudent student = clsStudent::Find(id);

    if (student.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    clsStudent studentToDelete(clsStudent::enMode::DeleteStudentMode,
        student.ID(), "", "", "", "", "", "");

    if (studentToDelete.Save() == clsStudent::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Student deleted successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Failed to delete student.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

#pragma endregion

// ======================================================================
#pragma region Teacher_Management
// ======================================================================

void AdminController::addTeacher(const HttpRequestPtr& req,
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

    string id = (*jsonPtr)["id"].asString();
    string firstName = clsString::UpperFirstLetterOfEachWord((*jsonPtr)["firstName"].asString());
    string lastName = clsString::UpperFirstLetterOfEachWord((*jsonPtr)["lastName"].asString());
    string email = clsString::LowerAllString((*jsonPtr)["email"].asString());
    string phone = (*jsonPtr)["phone"].asString();
    string deptInput = (*jsonPtr)["department"].asString();
    string password = (*jsonPtr)["password"].asString();

    if (password.length() < 4) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Password too short.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (!_IsValidTeacherID(id)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid ID Format! Must be T####.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (clsTeacher::IsTeacherExist(id)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Teacher ID already exists.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict);
        addCors(resp); callback(resp); return;
    }

    if (!email.empty() && !clsInputValidate::IsValidEmail(email)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Email Format.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (!phone.empty() && !clsInputValidate::IsValidJordanianPhone(phone)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Phone Format. Must be 07xxxxxxxx.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string fullDept = "";
    if (!deptInput.empty()) {
        fullDept = _ResolveDepartment(deptInput);
        if (fullDept == "") {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Department. Departments allowed are (CS,SE,CIS,CY,GA,MA,PHY) ";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            addCors(resp); callback(resp); return;
        }
    }

    clsTeacher newTeacher(clsTeacher::enMode::AddTeacherMode,
        id, firstName, lastName, email, phone, fullDept, password);

    if (newTeacher.Save() == clsTeacher::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Teacher added successfully.";
        responseJson["data"]["id"] = id;
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Database error.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::getAllTeachers(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    vector<clsTeacher> vTeachers = clsTeacher::GetTeachersList();
    Json::Value responseJson;
    Json::Value teachersArray(Json::arrayValue);

    string searchName = req->getParameter("name");
    string searchDept = req->getParameter("department");

    searchName = clsString::UpperAllString(searchName);
    searchDept = clsString::UpperAllString(searchDept);

    for (clsTeacher& T : vTeachers) {
        bool include = true;
        if (!searchName.empty()) {
            string tName = clsString::UpperAllString(T.FullName());
            if (tName.find(searchName) == string::npos) include = false;
        }
        if (include && !searchDept.empty()) {
            string tDept = clsString::UpperAllString(T.Department());
            if (tDept.find(searchDept) == string::npos) include = false;
        }
        if (include) {
            Json::Value obj;
            obj["id"] = T.ID();
            obj["name"] = T.FullName();
            obj["email"] = T.Email;
            obj["department"] = T.Department();
            teachersArray.append(obj);
        }
    }

    responseJson["status"] = "success";
    responseJson["count"] = teachersArray.size();
    responseJson["data"] = teachersArray;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::updateTeacher(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("id")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Teacher ID is required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string id = (*jsonPtr)["id"].asString();
    clsTeacher teacher = clsTeacher::Find(id);

    if (teacher.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Teacher ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    if ((*jsonPtr).isMember("firstName")) teacher.SetFirstName((*jsonPtr)["firstName"].asString());
    if ((*jsonPtr).isMember("lastName"))  teacher.SetLastName((*jsonPtr)["lastName"].asString());

    if ((*jsonPtr).isMember("email")) {
        string email = (*jsonPtr)["email"].asString();
        if (!clsInputValidate::IsValidEmail(email)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Email Format.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
        teacher.SetEmail(email);
    }

    if ((*jsonPtr).isMember("phone")) {
        string phone = (*jsonPtr)["phone"].asString();
        if (!clsInputValidate::IsValidJordanianPhone(phone)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Phone Format.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
        teacher.SetPhone(phone);
    }

    if ((*jsonPtr).isMember("password")) {
        teacher.SetPassword((*jsonPtr)["password"].asString());
    }

    if ((*jsonPtr).isMember("department")) {
        string d = _ResolveDepartment((*jsonPtr)["department"].asString());
        if (d != "") teacher.SetDepartment(d);
    }

    if (teacher.Save() == clsTeacher::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Teacher updated successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Failed to update teacher.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::deleteTeacher(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("id")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Teacher ID required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string id = (*jsonPtr)["id"].asString();
    clsTeacher teacher = clsTeacher::Find(id);

    if (teacher.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Teacher ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    clsTeacher teacherToDelete(clsTeacher::enMode::DeleteTeacherMode,
        teacher.ID(), "", "", "", "", "", "");

    if (teacherToDelete.Save() == clsTeacher::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Teacher deleted successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Failed to delete teacher.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

#pragma endregion

// ======================================================================
#pragma region Course_Management
// ======================================================================

void AdminController::getAllCourses(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    vector<clsCourse> vCourses = clsCourse::GetCoursesList();
    Json::Value responseJson;
    Json::Value coursesArray(Json::arrayValue);

    for (clsCourse& C : vCourses) {
        Json::Value obj;
        obj["code"] = C.Code();
        obj["name"] = C.Name();
        obj["instructor"] = C.Instructor();
        obj["creditHours"] = C.CreditHours();
        obj["days"] = C.Days();
        obj["time"] = C.Time();
        obj["room"] = C.Room();
        coursesArray.append(obj);
    }

    responseJson["status"] = "success";
    responseJson["data"] = coursesArray;
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::addCourse(const HttpRequestPtr& req,
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

    string code = clsString::UpperAllString((*jsonPtr)["code"].asString());
    string name = clsString::UpperFirstLetterOfEachWord((*jsonPtr)["name"].asString());
    string instructorInput = (*jsonPtr)["instructor"].asString();
    string creditHours = (*jsonPtr)["creditHours"].asString();
    string days = (*jsonPtr)["days"].asString();
    string time = (*jsonPtr)["time"].asString();
    string room = (*jsonPtr)["room"].asString();
    string prereq = (*jsonPtr)["prerequisite"].asString();

    if (!clsCourse::IsValidCourseCodeFormat(code)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Code Format (e.g. CS101).";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }
    if (clsCourse::IsCourseExist(code)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Course Code already exists.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict);
        addCors(resp); callback(resp); return;
    }
    string formattedInstructor;
    if (!clsCourse::TryGetInstructorFormattedName(instructorInput, formattedInstructor)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Instructor not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }
    if (!clsCourse::IsValidCreditHours(creditHours)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Credit Hours (1 or 3).";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }
    if (!clsCourse::IsValidDaysString(days)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Days format Use e.g. 'Sun,Tue'.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }
    if (!clsCourse::IsValidTimeFormat(time)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Time format (HH:MM-HH:MM).";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }
    if (!clsCourse::IsValidRoomFormat(room) || !clsCourse::IsRoomExistsInSystem(room)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Room or Room does not exist.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }
    if (prereq == "") prereq = "-";
    if (!clsCourse::IsValidPrerequisiteCode(prereq)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Prerequisite Code not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }
    if (clsCourse::IsTeacherHasScheduleConflict(formattedInstructor, days, time))
    {
        responseJson["status"] = "fail";
        responseJson["message"] = "Schedule Conflict: Instructor is busy.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict);
        addCors(resp); callback(resp); return;
    }

    clsCourse newCourse(clsCourse::AddMode, code, name, formattedInstructor,
        days, time, room, creditHours, prereq);

    if (newCourse.Save() == clsCourse::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Course added successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Database error.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

// Update Course 
void AdminController::updateCourse(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string code = (*jsonPtr)["code"].asString();
    string name = (*jsonPtr)["name"].asString();
    string teacherInput = (*jsonPtr)["instructor"].asString(); 
    string days = (*jsonPtr)["days"].asString();
    string time = (*jsonPtr)["time"].asString();
    string room = (*jsonPtr)["room"].asString();
    string credits = (*jsonPtr)["creditHours"].asString();
    string pre = (*jsonPtr)["prerequisite"].asString();

  
    clsTeacher teacher = clsTeacher::Find(teacherInput);
    string instructorName = teacherInput; 

    if (!teacher.IsEmpty()) {
        instructorName = "Dr. " + teacher.FirstName + " " + teacher.LastName;
    }

    sqlite3* db = clsDB::OpenDB("University.db");
    if (db) {
        string sql = "UPDATE Courses SET "
            "CourseName='" + name + "', "
            "Instructor='" + instructorName + "', " 
            "Days='" + days + "', "
            "Time='" + time + "', "
            "Room='" + room + "', "
            "CreditHours=" + credits + ", "
            "Prerequisite='" + pre + "' "
            "WHERE CourseCode='" + code + "';";

        clsDB::Execute(db, sql);
        clsDB::CloseDB(db);
    }

    Json::Value result;
    result["status"] = "success";
    result["message"] = "Course updated successfully.";

    auto resp = HttpResponse::newHttpJsonResponse(result);
    addCors(resp);
    callback(resp);
}
void AdminController::deleteCourse(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("code")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Course Code required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string code = (*jsonPtr)["code"].asString();
    clsCourse course = clsCourse::FindCourse(code);

    if (course.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Course not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    clsCourse courseToDelete(clsCourse::DeleteMode, code, "", "", "", "", "", "", "");

    if (courseToDelete.Save() == clsCourse::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Course deleted successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Failed to delete course.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::setExamSchedule(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (clsSemester::IsRegistrationOpen()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Cannot schedule exams while Registration is OPEN.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }

    if (!jsonPtr || !(*jsonPtr).isMember("courseCode") || !(*jsonPtr).isMember("type") || !(*jsonPtr).isMember("date")) {
        responseJson["status"] = "error";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    string code = (*jsonPtr)["courseCode"].asString();
    string type = (*jsonPtr)["type"].asString();
    string dateStr = (*jsonPtr)["date"].asString();
    string time = (*jsonPtr)["time"].asString();
    string room = (*jsonPtr)["room"].asString();

    // 1. Get Semester Info
    clsSemester currentSem = clsSemester::GetCurrent();

    // 2. Validate Exam Date against SEMESTER Duration (SemStartDate/SemEndDate)
    if (!clsDate::IsDateBetween(dateStr, currentSem.SemStartDate, currentSem.SemEndDate))
    {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Exam Date: Date must be within the current semester ("
            + currentSem.SemStartDate + " to " + currentSem.SemEndDate + ").";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    clsExam exam = clsExam::Find(code);
    if (type == "Mid" || type == "Midterm") {
        exam.MidDate = dateStr; exam.MidTime = time; exam.MidRoom = room;
    }
    else if (type == "Final") {
        exam.FinalDate = dateStr; exam.FinalTime = time; exam.FinalRoom = room;
    }
    exam.Save();

    responseJson["status"] = "success";
    responseJson["message"] = "Exam schedule updated.";
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

#pragma endregion

// ======================================================================
#pragma region Admin_Profile_Management
// ======================================================================

void AdminController::updateSelfInfo(const HttpRequestPtr& req,
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

    if (!(*jsonPtr).isMember("username")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Username is required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string username = (*jsonPtr)["username"].asString();
    clsAdmin admin = clsAdmin::Find(username);

    if (admin.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Admin not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    if ((*jsonPtr).isMember("email")) {
        string email = (*jsonPtr)["email"].asString();
        if (!clsInputValidate::IsValidEmail(email)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Email Format.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
        admin.SetEmail(email);
    }

    if ((*jsonPtr).isMember("phone")) {
        string phone = (*jsonPtr)["phone"].asString();
        if (!clsInputValidate::IsValidJordanianPhone(phone)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Phone Format.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
        admin.SetPhone(phone);
    }

    if (admin.Save() == clsAdmin::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Profile updated successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Database error.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::updateSelfPassword(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("username") ||
        !(*jsonPtr).isMember("oldPassword") ||
        !(*jsonPtr).isMember("newPassword") ||
        !(*jsonPtr).isMember("confirmPassword"))
    {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing fields.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string username = (*jsonPtr)["username"].asString();
    string oldPass = (*jsonPtr)["oldPassword"].asString();
    string newPass = (*jsonPtr)["newPassword"].asString();
    string confirmPass = (*jsonPtr)["confirmPassword"].asString();

    if (newPass.length() < 4) {
        responseJson["status"] = "fail";
        responseJson["message"] = "New Password too short.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    clsAdmin admin = clsAdmin::Find(username);

    if (admin.IsEmpty() || admin.Password() != oldPass) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Username or Old Password.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k401Unauthorized);
        addCors(resp); callback(resp); return;
    }

    if (newPass != confirmPass) {
        responseJson["status"] = "fail";
        responseJson["message"] = "New Password and Confirm Password do not match.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    admin.SetPassword(newPass);

    if (admin.Save() == clsAdmin::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Password changed successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Database error.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

#pragma endregion

// ======================================================================
#pragma region Dashboard_and_System
// ======================================================================

void AdminController::getSystemStats(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    Json::Value stats;

    vector<clsStudent> vStudents = clsStudent::GetStudentsList();
    vector<clsTeacher> vTeachers = clsTeacher::GetTeachersList();
    vector<clsCourse> vCourses = clsCourse::GetCoursesList();

    stats["totalStudents"] = (int)vStudents.size();
    stats["totalTeachers"] = (int)vTeachers.size();
    stats["totalCourses"] = (int)vCourses.size();

    map<string, int> majorCounts;
    for (clsStudent& S : vStudents) {
        string m = S.Major();
        if (m == "") m = "Unknown";
        majorCounts[m]++;
    }

    Json::Value majorsJson;
    for (auto const& [key, val] : majorCounts) {
        majorsJson[key] = val;
    }
    stats["studentsPerMajor"] = majorsJson;

    double totalGPA = 0;
    int studentCount = 0;
    for (clsStudent& S : vStudents) {
        double gpa = clsGrade::CalculateCumulativeGPA(S.ID());
        if (gpa > 0) {
            totalGPA += gpa;
            studentCount++;
        }
    }

    if (studentCount > 0) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << (totalGPA / studentCount);
        stats["averageSystemGPA"] = stream.str();
    }
    else {
        stats["averageSystemGPA"] = "0.00";
    }

    responseJson["status"] = "success";
    responseJson["data"] = stats;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::updateSemester(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("semester") ||
        !(*jsonPtr).isMember("startDate") ||
        !(*jsonPtr).isMember("endDate") ||
        !(*jsonPtr).isMember("withdrawDate"))
    {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing fields.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string newSem = (*jsonPtr)["semester"].asString();
    string start = (*jsonPtr)["startDate"].asString(); 
    string end = (*jsonPtr)["endDate"].asString();     
    string withdraw = (*jsonPtr)["withdrawDate"].asString();

    // 1. Time Travel Check (Start < End)
    if (clsDate::IsDate1AfterDate2(start, end)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Dates: Semester Start cannot be after End date.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    // 2. Withdraw Date Check (Must be within Semester Timeline)
    if (!clsDate::IsDateBetween(withdraw, start, end)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Withdraw Date: Must be within the semester timeline.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    // Update using the NEW columns (SemStartDate, SemEndDate)
    clsSemester::Update(newSem, start, end, withdraw);

    responseJson["status"] = "success";
    responseJson["message"] = "Semester timeline updated successfully.";

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::postSystemAnnouncement(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("title") || !(*jsonPtr).isMember("message") || !(*jsonPtr).isMember("target")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing fields.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string title = (*jsonPtr)["title"].asString();
    if (clsSystemAnnouncement::IsTitleExist(title))
    {
        responseJson["status"] = "fail";
        responseJson["message"] = "Duplicate Title.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k409Conflict);
        addCors(resp); callback(resp); return;
    }

    string message = (*jsonPtr)["message"].asString();
    string target = (*jsonPtr)["target"].asString();

    clsSystemAnnouncement newMsg(title, message, target);
    newMsg.Save();

    responseJson["status"] = "success";
    responseJson["message"] = "Announcement sent.";
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::updateSystemAnnouncement(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("id")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Announcement ID is required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    int id = (*jsonPtr)["id"].asInt();
    clsSystemAnnouncement msg = clsSystemAnnouncement::Find(id);

    if (msg.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Announcement not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    if ((*jsonPtr).isMember("title")) msg.SetTitle((*jsonPtr)["title"].asString());
    if ((*jsonPtr).isMember("message")) msg.SetMessage((*jsonPtr)["message"].asString());
    if ((*jsonPtr).isMember("target")) msg.SetTarget((*jsonPtr)["target"].asString());

    msg.Save();

    responseJson["status"] = "success";
    responseJson["message"] = "Announcement updated.";
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::deleteSystemAnnouncement(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("id")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Announcement ID is required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    int id = (*jsonPtr)["id"].asInt();
    clsSystemAnnouncement msg = clsSystemAnnouncement::Find(id);

    if (msg.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Announcement not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    msg.Delete();
    
    responseJson["status"] = "success";
    responseJson["message"] = "Announcement deleted.";
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::getAllSystemAnnouncements(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    // 1. Get all messages
    vector<clsSystemAnnouncement> msgs = clsSystemAnnouncement::GetListFor("Admin");

    Json::Value responseJson;
    Json::Value arr(Json::arrayValue);

    for (auto& m : msgs) {
        string t = m.Title();
        
        // 2. FILTER: Exclude Auto-Generated System Messages
        if (t.find("Scholarship Status") != string::npos) continue;
        if (t.find("Academic Excellence Award") != string::npos) continue;
        if (t.find("Honor Roll Reward") != string::npos) continue;
        if (t.find("Academic Warning") != string::npos) continue;
        if (t.find("Graduation Blocked") != string::npos) continue;

        Json::Value obj;
        obj["id"] = m.ID();
        obj["title"] = m.Title();
        obj["message"] = m.Message();
        obj["target"] = m.TargetAudience();
        obj["date"] = m.Date();
        arr.append(obj);
    }

    responseJson["status"] = "success";
    responseJson["data"] = arr;
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

///Update regesaration Date

void AdminController::updateRegistrationDates(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("startDate") || !(*jsonPtr).isMember("endDate")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing start or end date.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    string start = (*jsonPtr)["startDate"].asString();
    string end = (*jsonPtr)["endDate"].asString();

    // 1. Basic Logic Check (Start < End)
    if (clsDate::IsDate1AfterDate2(start, end)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid: Registration Start cannot be after End.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    int duration = clsDate::GetDifferenceInDays(start, end);
    if (duration > 30) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Policy Violation: Registration period cannot exceed 30 days.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }
    
    clsSemester currentSem = clsSemester::GetCurrent();

    // Ensure Semester Dates exist before checking
    if (currentSem.SemEndDate != "")
    {
        if (clsDate::IsDate1AfterDate2(start, currentSem.SemEndDate)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Logic Error: Registration cannot start after the Semester ends (" + currentSem.SemEndDate + ").";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }

        // Check B: Registration End is AFTER Semester End?
        if (clsDate::IsDate1AfterDate2(end, currentSem.SemEndDate)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Logic Error: Registration must finish before the Semester ends.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
    }

    // Save
    clsSemester::UpdateRegistrationDates(start, end);

    responseJson["status"] = "success";
    responseJson["message"] = "Registration dates updated.";
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

void AdminController::updateGradingPolicies(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    // 1. Read Desired State
    bool open = jsonPtr->isMember("gradingOpen") ? (*jsonPtr)["gradingOpen"].asBool() : false;
    bool allowAss = jsonPtr->isMember("allowAssignment") ? (*jsonPtr)["allowAssignment"].asBool() : false;
    bool allowMid = jsonPtr->isMember("allowMidterm") ? (*jsonPtr)["allowMidterm"].asBool() : false;
    bool allowFin = jsonPtr->isMember("allowFinal") ? (*jsonPtr)["allowFinal"].asBool() : false;

    
    if (clsSemester::IsRegistrationOpen())
    {
        if (open || allowAss || allowMid || allowFin)
        {
            Json::Value error;
            error["status"] = "fail";
            error["message"] = "Action Blocked: You cannot enable grading while the Registration Period is still OPEN.";

            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k409Conflict); 
            addCors(resp);
            callback(resp);
            return;
        }
    }

    clsSemester::UpdateGradingRules(open, allowAss, allowMid, allowFin);

    Json::Value result;
    result["status"] = "success";
    result["message"] = "Grading policies updated successfully.";

    result["status_now"]["gradingOpen"] = open;
    result["status_now"]["allowAssignment"] = allowAss;
    result["status_now"]["allowMidterm"] = allowMid;
    result["status_now"]["allowFinal"] = allowFin;

    auto resp = HttpResponse::newHttpJsonResponse(result);
    addCors(resp);
    callback(resp);

#pragma endregion
}