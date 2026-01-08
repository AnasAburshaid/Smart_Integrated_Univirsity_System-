#include "Global.h"
#include "clsDB.h"
#include <sqlite3.h>

#include "clsStudent.h"
#include "clsTeacher.h"
#include "clsAdmin.h"

clsStudent CurrentStudent;
clsTeacher CurrentTeacher;
clsAdmin   CurrentAdmin;
std::string CurrentSemester = "Spring2026";

void LoadSemesterFromDB()
{
    sqlite3* db = clsDB::OpenDB("University.db");
    if (!db) return;

    std::string sql =
        "SELECT CurrentSemester FROM Semester WHERE ID = 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char* txt = sqlite3_column_text(stmt, 0);
            if (txt)
                CurrentSemester = reinterpret_cast<const char*>(txt);
        }
    }

    sqlite3_finalize(stmt);
    clsDB::CloseDB(db);
}

void SaveSemesterToDB(const std::string& sem)
{
    sqlite3* db = clsDB::OpenDB("University.db");
    if (!db) return;

    std::string sql =
        "INSERT INTO Semester (ID, CurrentSemester) VALUES (1, '" +
        sem + "') "
        "ON CONFLICT(ID) DO UPDATE SET CurrentSemester = '" + sem + "';";

    clsDB::Execute(db, sql);

    clsDB::CloseDB(db);
}