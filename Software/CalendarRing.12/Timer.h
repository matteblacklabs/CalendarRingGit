#ifndef Timer_h
#define Timer_h

#include <Arduino.h>

class Timer {
public:
    Timer(unsigned long intervalMs);

    void          reset();
    bool          check();           // true once on expiry, then auto-resets
    bool          isExpired() const; // non-resetting expiry check
    unsigned long elapsed()  const;  // ms since last reset
    long          remaining() const; // ms until expiry, clamped to 0
    unsigned long duration()  const;

private:
    unsigned long _duration;
    unsigned long _startTime;
};

#endif
