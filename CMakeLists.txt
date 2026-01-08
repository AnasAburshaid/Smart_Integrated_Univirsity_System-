# --- 1. MINIMUM REQUIREMENTS ---
cmake_minimum_required(VERSION 3.10)
project(UniversityAPIServer)
# --- 2. CONFIGURATION & FIND FRAMEWORK ---
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(Drogon CONFIG REQUIRED)
add_executable(UniversityAPIServer main.cc   "core_logic/clsSystemAnnouncement.h" "core_logic/clsSemester.h" "core_logic/clsExam.h" "core_logic/clsAttendance.h")

target_include_directories(UniversityAPIServer PRIVATE 
    core_logic
)
target_sources(UniversityAPIServer PRIVATE

    main.cc

    core_logic/sqlite3.c

    # --- CORE LOGIC FILES (Minimal Required List) ---
    core_logic/Global.cpp
    core_logic/Database.cpp
    core_logic/DatabaseSetup.cpp

    # --- CONTROLLER FILES ---
    controllers/TestController.cc
    controllers/StudentController.cc
    controllers/TeacherController.cc
    controllers/AdminController.cc
)
 
target_link_libraries(UniversityAPIServer PRIVATE 
    Drogon::Drogon 
)
set_target_properties(UniversityAPIServer PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build"
)