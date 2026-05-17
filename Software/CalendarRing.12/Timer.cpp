#include <Arduino.h>
#include "Timer.h"

Timer::Timer(unsigned long intervalMs)
    : _duration(intervalMs), _startTime(millis())
{}

void Timer::reset() {
    _startTime = millis();
}

bool Timer::check() {
    if (millis() - _startTime >= _duration) {
        reset();
        return true;
    }
    return false;
}

bool Timer::isExpired() const {
    return millis() - _startTime >= _duration;
}

unsigned long Timer::elapsed() const {
    return millis() - _startTime;
}

long Timer::remaining() const {
    long r = (long)(_startTime + _duration) - (long)millis();
    // condition ? true : false, ternary operator 
    return r < 0 ? 0 : r;
}

unsigned long Timer::duration() const {
    return _duration;
}
