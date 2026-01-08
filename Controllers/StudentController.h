#pragma once
#include <drogon/HttpController.h>
#include <string>  

// Forward declaration
class clsStudent;

using namespace drogon;
using namespace Json;

class StudentController : public drogon::HttpController<StudentController>
{
public:
    METHOD_LIST_BEGIN
        // 1. Login 
        ADD_METHOD_TO(StudentController::login, "/login", Post, Options);

    // 2. Profile 
    ADD_METHOD_TO(StudentController::getStudent, "/student/{id}", Get, Options);

    // 3. Courses
    ADD_METHOD_TO(StudentController::getMyCourses, "/student/{id}/courses", Get, Options);

    // 4. Schedule
    ADD_METHOD_TO(StudentController::getMyCourses, "/student/{id}/schedule", Get, Options);

    // 5. Grades
    ADD_METHOD_TO(StudentController::getGrades, "/student/{id}/grades", Get, Options);

    // 6. Announcements
    ADD_METHOD_TO(StudentController::getAnnouncements, "/student/{id}/announcements", Get, Options);

    // 7. Recommended
    ADD_METHOD_TO(StudentController::getRecommendedCourses, "/student/{id}/recommended-courses", Get, Options);

    // 8. System Announcements
    ADD_METHOD_TO(StudentController::getSystemAnnouncements, "/student/{id}/system-announcements", Get, Options);

    // 9. Exams
    ADD_METHOD_TO(StudentController::getMyExams, "/student/{id}/exams", Get, Options);

    // 10. Available Courses
    ADD_METHOD_TO(StudentController::getAvailableCourses, "/student/{id}/available-courses", Get, Options);

    // 11. Actions
    ADD_METHOD_TO(StudentController::registerCourse, "/student/register", Post, Options);
    ADD_METHOD_TO(StudentController::dropCourse, "/student/drop", Post, Options);
    ADD_METHOD_TO(StudentController::withdrawCourse, "/student/course/withdraw", Post, Options);
    ADD_METHOD_TO(StudentController::updateInfo, "/student/profile/update", Put, Options);
    ADD_METHOD_TO(StudentController::updatePassword, "/student/profile/password", Post, Options);
    ADD_METHOD_TO(StudentController::getScholarshipStatus, "/student/{1}/scholarship", Get, Options);
    ADD_METHOD_TO(StudentController::getRegistrationStatus, "/system/registration-status", Get, Options);

    METHOD_LIST_END


        void getStudent(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);
    void login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

    // This handles BOTH /courses and /schedule
    void getMyCourses(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);

    void registerCourse(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void dropCourse(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updateInfo(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void updatePassword(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getGrades(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);
    void getAnnouncements(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);
    void getAvailableCourses(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);
    void getSystemAnnouncements(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);
    void getRecommendedCourses(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);
    void withdrawCourse(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getScholarshipStatus(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);
    void getMyExams(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string studentId);
    void getRegistrationStatus(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
};