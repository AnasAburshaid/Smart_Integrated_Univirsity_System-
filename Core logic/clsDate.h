#pragma once


#include <iostream>
#include <vector>
#include <cctype>
#include <string>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <ctime>
#include "clsString.h"
#pragma warning(disable : 4996)

using namespace std;

class clsDate
{
private:
	short _day;
	short _month;
	short _year;

public:
	short GetDay()
	{
		return _day;
	}

	void SetDay(short day)
	{
		_day = day;
	}

	short GetMonth()
	{
		return _month;
	}

	void SetmMonth(short month)
	{
		_month = month;
	}
	short GetYear()
	{
		return _year;
	}

	void SetYear(short year)
	{
		_year = year;
	}

	clsDate()
	{
		time_t t = time(0);
		tm now;
		localtime_s(&now, &t);

		_year = now.tm_year + 1900;
		_month = now.tm_mon + 1;
		_day = now.tm_mday;
	}

	clsDate(string date)
	{
		StringToDate(date, "/");
	}

	clsDate(short Days, short year)
	{
		DateForNumbersOfBeginingOfYear(Days, year);
	}

	clsDate(short day, short month, short year)
	{
		_day = day;
		_month = month;
		_year = year;
	}



	static clsDate GetSystemDate()
	{
		//system date
		time_t t = time(0);
		tm* now = localtime(&t);

		short Day, Month, Year;

		Year = now->tm_year + 1900;
		Month = now->tm_mon + 1;
		Day = now->tm_mday;

		return clsDate(Day, Month, Year);
	}

	static string GetSystemDateTimeString()
	{
		//system datetime string
		time_t t = time(0);
		tm* now = localtime(&t);

		short Day, Month, Year, Hour, Minute, Second;

		Year = now->tm_year + 1900;
		Month = now->tm_mon + 1;
		Day = now->tm_mday;
		Hour = now->tm_hour;
		Minute = now->tm_min;
		Second = now->tm_sec;

		return (to_string(Day) + "/" + to_string(Month) + "/"
			+ to_string(Year) + " - "
			+ to_string(Hour) + ":" + to_string(Minute)
			+ ":" + to_string(Second));

	}

	static void Print(clsDate date)
	{
		printf("\n%d/%d/%d", date._day, date._month, date._year);
	}

	void StringToDate(string DateString, string delum)
	{
		vector<string> vStringDate;

		vStringDate = clsString::Split(DateString, delum);

		_day = stoi(vStringDate[0]);
		_month = stoi(vStringDate[1]);
		_year = stoi(vStringDate[2]);

	}

	void DateForNumbersOfBeginingOfYear(int NumberOfDays, int year)
	{

		short reminder = NumberOfDays;
		short DaysInAMonth;
		short month = 1;

		while (true)
		{

			DaysInAMonth = DaysInMonth(month, year);
			if (reminder > DaysInAMonth)
			{
				reminder -= DaysInAMonth;
				month++;
			}
			else
			{
				_day = reminder;
				_month = month;
				_year = year;
				break;
			}

		}
	}

