#include "TeacherController.h"
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <json/json.h> 

#include "../core_logic/clsTeacher.h" 
#include "../core_logic/clsCourse.h" 
#include "../core_logic/clsGradeService.h" 
#include "../core_logic/clsStudent.h"
#include "../core_logic/clsInputValidate.h" 
#include "../core_logic/clsAnnouncments.h" 
#include "../core_logic/clsDate.h"
#include "../core_logic/clsString.h"
#include "../core_logic/clsGrades.h"
#include "../core_logic/clsSystemAnnouncement.h" 
#include "../core_logic/clsExam.h"
#include"../core_logic/clsAttendance.h"
#include"../core_logic/clsSemester.h"

static void addCors(const HttpResponsePtr& resp) {
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");
}

// ======================================================================
// YOUR ORIGINAL CODE (RESTORED EXACTLY)
// ======================================================================

// 1. TEACHER LOGIN
void TeacherController::login(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr) {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp);
        callback(resp);
        return;
    }

    string id = (*jsonPtr)["id"].asString();
    string password = (*jsonPtr)["password"].asString();

    clsTeacher teacher = clsTeacher::Find(id, password);

    if (teacher.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Teacher ID or Password.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k401Unauthorized);
        addCors(resp);
        callback(resp);
    }
    else {
        responseJson["status"] = "success";
        responseJson["data"]["id"] = teacher.ID();
        responseJson["data"]["fullName"] = teacher.FullName();
        responseJson["data"]["department"] = teacher.Department();
        responseJson["data"]["email"] = teacher.Email; 
        responseJson["data"]["phone"] = teacher.Phone;

        responseJson["id"] = teacher.ID();
        responseJson["name"] = teacher.FullName();

        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp);
        callback(resp);
    }
}

