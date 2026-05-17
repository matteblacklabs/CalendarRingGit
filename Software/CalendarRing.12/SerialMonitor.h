#ifndef SerialMonitor_h
#define SerialMonitor_h

#include <Arduino.h>

// SerialMonitor
// Wraps Arduino Serial for structured output.
// Mirrors OledDisplay pattern — begin() once, printInfo() on slow tier.

class SerialMonitor {
public:
    void begin(const char* name, const char* author, const char* location);
    void printInfo(const char* dateTimeStr, int doy, int range, const char* rangeState, const char* systemState);
    void appendMessage(const char* msg);

private:
    static const int BAUD_RATE = 19200;
};

#endif
