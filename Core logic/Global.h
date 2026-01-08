#pragma once
#include <string>

class clsStudent;
class clsTeacher;
class clsAdmin;

// Global variables using the classes
extern clsStudent CurrentStudent;
extern clsTeacher CurrentTeacher;
extern clsAdmin   CurrentAdmin;
extern std::string CurrentSemester;

void LoadSemesterFromDB();
void SaveSemesterToDB(const std::string& sem);