#include "Logic.h"
#include <Wire.h>
#include <math.h>
#include "Utilities.h"

// Constructor and initializer list
// Sets objects up in memory
Logic::Logic()
    : _ring(RING_PIXELS, RING_DATA_PIN, RING_CLOCK_PIN, RING_SHIFT),
      _sonar(SONAR_PIN),
      _state(stateAsleep),
      _lastState(stateAsleep),
      _rangeState(rangeStateAbsent),
      _rangePending(rangeStateAbsent),
      _rangeConfirmCount(0),
      _dayLast(-1),
      _pulseCount(0),
      _todayBaseR(0), _todayBaseG(0), _todayBaseB(0),

      _fastTimer(10),   // fast tier: sonar read + ring animation (100 Hz)
      _slowTimer(300),  // slow tier: rtc read + state check + oled update
      _wakingUpTimer(2000),      // how long the waking-up sequence lasts before fully awake
      _absenceTimer(2000),       // how long after losing presence to start going to sleep
      _goingToSleepTimer(2000), // how long the going-to-sleep sequence lasts before fully asleep
      _attractTimer(2000),       // fade-in duration for attract max LEDs

      _presenceStart(0),
      _absenceStart(0),
      _absenceTimerStarted(false)
{}


// ============================== LOGIC SETUP =========================== //
void Logic::setup() {
    // Initialize hardware
    _serial.begin(FILE_NAME, AUTHOR, FILE_LOCATION);  delay(1000);
    Wire.begin(I2C_SDA, I2C_SCL); // single init for shared I2C bus
    _ring.begin();
    _sonar.begin();
    _rtc.begin();
    _oled.begin(FILE_NAME, AUTHOR, FILE_LOCATION);  delay(1000);
    _serial.begin(FILE_NAME, AUTHOR, FILE_LOCATION);  delay(1000);


    // Initial RTC read and ring setup
    _rtc.read();
    _dayLast = _rtc.day();

    // Output to Screens
    _rtc.print();   // all date time data
    _rtc.printFirstDays();  // first days of each month
    _oled.printInfo(_rtc.dateTimeStr(), _rtc.dayOfYear(), _sonar.getRange(), rangeStateStr(), systemStateStr());
    _serial.printInfo(_rtc.dateTimeStr(), _rtc.dayOfYear(), _sonar.getRange(), rangeStateStr(), systemStateStr());

    _oled.appendMessage("CalendarRing ready");
    _serial.appendMessage("CalendarRing ready");
    delay(1000);

    if (DEBUG) Serial.println("DEBUG mode: sonar bypassed, brightness max");
}


// ============================== LOGIC LOOP =========================== //
void Logic::loop() {
    // FAST TIER (10ms) — sonar sense + ring respond
    // Drives smooth human-perceptible output: lerp, pulse, ring.show
    if (_fastTimer.check()) {
        _sonar.read();
        checkRangeState();
        // sense organ read()
        loopState();
        _ring.show();
        // motor organ respond()
    }

    // SLOW TIER (500ms) — rtc sense + state decisions + oled respond
    // Drives conscious awareness: time tracking, state transitions, display
    if (_slowTimer.check()) {
        _rtc.read();
        _calendar.update(_rtc.year());

        // day change: rebuild ring layers
        if (_rtc.day() != _dayLast) {
            _dayLast = _rtc.day();
            _ring.clear();
            setBaseLEDs();
            getTodayBaseColors();
            getRangeLEDBaseColors();
            if (_state == stateAsleep) _ring.clear(); // don't flash markers while asleep
            _ring.show();
            _rtc.print();
        }

        // state machine: check for transitions, run setupState on change
        _stateMessage[0] = '\0';  // clear message each tick
        SystemState next = checkState();
        if (next != _state) {
            _lastState = _state;
            _state = next;
            setupState(_state);
        }

        // Output to Screens
        _oled.printInfo(_rtc.dateTimeStr(), _rtc.dayOfYear(), _sonar.getRange(), rangeStateStr(), systemStateStr());
        _serial.printInfo(_rtc.dateTimeStr(), _rtc.dayOfYear(), _sonar.getRange(), rangeStateStr(), systemStateStr());
        _oled.appendMessage(_stateMessage);
        _serial.appendMessage(_stateMessage);
    }
}


