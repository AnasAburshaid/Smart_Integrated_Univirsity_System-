#pragma once
#include <drogon/HttpController.h>
#include "../core_logic/clsAdmin.h"
#include "../core_logic/clsStudent.h" 
#include "../core_logic/clsTeacher.h" 
#include "../core_logic/clsCourse.h"
#include "../core_logic/clsSystemAnnouncement.h"

using namespace drogon;
using namespace Json;

class AdminController : public drogon::HttpController<AdminController>
{
public:
    METHOD_LIST_BEGIN

        // System & Stats
        ADD_METHOD_TO(AdminController::getSystemStats, "/admin/stats", Get, Options);
    ADD_METHOD_TO(AdminController::postSystemAnnouncement, "/admin/system-announcement/add", Post, Options);
    ADD_METHOD_TO(AdminController::updateSystemAnnouncement, "/admin/system-announcement/update", Put, Options);
    ADD_METHOD_TO(AdminController::deleteSystemAnnouncement, "/admin/system-announcement/delete", Post, Options);
    ADD_METHOD_TO(AdminController::getAllSystemAnnouncements, "/admin/system-announcements", Get, Options);

    // Semester & Rules
    ADD_METHOD_TO(AdminController::updateGradingPolicies, "/admin/system/grading-rules", Post, Options);
    ADD_METHOD_TO(AdminController::updateSemester, "/admin/system/semester", Post, Options); 
    ADD_METHOD_TO(AdminController::updateRegistrationDates, "/admin/update-registration-dates", Post, Put, Options); 

    // Exams
    ADD_METHOD_TO(AdminController::setExamSchedule, "/admin/exams/set", Post, Options);

    // Profile
    ADD_METHOD_TO(AdminController::updateSelfInfo, "/admin/profile/update", Put, Options);
    ADD_METHOD_TO(AdminController::updateSelfPassword, "/admin/profile/password", Post, Options);
    ADD_METHOD_TO(AdminController::getProfile, "/admin/profile/{1}", Get, Options);

    // Student Management
    ADD_METHOD_TO(AdminController::login, "/admin/login", Post, Options);
    ADD_METHOD_TO(AdminController::addStudent, "/admin/student/add", Post, Options);
    ADD_METHOD_TO(AdminController::getAllStudents, "/admin/students", Get, Options);
    ADD_METHOD_TO(AdminController::updateStudent, "/admin/student/update", Put, Options);
    ADD_METHOD_TO(AdminController::deleteStudent, "/admin/student/delete", Post, Options);

    // Teacher Management
    ADD_METHOD_TO(AdminController::addTeacher, "/admin/teacher/add", Post, Options);
    ADD_METHOD_TO(AdminController::updateTeacher, "/admin/teacher/update", Put, Options);
    ADD_METHOD_TO(AdminController::deleteTeacher, "/admin/teacher/delete", Post, Options);
    ADD_METHOD_TO(AdminController::getAllTeachers, "/admin/teachers", Get, Options);

    // Course Management
    ADD_METHOD_TO(AdminController::getAllCourses, "/admin/courses", Get, Options);
    ADD_METHOD_TO(AdminController::addCourse, "/admin/course/add", Post, Options);
    ADD_METHOD_TO(AdminController::updateCourse, "/admin/course/update", Put, Options);
    ADD_METHOD_TO(AdminController::deleteCourse, "/admin/course/delete", Post, Options);

    METHOD_LIST_END

        // Function Declarations
        void getSystemStats(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void postSystemAnnouncement(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateSystemAnnouncement(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void deleteSystemAnnouncement(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getAllSystemAnnouncements(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

    void setExamSchedule(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateSemester(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateRegistrationDates(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateGradingPolicies(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

    void updateSelfInfo(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateSelfPassword(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getProfile(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string username);

    void login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void addStudent(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getAllStudents(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateStudent(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void deleteStudent(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

    void addTeacher(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateTeacher(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void deleteTeacher(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getAllTeachers(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

    void getAllCourses(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void addCourse(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateCourse(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void deleteCourse(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
};