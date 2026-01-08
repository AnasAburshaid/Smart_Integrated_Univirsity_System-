#pragma once
#include <string>
#include <vector>
#include <cctype>
#include "clsString.h"
using namespace std;

class clsSchedule
{
public:
    enum enDayMask {
        Sun = 1, Mon = 2, Tue = 4, Wed = 8, Thu = 16
    };

    struct stTimeRange {
        int Start;
        int End;
    };

    static int DayTokenToMask(const string& d) {
        string s = d;
        string t; t.reserve(s.size());
        for (char c : s)
            if (!isspace((unsigned char)c))
                t.push_back((char)tolower(c));

        if (t == "sun") return Sun;
        if (t == "mon") return Mon;
        if (t == "tue") return Tue;
        if (t == "wed") return Wed;
        if (t == "thu") return Thu;
        return 0;
    }

    static int ParseDaysMask(const string& daysCSV) {
        int mask = 0;
        vector<string> parts = clsString::Split(daysCSV, ",");
        for (string& p : parts) {
            clsString::Trim(p);
            mask |= DayTokenToMask(p);
        }
        return mask;
    }

    // return the time to int 
    static int ParseHHMMtoMinutes(const string& hhmm) {
        vector<string> v = clsString::Split(hhmm, ":");
        if (v.size() != 2) return -1;
        int hh = stoi(v[0]);
        int mm = stoi(v[1]);
        return hh * 60 + mm;
    }

    // Convert "10:00-11:30" ? {start, end}
    static stTimeRange ParseTimeRange(const string& range) {
        vector<string> v = clsString::Split(range, "-");
        if (v.size() != 2) return { -1,-1 };
        stTimeRange tr;
        tr.Start = ParseHHMMtoMinutes(v[0]);
        tr.End = ParseHHMMtoMinutes(v[1]);
        return tr;
    }
    // Check if two schedules conflict (same day + overlapping time)
    static bool HasConflict(int daysMaskA, stTimeRange timeA,
        int daysMaskB, stTimeRange timeB) {
        if ((daysMaskA & daysMaskB) == 0)
            return false;

        // Overlap if times intersect
        return (timeA.Start < timeB.End) && (timeB.Start < timeA.End);
    }
};
    