// 4. UPDATE TEACHER INFO (Profile)
void TeacherController::updateInfo(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr) {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid JSON.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    if (!(*jsonPtr).isMember("teacherId")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Teacher ID is required.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string id = (*jsonPtr)["teacherId"].asString();
    clsTeacher teacher = clsTeacher::Find(id);

    if (teacher.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Teacher not found.";
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
        teacher.SetEmail(email);
    }

    if ((*jsonPtr).isMember("phone")) {
        string phone = (*jsonPtr)["phone"].asString();
        if (!clsInputValidate::IsValidJordanianPhone(phone)) {
            responseJson["status"] = "fail";
            responseJson["message"] = "Invalid Phone Format. Must be 07xxxxxxxx.";
            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            resp->setStatusCode(k400BadRequest);
            addCors(resp); callback(resp); return;
        }
        teacher.SetPhone(phone);
    }

    if (teacher.Save() == clsTeacher::svSucceeded) {
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

// 5. CHANGE PASSWORD
void TeacherController::updatePassword(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("teacherId") ||
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

    string id = (*jsonPtr)["teacherId"].asString();
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

    clsTeacher teacher = clsTeacher::Find(id);

    if (teacher.IsEmpty() || teacher.Password() != oldPass) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Invalid Teacher ID or Old Password.";
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

    teacher.SetPassword(newPass);

    if (teacher.Save() == clsTeacher::svSucceeded) {
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

// 2. GET TEACHER COURSES (YOUR ORIGINAL)
void TeacherController::getMyCourses(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string teacherId)
{
    Json::Value responseJson;
    clsTeacher teacher = clsTeacher::Find(teacherId);

    if (teacher.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Teacher ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    string teacherFullName = teacher.FullName();
    vector<clsCourse> courses = clsCourse::GetCoursesByInstructor(teacherFullName);
    clsCourse::SortCoursesBySchedule(courses);

    Json::Value coursesArray(Json::arrayValue);
    for (clsCourse& C : courses)
    {
        Json::Value courseObj;
        courseObj["code"] = C.Code();
        courseObj["name"] = C.Name();
        courseObj["days"] = C.Days();
        courseObj["time"] = C.Time();
        courseObj["room"] = C.Room();
        coursesArray.append(courseObj);
    }

    responseJson["status"] = "success";
    responseJson["teacher"] = teacherFullName;
    responseJson["data"] = coursesArray;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

// 8. DELETE ANNOUNCEMENT (YOUR ORIGINAL)
void TeacherController::deleteAnnouncement(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    if (!jsonPtr || !(*jsonPtr).isMember("courseCode") || !(*jsonPtr).isMember("title")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing fields.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    string code = (*jsonPtr)["courseCode"].asString();
    string title = (*jsonPtr)["title"].asString();
    clsAnnouncement deleteA(code, "", "", title, "", clsAnnouncement::enMode::DeleteMode);

    if (deleteA.Save() == clsAnnouncement::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Announcement deleted successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Failed to delete.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp); callback(resp); return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

// 12. GET SYSTEM ANNOUNCEMENTS (YOUR ORIGINAL)
void TeacherController::getSystemAnnouncements(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string teacherId)
{
    Json::Value responseJson;
    Json::Value messagesArray(Json::arrayValue);

    if (!clsTeacher::IsTeacherExist(teacherId)) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Teacher ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    vector<clsSystemAnnouncement> adminMsgs = clsSystemAnnouncement::GetListFor("Teacher");
    for (clsSystemAnnouncement& A : adminMsgs) {
        Json::Value msg;
        msg["type"] = "Admin Announcement";
        msg["title"] = A.Title();
        msg["date"] = A.Date();
        msg["message"] = A.Message();
        messagesArray.append(msg);
    }

    responseJson["status"] = "success";
    responseJson["data"] = messagesArray;
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

// 13. GET TEACHER EXAM SCHEDULE (YOUR ORIGINAL)
void TeacherController::getExamSchedule(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string teacherId)
{
    clsTeacher T = clsTeacher::Find(teacherId);
    if (T.IsEmpty()) {
        Json::Value responseJson;
        responseJson["status"] = "fail";
        responseJson["message"] = "Teacher ID not found.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    vector<clsCourse> myCourses = clsCourse::GetCoursesByInstructor(T.FullName());
    Json::Value examList(Json::arrayValue);

    for (clsCourse& C : myCourses) {
        clsExam E = clsExam::Find(C.Code());
        Json::Value obj;
        obj["courseCode"] = C.Code();
        obj["courseName"] = C.Name();
        obj["midDate"] = E.MidDate;
        obj["midTime"] = E.MidTime;
        obj["midRoom"] = E.MidRoom;
        obj["finalDate"] = E.FinalDate;
        obj["finalTime"] = E.FinalTime;
        obj["finalRoom"] = E.FinalRoom;
        examList.append(obj);
    }

    Json::Value responseJson;
    responseJson["status"] = "success";
    responseJson["data"] = examList;
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

// ======================================================================
// NEW FRONTEND REQUIREMENTS (ADDED FUNCTIONS)
// ======================================================================

// 1. Get Courses (Format for Frontend)
void TeacherController::getTeacherCourses_Frontend(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId)
{
    clsTeacher teacher = clsTeacher::Find(teacherId);
    if (teacher.IsEmpty()) {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value(Json::arrayValue));
        addCors(resp); callback(resp); return;
    }

    string instructorName = "Dr. " + teacher.FirstName + " " + teacher.LastName;
    vector<clsCourse> courses = clsCourse::GetCoursesByInstructor(instructorName);
    clsCourse::SortCoursesBySchedule(courses); 

    Json::Value jsonArray(Json::arrayValue);
    for (const auto& c : courses) {
        Json::Value item;
        item["code"] = c.Code();
        item["name"] = c.Name();
        item["section"] = "1";
        item["studentsCount"] = clsCourse::GetEnrolledCount(c.Code());
        item["days"] = c.Days();
        item["time"] = c.Time();
        jsonArray.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(jsonArray);
    addCors(resp); callback(resp);
}

// 2. Get Students
void TeacherController::getCourseStudents(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId, std::string courseCode, std::string section)
{
    vector<string> studentIDs = clsCourse::GetStudentsInCourse(courseCode);
    Json::Value jsonArray(Json::arrayValue);

    for (const string& id : studentIDs) {
        clsStudent s = clsStudent::Find(id);
        if (s.IsEmpty()) continue;

        int absences = clsAttendance::GetAbsenceCount(id, courseCode);
        Json::Value item;
        item["studentId"] = s.ID();
        item["fullName"] = s.FirstName + " " + s.LastName;
        item["absences"] = absences;

        if (absences >= 8) item["status"] = "Failed";
        else if (absences >= 6) item["status"] = "Warning";
        else item["status"] = "Safe";

        jsonArray.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(jsonArray);
    addCors(resp); callback(resp);
}

// 3. Get Grades
void TeacherController::getCourseGrades(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId, std::string courseCode, std::string section)
{
    std::vector<clsGrade> grades = clsGrade::GetCourseGrades(courseCode);
    Json::Value responseJson;
    Json::Value arr(Json::arrayValue);

    for (auto& G : grades) {
        Json::Value obj;
        obj["studentId"] = G.StudentID();

        clsStudent S = clsStudent::Find(G.StudentID());
        obj["studentName"] = S.FullName();

        obj["assignment"] = (G.Assignment() == "-" || G.Assignment() == "") ? 0 : stoi(G.Assignment());
        obj["midterm"] = (G.Midterm() == "-" || G.Midterm() == "") ? 0 : stoi(G.Midterm());
        obj["final"] = (G.Final() == "-" || G.Final() == "") ? 0 : stoi(G.Final());

        obj["total"] = obj["assignment"].asInt() + obj["midterm"].asInt() + obj["final"].asInt();
        obj["letter"] = G.Letter();

        if (G.Letter() == "W") {
            obj["locked"] = true;
        }
        else {
            obj["locked"] = false;
        }

        arr.append(obj);
    }

    responseJson["status"] = "success";
    responseJson["data"] = arr;
    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}

// 4. Update Grades Bulk 
void TeacherController::updateGradesBulk(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto jsonPtr = req->getJsonObject();

    // 1. Basic Validation
    if (!jsonPtr || !(*jsonPtr).isMember("courseCode") || !(*jsonPtr).isMember("rows")) {
        Json::Value err; err["ok"] = false; err["message"] = "Invalid request format.";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        addCors(resp); callback(resp); return;
    }

    // 2. Security Check (Admin Rules)
    clsSemester rules = clsSemester::GetCurrent();
    if (clsSemester::IsRegistrationOpen()) {
        Json::Value err; err["ok"] = false; err["message"] = "Grading is BLOCKED during Registration.";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }
    if (!rules.GradingOpen) {
        Json::Value err; err["ok"] = false; err["message"] = "Grading is currently LOCKED by Admin.";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }

    const Json::Value& rows = (*jsonPtr)["rows"];
    string courseCode = (*jsonPtr)["courseCode"].asString();

    bool hasError = false;
    string specificError = "";

    for (const auto& row : rows) {
        if (!row.isMember("studentId")) continue;
        string studentId = row["studentId"].asString();

        if (row.isMember("practical")) {
            int val = row["practical"].isInt() ? row["practical"].asInt() : stoi(row["practical"].asString());

            if (val < 0 || val > 20) {
                hasError = true; specificError = "Assignment grade must be between 0-20."; break;
            }

           
            if (!rules.AllowAssignment && val > 0) {
                // Do nothing (Skip)
            }
            else {
                if (!clsGradeService::AddOrEditGrade(studentId, courseCode, clsGradeService::Assignment, val)) {
                    hasError = true; specificError = "Database rejected grade for " + studentId; break;
                }
            }
        }

       
        if (row.isMember("mid")) {
            int val = row["mid"].isInt() ? row["mid"].asInt() : stoi(row["mid"].asString());

            if (val < 0 || val > 30) {
                hasError = true; specificError = "Midterm grade must be between 0-30."; break;
            }

            if (!rules.AllowMidterm && val > 0) {
                // Do nothing (Skip)
            }
            else {
                if (!clsGradeService::AddOrEditGrade(studentId, courseCode, clsGradeService::Midterm, val)) {
                    hasError = true; specificError = "Database rejected grade for " + studentId; break;
                }
            }
        }

        if (row.isMember("final")) {
            int val = row["final"].isInt() ? row["final"].asInt() : stoi(row["final"].asString());

            if (val < 0 || val > 50) {
                hasError = true; specificError = "Final Exam grade must be between 0-50."; break;
            }

            if (!rules.AllowFinal && val > 0) {
                // Do nothing (Skip)
            }
            else {
                if (!clsGradeService::AddOrEditGrade(studentId, courseCode, clsGradeService::Final, val)) {
                    hasError = true; specificError = "Database rejected grade for " + studentId; break;
                }
            }
        }
    }

    // Response
    Json::Value resultJson;
    if (hasError) {
        resultJson["ok"] = false;
        resultJson["message"] = specificError;
        auto resp = HttpResponse::newHttpJsonResponse(resultJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp);
        callback(resp);
    }
    else {
        resultJson["ok"] = true;
        resultJson["message"] = "Grades saved successfully.";
        auto resp = HttpResponse::newHttpJsonResponse(resultJson);
        addCors(resp);
        callback(resp);
    }
}

// 5. Update Absence Delta (Fixed: Lock at 8)
void TeacherController::updateAbsenceDelta(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto jsonPtr = req->getJsonObject();
    Json::Value responseJson;

    // 1. Basic Validation
    if (!jsonPtr) {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid JSON or missing 'Content-Type: application/json' header.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);

        // Manual CORS
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");

        callback(resp);
        return;
    }

    if (!(*jsonPtr).isMember("studentId") || !(*jsonPtr).isMember("courseCode") || !(*jsonPtr).isMember("delta")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing fields: studentId, courseCode, delta.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);

        // Manual CORS
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");

        callback(resp);
        return;
    }

    // =========================================================
    // 2. THE FIX: BLOCK IF REGISTRATION IS OPEN
    // =========================================================
    if (clsSemester::IsRegistrationOpen()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Action Blocked: Attendance cannot be modified while Registration is OPEN.";

        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);

        // Manual CORS
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");

        callback(resp);
        return;
    }
    // =========================================================

    string studentId = (*jsonPtr)["studentId"].asString();
    string courseCode = (*jsonPtr)["courseCode"].asString();
    int delta = (*jsonPtr)["delta"].asInt();

    int currentAbsences = clsAttendance::GetAbsenceCount(studentId, courseCode);

    if (currentAbsences >= 8)
    {
        responseJson["status"] = "fail";
        responseJson["message"] = "Action Blocked: Student has reached 8 absences and failed (F). Attendance is locked.";

        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);

        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");

        callback(resp);
        return;
    }

    if (delta > 0) {
        string date = clsDate::GetSystemDateTimeString();
        clsAttendance att(courseCode, studentId, date, "Absent");
        att.Save();
    }
    else {
        sqlite3* db = clsDB::OpenDB("University.db");
        if (db) {
            string sql = "DELETE FROM Attendance WHERE rowid = (SELECT MAX(rowid) FROM Attendance WHERE StudentID='" + studentId + "' AND CourseCode='" + courseCode + "' AND Status='Absent');";
            clsDB::Execute(db, sql);
            clsDB::CloseDB(db);
        }
    }

    Json::Value result;
    result["ok"] = true;
    auto resp = HttpResponse::newHttpJsonResponse(result);

    // Manual CORS
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, ngrok-skip-browser-warning");

    callback(resp);
}

// 6. Get Announcements
void TeacherController::getAnnouncements(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId, std::string courseCode, std::string section)
{
    vector<clsAnnouncement> all = clsAnnouncement::GetAnnouncementsData();
    Json::Value jsonArray(Json::arrayValue);

    for (const auto& a : all) {
        if (a.CourseCode() == courseCode) {
            Json::Value item;
            item["id"] = 1;
            item["title"] = a.Title();
            item["message"] = a.Message();
            item["createdAt"] = a.Date();
            jsonArray.append(item);
        }
    }
    auto resp = HttpResponse::newHttpJsonResponse(jsonArray);
    addCors(resp); callback(resp);
}

// 7. Post Announcement
void TeacherController::createAnnouncement(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto jsonPtr = req->getJsonObject();
    string teacherId = (*jsonPtr)["teacherId"].asString();
    string courseCode = (*jsonPtr)["courseCode"].asString();
    string title = (*jsonPtr)["title"].asString();
    string message = (*jsonPtr)["message"].asString();

    clsTeacher t = clsTeacher::Find(teacherId);
    string tName = t.FirstName + " " + t.LastName;
    string date = clsDate::GetSystemDateTimeString();

    clsAnnouncement ann(courseCode, tName, date, title, message, clsAnnouncement::AddMode);
    ann.Save();

    Json::Value result; result["ok"] = true;
    auto resp = HttpResponse::newHttpJsonResponse(result);
    addCors(resp); callback(resp);
}

// 9. UPDATE ANNOUNCEMENT (Restored)
void TeacherController::updateAnnouncement(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;
    auto jsonPtr = req->getJsonObject();

    // 1. Validation
    if (!jsonPtr || !(*jsonPtr).isMember("courseCode") ||
        !(*jsonPtr).isMember("title") || !(*jsonPtr).isMember("newMessage"))
    {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing fields (courseCode, title, newMessage).";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k400BadRequest);
        addCors(resp);
        callback(resp);
        return;
    }

    string code = (*jsonPtr)["courseCode"].asString();
    string title = (*jsonPtr)["title"].asString();
    string newMessage = (*jsonPtr)["newMessage"].asString();

    clsAnnouncement updateA(code, "", "", title, newMessage, clsAnnouncement::enMode::UpdateMode);

    if (updateA.Save() == clsAnnouncement::svSucceeded) {
        responseJson["status"] = "success";
        responseJson["message"] = "Announcement updated successfully.";
    }
    else {
        responseJson["status"] = "error";
        responseJson["message"] = "Failed to update (Announcement might not exist).";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k500InternalServerError);
        addCors(resp);
        callback(resp);
        return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp);
    callback(resp);
}

// GET ATTENDANCE HISTORY

void TeacherController::getAttendanceHistory(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string studentId, std::string courseCode)
{
    Json::Value responseJson;

    // 1. Get Data from Logic Layer
    vector<clsAttendance::stRecord> history = clsAttendance::GetHistory(studentId, courseCode);

    Json::Value list(Json::arrayValue);
    for (auto& rec : history) {
        Json::Value item;
        item["date"] = rec.Date;
        item["status"] = rec.Status;
        list.append(item);
    }

    responseJson["status"] = "success";
    responseJson["data"] = list;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);

    addCors(resp);
    callback(resp);

}
// ======================================================================
// GET GRADING RULES 
// ======================================================================
void TeacherController::getGradingRules(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    // 1. Get Current Semester Settings
    clsSemester rules = clsSemester::GetCurrent();

    Json::Value responseJson;
    responseJson["status"] = "success";

    // 2. Return the flags
    responseJson["data"]["gradingOpen"] = rules.GradingOpen;
    responseJson["data"]["allowAssignment"] = rules.AllowAssignment;
    responseJson["data"]["allowMidterm"] = rules.AllowMidterm;
    responseJson["data"]["allowFinal"] = rules.AllowFinal;

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp);
    callback(resp);
}

// ======================================================================
// UPDATE GRADE (With Security Checks)
// ======================================================================
void TeacherController::updateStudentGrade(const HttpRequestPtr& req,
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

    clsSemester rules = clsSemester::GetCurrent();

    if (!rules.GradingOpen) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Grading is currently CLOSED by Admin.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden); 
        addCors(resp); callback(resp); return;
    }

    // Field-Level Lock Checks
    if (jsonPtr->isMember("assignment") && !rules.AllowAssignment) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Assignment grading is currently LOCKED.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }

    if (jsonPtr->isMember("midterm") && !rules.AllowMidterm) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Midterm grading is currently LOCKED.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }

    if (jsonPtr->isMember("final") && !rules.AllowFinal) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Final exam grading is currently LOCKED.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k403Forbidden);
        addCors(resp); callback(resp); return;
    }
    // --- 2. PROCEED TO SAVE ---
    if (!(*jsonPtr).isMember("studentId") || !(*jsonPtr).isMember("courseCode")) {
        responseJson["status"] = "error";
        responseJson["message"] = "Missing studentId or courseCode.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        addCors(resp); callback(resp); return;
    }

    string studentId = (*jsonPtr)["studentId"].asString();
    string courseCode = (*jsonPtr)["courseCode"].asString();

    clsGrade grade = clsGrade::Find(studentId, courseCode);

    if (grade.IsEmpty()) {
        responseJson["status"] = "fail";
        responseJson["message"] = "Student not registered for this course.";
        auto resp = HttpResponse::newHttpJsonResponse(responseJson);
        resp->setStatusCode(k404NotFound);
        addCors(resp); callback(resp); return;
    }

    // Update only the fields sent in JSON
    if (jsonPtr->isMember("assignment")) grade.SetAssignment((*jsonPtr)["assignment"].asInt());
    if (jsonPtr->isMember("midterm"))    grade.SetMidterm((*jsonPtr)["midterm"].asInt());
    if (jsonPtr->isMember("final"))      grade.SetFinal((*jsonPtr)["final"].asInt());

    grade.Save();

    responseJson["status"] = "success";
    responseJson["message"] = "Grade saved successfully.";

    auto resp = HttpResponse::newHttpJsonResponse(responseJson);
    addCors(resp); callback(resp);
}