	static int DaysInMonth(int month, int year)
	{
		int NumberOfMonths[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
		return (month == 2) ? (LeapYear(year) ? 29 : 28) : NumberOfMonths[month - 1];
	}

	int DaysInMonth()
	{
		return DaysInMonth(_month, _year);
	}

	static bool LeapYear(int year)
	{
		return (((year % 400) == 0) || ((year % 4) == 0 && (year % 100) != 0));
	}

	bool LeapYear()
	{
		return LeapYear(_year);
	}

	static string DayName(int DayOrder)
	{
		string DaysArr[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };

		return DaysArr[DayOrder];
	}

	static int DayOfWeekOrder(int year, int month, int day)
	{
		int a = (14 - month) / 12;
		int y = year - a;
		int m = month + 12 * a - 2;

		int d = (day + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12) % 7;
		return d;
	}

	static int DayOfWeekOrder(clsDate date)
	{
		return DayOfWeekOrder(date._year, date._month, date._day);
	}

	int DayOfWeekOrder()
	{
		return DayOfWeekOrder(_year, _month, _day);
	}

	static string MonthName(int month)
	{
		string MonthName[12]{ "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

		return MonthName[month - 1];
	}

	string MonthName()
	{
		return MonthName(_month);
	}

	void Print()
	{
		printf("\n%d/%d/%d", _day, _month, _year);

	}

	static void PrintCelender(int month, int year)
	{
		int Days = DaysInMonth(month, year);
		//it gets how much is the day of the requierd month(28/29/30/31) to use it in the loop 

		cout << "\n  --------------- " << MonthName(month) << " ---------------\n\n";
		cout << "  Sun  Mon  Tue  Wed  Thu  Fri  Sat\n";

		int current = DayOfWeekOrder(year, month, 1);
		//cuurent gets the specefied day of the month
		//we used 1 inside the functios because in the celender every month his day starts with 1 
		//so we need to know the first day in that month 

		int i = 0;
		//i is globlal because we need it in the for loop

		for (i = 0; i < current; i++)
		{
			cout << "     ";
			//if it is not the current day make spaces 
		}

		for (int j = 1; j <= Days; j++)
			//if it is here then it had found the current day in the month 
		{
			cout << setw(5) << j;
			//it make space then print the first number 
			if (++i == 7)
			{
				cout << "\n";
				i = 0;
			}
			//we are using i as counter if it is 7 then it is new week then new line
			//we uses ++i to keep the i counting in the check condition so it gets true every 6 times and reset it to 0 after that
		}

		cout << "\n\n  -----------------------------------\n";
	}

	void PrintCelender()
	{
		PrintCelender(_month, _year);
	}

	static void PrintYearCelender(int year)
	{
		cout << "\n  -----------------------------------\n";
		cout << "\t    Celender - " << year;
		cout << "\n  -----------------------------------\n";

		for (int i = 1; i <= 12; i++)
		{
			PrintCelender(i, year);
		}
	}

	void PrintYearCelender()
	{
		PrintYearCelender(_year);
	}

	static short NumberOfDaysInYear(int day, int month, int year)
	{
		short counter = 0;

		for (int i = 1; i < month; i++)
		{
			counter += DaysInMonth(i, year);
		}

		return counter + day;
	}

	short NumberOfDaysInYear()
	{
		return NumberOfDaysInYear(_day, _month, _year);
	}

	static clsDate DateAfterAddingMoreDays(short NewDays, clsDate Date)
	{
		short DaysReminder = NewDays + Date._day;
		short Days = 0;

		while (true)
		{
			Days = DaysInMonth(Date._month, Date._year);

			if (DaysReminder > Days)
			{
				DaysReminder -= Days;

				if (Date._month < 12)
				{
					Date._month++;
				}
				else
				{
					Date._year++;
					Date._month = 1;
				}
			}
			else
			{
				Date._day = DaysReminder;
				break;
			}
		}
		return Date;
	}

	clsDate DateAfterAddingMoreDays(short NewDays)
	{
		return DateAfterAddingMoreDays(NewDays, *this);
	}

	static bool IsLastDayInMonth(clsDate date)
	{
		return (DaysInMonth(date._month, date._year) == date._day) ? true : false;
	}

	bool IsLastDayInMonth()
	{
		return IsLastDayInMonth(*this);
	}

	static bool IsLastMonthInYear(short month)
	{
		return (month == 12);
	}

	bool IsLastMonthInYear()
	{
		return IsLastMonthInYear(_month);
	}

	static clsDate IncreasedDateByOneDay(clsDate date)
	{

		if (IsLastDayInMonth(date))
		{
			if (IsLastMonthInYear(date._month))
			{
				date._month = 1;
				date._day = 1;
				date._year++;
			}
			else
			{
				date._day = 1;
				date._month++;
			}
		}
		else
		{
			date._day++;
		}

		return date;
	}

	clsDate IncreasedDateByOneDay()
	{
		return IncreasedDateByOneDay(*this);
	}

	static bool IsDateBeforeDate2(clsDate Date1, clsDate Date2)
	{
		return  (Date1._year < Date2._year) ? true : ((Date1._year == Date2._year) ? (Date1._month < Date2._month ? true : (Date1._month == Date2._month ? Date1._day < Date2._day : false)) : false);
	}

	bool IsDateBeforeDate2(clsDate date2)
	{
		return IsDateBeforeDate2(*this, date2);
	}

	static void DateSwap(clsDate& date1, clsDate& date2)
	{
		clsDate tempDate;

		tempDate = date1;
		date1 = date2;
		date2 = tempDate;

	}

	static int GetDifferenceInDays(clsDate Date1, clsDate Date2, bool IncludeEndDay = false)
	{
		//this will take care of negative diff
		int Days = 0;
		short SawpFlagValue = 1;

		if (!IsDateBeforeDate2(Date1, Date2))
		{
			//Swap Dates 
			DateSwap(Date1, Date2);
			SawpFlagValue = -1;

		}

		while (IsDateBeforeDate2(Date1, Date2))
		{
			Days++;
			Date1 = IncreasedDateByOneDay(Date1);
		}

		return IncludeEndDay ? ++Days * SawpFlagValue : Days * SawpFlagValue;
	}

	int GetDifferenceInDays(clsDate Date2, bool IncludeEndDay = false)
	{
		return GetDifferenceInDays(*this, Date2, IncludeEndDay);
	}

	static short CalculateMyAgeInDays(clsDate DateOfBirth)
	{
		return GetDifferenceInDays(DateOfBirth, clsDate::GetSystemDate(), true);
	}


	static short DifferntInDaysInTwoDates(clsDate date1, clsDate date2, bool EndDay = false)
	{
		int counter = 0;
		int Flag = 1;

		if (!IsDateBeforeDate2(date1, date2))
		{
			DateSwap(date1, date2);
			Flag = -1;
		}

		while (IsDateBeforeDate2(date1, date2))
		{
			counter++;
			date1 = IncreasedDateByOneDay(date1);
		}

		return (EndDay) ? ++counter * Flag : counter * Flag;

	}

	short DifferntInDaysInTwoDates(clsDate date2, bool EndDay = false)
	{
		return DifferntInDaysInTwoDates(*this, date2, EndDay);
	}

	static bool IsEndOfWeek(clsDate date)
	{
		return (DayOfWeekOrder(date) == 6);
	}

	bool IsEndOfWeek()
	{
		return IsEndOfWeek(*this);
	}

	static bool IsItWeekEnd(clsDate date)
	{
		return (DayOfWeekOrder(date) == 5 || DayOfWeekOrder(date) == 6);
	}

	bool IsItWeekEnd()
	{
		return IsItWeekEnd(*this);
	}

	static bool IsItBusinessDay(clsDate date)
	{
		return (!IsItWeekEnd(date)) ? true : false;
	}

	bool IsItBusinessDay()
	{
		return IsItBusinessDay(*this);
	}

	static short DaysUntilTheEndOfWeek(clsDate date)
	{
		short DayOrder = DayOfWeekOrder(date) + 1;

		return (7 - DayOrder);
	}

	short DaysUntilTheEndOfWeek()
	{
		return DaysUntilTheEndOfWeek(*this);
	}

	static short DaysUntilTheEndOfMonth(clsDate date)
	{
		short Days = DaysInMonth(date._month, date._year);

		return (Days - date._day) + 1;
	}

	short DaysUntilTheEndOfMonth()
	{
		return DaysUntilTheEndOfMonth(*this);
	}

	static short DaysUntilTheEndOfYear(clsDate date)
	{
		short DaysInYear = NumberOfDaysInYear(date._day, date._month, date._year) - 1;

		return LeapYear(date._year) ? 366 - DaysInYear : 365 - DaysInYear;
	}

	short DaysUntilTheEndOfYear()
	{
		return DaysUntilTheEndOfYear(*this);
	}

	static short VactionPeriod(clsDate DateFrom, clsDate DateTo)
	{
		short Vacation = 0;

		while (IsDateBeforeDate2(DateFrom, DateTo))
		{
			if (IsItBusinessDay(DateFrom))
			{
				Vacation++;
			}

			DateFrom = IncreasedDateByOneDay(DateFrom);
		}

		return Vacation;
	}

	static bool IsDateEqualsDate2(clsDate date1, clsDate date2)
	{
		return (date1._year == date2._year && date1._month == date2._month && date1._day == date2._day);
	}

	bool IsDateEqualsDate2(clsDate date2)
	{
		return IsDateEqualsDate2(*this, date2);
	}


	static clsDate VacationReturnDate(short VacationDays, clsDate date)
	{

		short counter = 0;
		while (VacationDays >= counter)
		{
			date = IncreasedDateByOneDay(date);

			if (IsItBusinessDay(date))
			{
				counter++;
			}

		}

		return date;
	}

	clsDate VacationReturnDate(short VacationDays)
	{
		return VacationReturnDate(VacationDays, *this);
	}

	static bool IsDateAfterDate2(clsDate date1, clsDate date2)
	{
		return (!IsDateBeforeDate2(date1, date2) && !IsDateEqualsDate2(date1, date2));
	}

	bool IsDateAfterDate2(clsDate date2)
	{
		return IsDateAfterDate2(*this, date2);
	}

	static bool IsDateValidate(clsDate date)
	{
		return (date._day <= DaysInMonth(date._month, date._year)) ? true : false;
	}

	bool IsDateValidate()
	{
		return IsDateValidate(*this);
	}

	static bool IsDate1AfterDate2(string d1, string d2)
	{
		clsDate date1(d1);
		clsDate date2(d2);
		return IsDateAfterDate2(date1, date2);
	}

	static int GetDifferenceInDays(string d1, string d2)
	{
		clsDate date1(d1);
		clsDate date2(d2);
		return GetDifferenceInDays(date1, date2, false);
	}

	static bool IsDateBetween(string dateToCheck, string start, string end)
	{
		clsDate d(dateToCheck);
		clsDate s(start);
		clsDate e(end);

		return (!IsDateBeforeDate2(d, s) && !IsDateAfterDate2(d, e));
	}

};

