#ifndef Logic_h
#define Logic_h

#include "DotStarLED.h"
#include "RTClock.h"
#include "SonarSensor.h"
#include "OledDisplay.h"
#include "SerialMonitor.h"
#include "CalendarData.h"
#include "Timer.h"

// System states driven by proximity sensor and connectivity
enum SystemState { stateDeepSleep = 0, stateAsleep = 1, stateAwake = 2, stateGoingToSleep = 3, stateWakingUp = 4, stateAttract = 5 };

// Interpreted sonar range — updated every fast tier tick
enum RangeState { rangeStateAbsent, rangeStateAttract, rangeStateActive, rangeStateMin };

// Animationation descriptor: delay before start, duration to complete
struct Animation {
    unsigned long delay;
    unsigned long duration;
};

// Logic
// Coordinator class. Owns all hardware objects and implements the program.
// Ring rendering uses a layered pipeline:
//   1. setBaseLEDs()      — draws month markers (layer 1) and solinoxes (layer 2)
//   2. checkTodayBaseColor() — reads back what's at today's position after layers 1+2
//   3. setTodayLED()      — draws today on top using captured base color
// The pulse timer then animates today's LED by overlaying a sine wave on the base color.

class Logic {
public:
    Logic();
    void setup();
    void loop();

private:
    // Oragans / Hardware Instantiations
    // Classes and objects
    mkp_dotStar   _ring;
    RTClock       _rtc;
    SonarSensor   _sonar;
    OledDisplay   _oled;
    SerialMonitor _serial;
    CalendarData  _calendar;

    // Pinouts
    static const int RING_DATA_PIN = 10;
    static const int RING_CLOCK_PIN = 8;
    static const int SONAR_PIN = 4;
    static const int I2C_SDA = 6;
    static const int I2C_SCL = 7;

    // Hardware parameters
    static const int RING_SHIFT = 182;
    static const int RING_PIXELS = 365;

    // Behavior config
    static const int RANGE_MIN = 6;       // [in]
    static const int RANGE_ACTIVE = 48;   // [in]
    static const int RANGE_ATTRACT = RANGE_ACTIVE + 48; // [in]
    static const int BRIGHTNESS_MAX = 200;
    static const int BRIGHTNESS_DIM = 20;
    static const int RANGE_LED = 340;     // ring position for proximity color indicator
    static const int TIMER_LED = 341;     // ring position for timer countdown indicator
    static const bool DEBUG = false;

    // States
    SystemState  _state;
    SystemState  _lastState;
    RangeState   _rangeState;
    int          _dayLast;
    uint16_t     _pulseCount;
    uint8_t      _todayBaseR;
    uint8_t      _todayBaseG;
    uint8_t      _todayBaseB;
    char         _stateMessage[32];

    // Transition lerp buffers
    uint32_t _ringStart[RING_PIXELS];
    uint32_t _ringTarget[RING_PIXELS];

    // Cached base colors for rangeLED adjacent pixels (captured after setBaseLEDs)
    uint32_t _rangeLEDBase[2];

    // Organism timers
    Timer _fastTimer;  // fast tier: sonar sense + ring respond
    Timer _slowTimer;  // slow tier: rtc sense + state decisions + oled respond

    // State timers
    Timer _wakingUpTimer;     // countdown for waking-up sequence
    Timer _absenceTimer;      // countdown before going to sleep
    Timer _goingToSleepTimer; // countdown till asleep
    Timer _attractTimer;      // fade-in delay for attract range LEDs

    // Range state debounce — candidate must match for N consecutive ticks before _rangeState commits
    static const int RANGE_CONFIRM_THRESHOLD = 5;
    RangeState _rangePending;
    int        _rangeConfirmCount;

    // Presence tracking
    unsigned long _presenceStart;
    unsigned long _absenceStart;
    bool          _absenceTimerStarted;

    // Behavior functions — called by loopX() functions
    int  setTodayPulse();
    void setAttractLEDs(float tMax, float tRange);
    void lerpRingAnimated(unsigned long elapsed, Animation firstDaysAnim, Animation solinoxAnim, Animation firstLastDaysAnim);
    void setRangeLED1(); // unused

    // Indicator LEDs
    void setTimerLED(int timeRemaining, int timerDuration);

    // Ring rendering helpers — called by setupX() and behavior functions
    void setBaseLEDs();
    void setFirstDaysLEDs();
    void setFirstLastDayLEDs();
    void setSolinoxLEDs();
    void getTodayBaseColors();
    void getRangeLEDBaseColors();
    void getRingColors(uint32_t* buffer);
    void lerpRing(uint32_t* from, uint32_t* to, float t);
    void arc(int pulse);
    static float localT(unsigned long elapsed, Animation a);

    // State machine
    SystemState  checkState();
    void         setupState(SystemState newState);
    void         loopState();
    const char*  systemStateStr() const;
    const char*  rangeStateStr() const;
    void         checkRangeState();
   
    void         setupDeepSleep();
    void         setupAsleep();
    void         setupAttract();
    void         setupAwake();
    void         setupGoingToSleep();
    void         setupWakingUp();

    void         loopDeepSleep();
    void         loopAsleep();
    void         loopAttract();
    void         loopAwake();
    void         loopGoingToSleep();
    void         loopWakingUp();
};

#endif
