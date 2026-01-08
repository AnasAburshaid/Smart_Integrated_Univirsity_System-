#pragma once
#include <iostream>
#include <string>
#include <limits> 
#include "clsString.h"
#include "clsDate.h"

class clsInputValidate
{

public:

	static bool IsNumberBetween(short Number, short From, short To)
	{
		if (Number >= From && Number <= To)
			return true;
		else
			return false;
	}

	static bool IsNumberBetween(int Number, int From, int To)
	{
		if (Number >= From && Number <= To)
			return true;
		else
			return false;

	}

	static bool IsNumberBetween(float Number, float From, float To)
	{
		if (Number >= From && Number <= To)
			return true;
		else
			return false;
	}

	static bool IsNumberBetween(double Number, double From, double To)
	{
		if (Number >= From && Number <= To)
			return true;
		else
			return false;
	}

	static bool IsDateBetween(clsDate Date, clsDate From, clsDate To)
	{
			if ((clsDate::IsDateAfterDate2(Date, From) || clsDate::IsDateEqualsDate2(Date, From))
			&&
			(clsDate::IsDateBeforeDate2(Date, To) || clsDate::IsDateEqualsDate2(Date, To))
			)	
		{
			return true;
		}

		if ((clsDate::IsDateAfterDate2(Date, To) || clsDate::IsDateEqualsDate2(Date, To))
			&&
			(clsDate::IsDateBeforeDate2(Date, From) || clsDate::IsDateEqualsDate2(Date, From))
			)
		{
			return true;
		}

		return false;
	}
	static string ReadString()
	{
		string S1 = "";
		getline(cin >> ws, S1);
		return S1;
	}

	static int ReadIntNumber(string ErrorMessage = "Invalid Number, Enter again\n")
	{
		int Number;
		while (!(cin >> Number)) {
			cin.clear();
			cin.ignore((numeric_limits<streamsize>::max)(), '\n');
			cout << ErrorMessage;
		}
		return Number;
	}

	static int ReadIntNumberBetween(int From, int To, string ErrorMessage = "Number is not within range, Enter again:\n")
	{
		int Number = ReadIntNumber();

		while (!IsNumberBetween(Number, From, To))
		{
			cout << ErrorMessage;
			Number = ReadIntNumber();
		}
		return Number;
	}

	static double ReadDblNumber(string ErrorMessage = "Invalid Number, Enter again\n")
	{
		double Number;
		while (!(cin >> Number)) {
			cin.clear();
			cin.ignore((numeric_limits<streamsize>::max)(), '\n');
			cout << ErrorMessage;
		}
		return Number;
	}

	static double ReadFloatNumber(string ErrorMessage = "Invalid Number, Enter again\n")
	{
		float Number;
		while (!(cin >> Number)) {
			cin.clear();
			cin.ignore((numeric_limits<streamsize>::max)(), '\n');
			cout << ErrorMessage;
		}
		return Number;
	}

	static int ReadShortNumber(string ErrorMessage = "Invalid Number, Enter again\n")
	{
		short Number;
		while (!(cin >> Number)) {
			cin.clear();
			cin.ignore((numeric_limits<streamsize>::max)(), '\n');
			cout << ErrorMessage;
		}
		return Number;
	}

	static short ReadShortNumberBetween(short From, short To, string ErrorMessage = "Number is not within range, Enter again:\n")
	{
		short Number = ReadDblNumber();

		while (!IsNumberBetween(Number, From, To)) {
			cout << ErrorMessage;
			Number = ReadShortNumber();
		}
		return Number;
	}


	static double ReadDblNumberBetween(double From, double To, string ErrorMessage = "Number is not within range, Enter again:\n")
	{
		double Number = ReadDblNumber();

		while (!IsNumberBetween(Number, From, To)) {
			cout << ErrorMessage;
			Number = ReadDblNumber();
		}
		return Number;
	}

	static bool IsValidEmail(const string& Email)
	{
		if (Email.empty()) return false;

		size_t atPos = Email.find('@');
		size_t dotPos = Email.find('.', atPos);

		if (atPos == string::npos || dotPos == string::npos)
			return false;

		if (atPos == 0 || dotPos == Email.length() - 1)
			return false;

		if (Email.find(' ') != string::npos)
			return false;

		return true;
	}

	static bool IsValidJordanianPhone(const string& Phone)
	{
		if (Phone.length() != 10)
			return false;

		if (Phone.substr(0, 2) != "07")
			return false;

		for (char c : Phone)
		{
			if (!isdigit(c))
				return false;
		}

		char carrier = Phone[2];
		if (carrier != '7' && carrier != '8' && carrier != '9')
			return false;

		return true;
	}

};