#ifndef RTClock_h
#define RTClock_h

#include <Arduino.h>
#include "RTClib.h"

// RTClock
// Wraps RTC_DS1307. Reads date/time, computes dayOfYear, applies DST correction.
// Wire.begin() must be called before begin().

class RTClock {
// functions that can be called from outside
public:
    RTClock();
    void begin();
    void read();
    static bool isLeapYear(int year);
    void set(int year, int month, int day, int hourRaw, int minute, int second);
    void print() const;
    void printFirstDays() const;
    int getFirstDay(int month) const; //  returns the day of year of the first day of each month (1-indexed) based on month lengths

    int  year()      const { return _year; }
    int  month()     const { return _month; }
    int  day()       const { return _day; }
    int  hourRaw()   const { return _hourRaw; }
    int  hourDST()   const { return _hourDST; }
    int  minute()    const { return _minute; }
    int  second()    const { return _second; }
    int  dayOfYear() const { return _dayOfYear; }
    bool isDST()     const { return _dst; }
    const char* dateTimeStr() const { return _dateTimeStr; }
    
//  functions that are only used internally
private:
    RTC_DS1307 _rtc;    // RTC object
    int  _year, _month, _day, _hourDST, _hourRaw, _minute, _second;
    int  _dayOfYear;
    bool _dst;
    char _dateTimeStr[24]; // "YYYY/MM/DD  HH:MM:SS"
    int  _monthLengths[12];
    int  _firstDays[12];
    static const int DST_TABLE_SIZE = 10;
    static const int DST_TABLE[DST_TABLE_SIZE][3]; // {year, startDOY, endDOY}

    void setMonthLengths();
    void setFirstDays();
    void setDayOfYear();
    void checkDST();
    void setHourDST();
};

#endif
