#pragma once
#include <iostream>
#include "clsScreen.h"
#include "clsInputValidate.h"
#include "Global.h"

using namespace std;

class clsUpdateCurrentSemesterScreen : protected clsScreen
{
public:

    static void ShowUpdateSemester()
    {
        system("cls");
        _DrawScreenHeader("\t Update Current Semester ");

        cout << "\nCurrent Semester: " << CurrentSemester << endl;
        cout << "\nEnter new semester (Ex: Spring2026, Fall2025): ";

        string NewSemester = clsInputValidate::ReadString();
        NewSemester = clsString::Trim(NewSemester);

        cout << "\nAre you sure you want to change the semester to (" << NewSemester << ") ? (y/n): ";
        char c;
        cin >> c;

        if (toupper(c) == 'Y')
        {
            CurrentSemester = NewSemester;
            SaveSemesterToDB(NewSemester);

            cout << "\nSemester Updated Successfully! New semester is: " << CurrentSemester << endl;
        }
        else
        {
            cout << "\nOperation canceled.\n";
        }

        cout << "\n";
        system("pause");
    }
};
