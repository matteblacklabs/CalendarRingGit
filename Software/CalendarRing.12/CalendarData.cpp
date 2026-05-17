#include "CalendarData.h"
// designed to...

// must be implemented here since it's a static const array
// Day of year of solstices and equinoxes for non-leap years. Layer 2 of the ring.
const int CalendarData::SOLINOX_DOY[CalendarData::SOLINOX_COUNT] = {79, 172, 266, 355};

CalendarData::CalendarData() {
    const int init[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int i = 0; i < 12; i++) _monthLengths[i] = init[i];

    firstDayColor = {  0, 60, 60 }; // cyan
    solinoxColor  = { 40, 10, 80 }; // purple
    jan1Color     = { 10, 50,  0 }; // green
    dec31Color    = { 50,  0,  0 }; // red
    todayColor    = {  0,255,  0 }; // green
}

// Called after RTC read. Updates February length for leap year and recalculates first days.
void CalendarData::update(int year) {
    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    _monthLengths[1] = isLeap ? 29 : 28;
    _firstDays[0] = 1;
    for (int i = 1; i < 12; i++)
        _firstDays[i] = _firstDays[i - 1] + _monthLengths[i - 1];
}


// Should in RTC or deleted???
int CalendarData::getFirstDay(int month) const {
    if (month < 1 || month > 12) return 1;
    return _firstDays[month - 1];
}


// Should be from RTC
void CalendarData::printFirstDays() const {
    Serial.print("firstDays: ");
    for (int i = 0; i < 12; i++) {
        Serial.print(_firstDays[i]); Serial.print(' ');
    }
    Serial.println();
}
