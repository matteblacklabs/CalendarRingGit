#ifndef CalendarData_h
#define CalendarData_h

#include <Arduino.h>

struct RGB { uint8_t r, g, b; };

// CalendarData
// Pure calendar math and color data — no hardware.
// Call update(year) after reading the RTC to recalculate for the current year.
// Logic uses getFirstDay() and SOLINOX_DOY[] to know which LEDs to light,
// and the color members to know what color to use.

class CalendarData {
public:
    CalendarData(); // constructor
    void update(int year);
    int  getFirstDay(int month) const; // month 1–12 → 1-based day-of-year
    void printFirstDays() const;

    // Layer colors — edit these to change the ring appearance
    RGB firstDayColor; // month markers (layers 1)
    RGB solinoxColor;  // solstices and equinoxes (layer 2)
    RGB jan1Color;     // Jan 1 override
    RGB dec31Color;    // Dec 31 override
    RGB todayColor;    // today LED pulse color

    // Single number can be delcare and implemented in header
    // Constant variables are all caps, do not change
    static const int SOLINOX_COUNT = 4; 

    // Array must be defined in header then implemented in cpp
    static const int SOLINOX_DOY[SOLINOX_COUNT]; // Mar 20, Jun 21, Sep 23, Dec 21, 
private:
    int _monthLengths[12];
    int _firstDays[12];

};

#endif
