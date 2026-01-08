#pragma once
#include <drogon/HttpController.h>
#include "../core_logic/clsTeacher.h"
#include "../core_logic/clsCourse.h"
#include "../core_logic/clsAnnouncments.h"
#include "../core_logic/clsSemester.h"

using namespace drogon;
using namespace Json;

class TeacherController : public drogon::HttpController<TeacherController>
{
public:
    METHOD_LIST_BEGIN

        // 1. Authentication & Profile
        ADD_METHOD_TO(TeacherController::login, "/teacher/login", Post, Options);
    ADD_METHOD_TO(TeacherController::updateInfo, "/teacher/profile/update", Put, Options);
    ADD_METHOD_TO(TeacherController::updatePassword, "/teacher/profile/password", Post, Options);

    // 2. Main Dashboard & System Info
    ADD_METHOD_TO(TeacherController::getMyCourses, "/teacher/courses/{teacherId}", Get, Options);
    ADD_METHOD_TO(TeacherController::getExamSchedule, "/teacher/{1}/exams", Get, Options);
    ADD_METHOD_TO(TeacherController::getSystemAnnouncements, "/teacher/{1}/system-announcements", Get, Options);

    // 3. Grading Management (Rules & Updates)
    ADD_METHOD_TO(TeacherController::getGradingRules, "/teacher/grading-rules", Get, Options);
    ADD_METHOD_TO(TeacherController::updateStudentGrade, "/teacher/course/grades", Post, Options);
    ADD_METHOD_TO(TeacherController::updateGradesBulk, "/teacher/course/grades/bulk", Post, Options); // Changed path slightly to avoid conflict if needed, or keep same if handling logic differs inside

    // 4. Frontend Specific Routes (Course Details)
    ADD_METHOD_TO(TeacherController::getTeacherCourses_Frontend, "/teacher/{1}/courses", Get, Options);
    ADD_METHOD_TO(TeacherController::getCourseStudents, "/teacher/{1}/course/{2}/{3}/students", Get, Options);
    ADD_METHOD_TO(TeacherController::getCourseGrades, "/teacher/{1}/course/{2}/{3}/grades", Get, Options);

    // 5. Attendance Management
    ADD_METHOD_TO(TeacherController::updateAbsenceDelta, "/teacher/course/absence", Post, Options);
    ADD_METHOD_TO(TeacherController::getAttendanceHistory, "/teacher/attendance/history?studentId={1}&courseCode={2}", Get, Options);

    // 6. Course Announcements Management
    ADD_METHOD_TO(TeacherController::getAnnouncements, "/teacher/{1}/course/{2}/{3}/announcements", Get, Options);
    ADD_METHOD_TO(TeacherController::createAnnouncement, "/teacher/course/announcement", Post, Options);
    ADD_METHOD_TO(TeacherController::updateAnnouncement, "/teacher/announcement/update", Put, Options);
    ADD_METHOD_TO(TeacherController::deleteAnnouncement, "/teacher/announcement/delete", Post, Options);

    METHOD_LIST_END

        // --- Function Definitions ---

        // Authentication & Profile
        void login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateInfo(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updatePassword(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

    // Dashboard & System
    void getMyCourses(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId);
    void getExamSchedule(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId);
    void getSystemAnnouncements(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId);

    // Grading Logic
    void getGradingRules(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateStudentGrade(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateGradesBulk(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

    // Frontend Data Fetchers
    void getTeacherCourses_Frontend(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId);
    void getCourseStudents(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId, std::string courseCode, std::string section);
    void getCourseGrades(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId, std::string courseCode, std::string section);

    // Attendance
    void updateAbsenceDelta(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getAttendanceHistory(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId, std::string courseCode);

    // Course Announcements
    void getAnnouncements(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string teacherId, std::string courseCode, std::string section);
    void createAnnouncement(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateAnnouncement(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void deleteAnnouncement(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
};