// ============================== SENSOR STATES ================================ //
void Logic::checkRangeState() {
    int range = _sonar.getRange();
    RangeState candidate;
    if      (range <= RANGE_MIN)          candidate = rangeStateMin;
    else if (range <= RANGE_ACTIVE)       candidate = rangeStateActive;
    else if (range <= RANGE_ATTRACT)      candidate = rangeStateAttract;
    else                                  candidate = rangeStateAbsent;

    // Debounce: commit only after RANGE_CONFIRM_THRESHOLD consecutive ticks in same state
    if (candidate == _rangePending) {
        if (++_rangeConfirmCount >= RANGE_CONFIRM_THRESHOLD)
            _rangeState = candidate;
    } else {
        _rangePending      = candidate; // new candidate — reset counter
        _rangeConfirmCount = 1;
    }
}


// ============================== LOGIC STATE MACHINE ========================= //

// -------------- Transition Logic -------------- //
// State transition logic. Each case handles presence and absence branches independently.
SystemState Logic::checkState() {
    if (DEBUG) return stateAwake;
    bool present = (_rangeState != rangeStateAbsent);
    switch (_state) {

        case stateAsleep:
            if (_rangeState == rangeStateAttract) return stateAttract;
            if (_rangeState != rangeStateAbsent)  return stateWakingUp;
            return stateAsleep;

        case stateAttract:
            if (_rangeState == rangeStateActive || _rangeState == rangeStateMin) return stateWakingUp;
            if (_rangeState == rangeStateAbsent) {
                _goingToSleepTimer.reset();
                return stateGoingToSleep;
            }
            return stateAttract;

        case stateWakingUp:
            if (_rangeState == rangeStateAbsent)  return stateAsleep;
            if (_rangeState == rangeStateAttract) return stateAttract;
            if (_wakingUpTimer.check()) return stateAwake;
            return stateWakingUp;

        case stateAwake:
            // Person backed into attract zone — show bracket preview
            if (_rangeState == rangeStateAttract) return stateAttract;
            // Person still close — reset absence timer and stay awake
            if (_rangeState == rangeStateActive || _rangeState == rangeStateMin) {
                _absenceTimerStarted = false;
                return stateAwake;
            }
            // Person gone — start absence timer on first absent tick
            if (!_absenceTimerStarted) {
                _absenceTimer.reset();
                _absenceTimerStarted = true;
            }

            snprintf(_stateMessage, sizeof(_stateMessage), "tAbsence: %d [ms]", _absenceTimer.remaining());

            // Absence confirmed — begin fade to sleep
            if (_absenceTimer.check()) {
                _goingToSleepTimer.reset();
                return stateGoingToSleep;
            }
            return stateAwake;

        case stateGoingToSleep:
            if (_rangeState == rangeStateAttract) return stateAttract;
            if (_rangeState == rangeStateActive || _rangeState == rangeStateMin) return stateWakingUp;
            snprintf(_stateMessage, sizeof(_stateMessage), "tRemaining: %d [ms]", _goingToSleepTimer.remaining());
            if (_goingToSleepTimer.check()) return stateAsleep;
            return stateGoingToSleep;

        default:
            return _state;
    }
}

// ---------------- Coordinator Functions ---------------- //
// Coordinator: calls setupState()
void Logic::setupState(SystemState newState) {
    switch (newState) {
        case stateDeepSleep:    setupDeepSleep();    break;
        case stateAsleep:       setupAsleep();       break;
        case stateAttract:      setupAttract();      break;
        case stateAwake:        setupAwake();        break;
        case stateGoingToSleep: setupGoingToSleep(); break;
        case stateWakingUp:     setupWakingUp();     break;
    }
}

// Coordinator: calls loopState()
void Logic::loopState() {
    switch (_state) {
        case stateDeepSleep:    loopDeepSleep();    break;
        case stateAsleep:       loopAsleep();       break;
        case stateAttract:      loopAttract();      break;
        case stateAwake:        loopAwake();        break;
        case stateGoingToSleep: loopGoingToSleep(); break;
        case stateWakingUp:     loopWakingUp();     break;
    }
}

// returns _state string for OLED and Serial output
const char* Logic::systemStateStr() const {
    switch (_state) {
        case stateDeepSleep:    return "DeepSleep [xx]";
        case stateAsleep:       return "Asleep [--]";
        case stateAttract:      return "Attract [##]";
        case stateAwake:        return "Awake [oo]";
        case stateGoingToSleep: return "GoingToSleep [vv]";
        case stateWakingUp:     return "WakingUp [^^]";
        default:                return "Unknown";
    }
}

// returns _rangeState string for OLED and Serial output
const char* Logic::rangeStateStr() const {
    switch (_rangeState) {
        case rangeStateAbsent: return "Absent";
        case rangeStateAttract: return "Attract"; 
        case rangeStateActive: return "Active";
        case rangeStateMin:    return "Min";
        default:               return "Unknown";
    }
}
