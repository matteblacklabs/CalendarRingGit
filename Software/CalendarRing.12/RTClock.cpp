#include "RTClock.h"


//DST_TABLE (daylight savings time) is a hardcoded schedule of DST start/end dayOfYear for years 2026-2035.
// (year, DST start dayOfYear, DST end dayOfYear)
const int RTClock::DST_TABLE[RTClock::DST_TABLE_SIZE][3] =
{
    {2026, 67, 305},
    {2027, 73, 311},
    {2028, 72, 310},
    {2029, 70, 308},
    {2030, 69, 307},
    {2031, 68, 306},
    {2032, 74, 312},
    {2033, 72, 310},
    {2034, 71, 309},
    {2035, 70, 308}
};

RTClock::RTClock()
    : _year(2026),
    _month(1),
    _day(1),
    _hourDST(0),
    _hourRaw(0),
    _minute(0),
    _second(0),
    _dayOfYear(1),
    _dst(false)
{
    const int defaultMonthLengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int i = 0; i < 12; i++) _monthLengths[i] = defaultMonthLengths[i];
    setFirstDays();
}

void RTClock::begin() {
    _rtc.begin();
    delay(200);
}

void RTClock::read() {
    // Read RTC and set RTC variables
    DateTime now = _rtc.now();
    _year    = now.year();
    _month   = now.month();
    _day     = now.day();
    _minute  = now.minute();
    _second  = now.second();
    _hourRaw = now.hour();

    setMonthLengths();
    setFirstDays();
    setDayOfYear();
    setHourDST();

    // set date/time string
    snprintf(_dateTimeStr, sizeof(_dateTimeStr),
             "%04d/%02d/%02d  %02d:%02d:%02d",
             _year, _month, _day, _hourDST, _minute, _second);
}

// check leap year based off year and formula
bool RTClock::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Set February length based off leap year
void RTClock::setMonthLengths() {
    _monthLengths[1] = isLeapYear(_year) ? 29 : 28;
}

// DOY of the first day of each month (1-indexed)
void RTClock::setFirstDays() {
    _firstDays[0] = 1;
    for (int i = 1; i < 12; i++)
        _firstDays[i] = _firstDays[i - 1] + _monthLengths[i - 1];
}

// _firstDays[] is the day of year of the particular month
// then add the day - 1 becasue the first day of the month is already counted in _firstDays
void RTClock::setDayOfYear() {
    _dayOfYear = _firstDays[_month - 1] + _day - 1;
}

// Check Daylight Savings Time based on the hardcoded schedule in DST_TABLE.
void RTClock::checkDST() {
    for (int i = 0; i < DST_TABLE_SIZE; i++) {
        if (DST_TABLE[i][0] == _year) {
            _dst = (_dayOfYear >= DST_TABLE[i][1] && _dayOfYear <= DST_TABLE[i][2]);
            return;
        }
    }
    _dst = false;
}

// checkDST() then add 1 hour if DST is true
void RTClock::setHourDST() {
    checkDST();
    _hourDST = _hourRaw + (_dst ? 1 : 0);
}

// set RTC for initial setup
void RTClock::set(int year, int month, int day, int hour, int minute, int second) {
    _rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.print("RTC set: ");
    Serial.print(year); Serial.print('/');
    Serial.print(month); Serial.print('/');
    Serial.println(day);
}

// Print RTC
void RTClock::print() const {
    Serial.print("RTC: ");
    Serial.println(_dateTimeStr);
    Serial.print("dayOfYear: "); Serial.print(_dayOfYear);
    Serial.print("  DST: ");     Serial.println(_dst);
}

// to be called from calendarRing
int RTClock::getFirstDay(int month) const {
    return _firstDays[month - 1];
}

// to be called to Print
void RTClock::printFirstDays() const {
    for (int m = 1; m <= 12; m++) {
        Serial.print("Month "); Serial.print(m);
        Serial.print(" first day: "); Serial.println(_firstDays[m - 1]);
    }
}