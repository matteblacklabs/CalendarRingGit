#include "Logic.h"
#include <math.h>

// ============================== STATE SETUPS =========================== //
void Logic::setupDeepSleep() {
    _ring.turnOff();
    // functionX();
}

void Logic::setupAsleep() {
    _ring.clear();
    _ring.show();
}

void Logic::setupAttract() {
    getRingColors(_ringStart);   // snapshot current ring
    _ring.clear();
    getRingColors(_ringTarget);  // all black
    _attractTimer.reset();
}

void Logic::setupAwake() {
    _presenceStart = millis();
    _ring.clear();
    setBaseLEDs();
    getTodayBaseColors();
    getRangeLEDBaseColors();
}

void Logic::setupGoingToSleep() {
    _absenceStart = millis();
    getRingColors(_ringStart);    // current full ring
    _ring.clear();
    getRingColors(_ringTarget);   // all black
    lerpRing(_ringStart, _ringStart, 0.0f); // begin at full ring
    _ring.show();
}

void Logic::setupWakingUp() {
    getRingColors(_ringStart);    // current state (likely dark)
    _ring.clear();
    setBaseLEDs();
    getTodayBaseColors();
    getRangeLEDBaseColors();
    getRingColors(_ringTarget);   // full ring
    lerpRing(_ringStart, _ringStart, 0.0f); // begin at start
    _ring.show();
    _wakingUpTimer.reset();
}

// ============================== STATE LOOPS =========================== //
void Logic::loopDeepSleep() {
}

void Logic::loopAsleep() {
    setTodayPulse();
}

void Logic::loopAttract() {
    static const Animation firstDaysAnim     = {   0, 1000 };
    static const Animation solinoxAnim       = { 400, 1000 };
    static const Animation firstLastDaysAnim = { 800,  800 };
    static const Animation maxLEDsAnim       = {   0, 1000 };
    static const Animation rangeLEDsAnim     = { 500, 1000 };
    unsigned long elapsed = _attractTimer.elapsed();

    lerpRingAnimated(elapsed, firstDaysAnim, solinoxAnim, firstLastDaysAnim);
    setAttractLEDs(localT(elapsed, maxLEDsAnim), localT(elapsed, rangeLEDsAnim));
    setTodayPulse();
}

void Logic::loopWakingUp() {
    static const Animation firstDaysAnim     = {   0, 1000 };
    static const Animation solinoxAnim       = { 400, 1000 };
    static const Animation firstLastDaysAnim = { 800,  800 };
    lerpRingAnimated(_wakingUpTimer.elapsed(), firstDaysAnim, solinoxAnim, firstLastDaysAnim);
    setTodayPulse();  
}

void Logic::loopAwake() {
    setTodayPulse();
}

void Logic::loopGoingToSleep() {
    static const Animation firstLastDaysAnim = {   0,  800 };
    static const Animation solinoxAnim       = { 400, 1000 };
    static const Animation firstDaysAnim     = { 800, 1000 };
    lerpRingAnimated(_goingToSleepTimer.elapsed(), firstDaysAnim, solinoxAnim, firstLastDaysAnim);
    setTodayPulse();
}


// ============================== ATTRACT FUNCTIONS ========================= //

// Fades in the max-range bracket LEDs (tMax) then the sonar-scaled bracket LEDs (tRange).
// tMax, tRange: [0.0, 1.0] progress values produced by localT() in loopAttract().
// Max bracket: fixed ±15 days around today. Range bracket: scales with sonar distance.
void Logic::setAttractLEDs(float tMax, float tRange) {
    int doy = _rtc.dayOfYear();
    int arcSpread = 60;

    // Max bracket — fixed ±15 days, fades in with tMax
    uint8_t rMax = (uint8_t)(40 * tMax);
    uint8_t gMax = (uint8_t)(10 * tMax);
    uint8_t bMax = (uint8_t)(80 * tMax);
    _ring.setDay((doy - arcSpread - 1 + 365) % 365 + 1, rMax, gMax, bMax);
    _ring.setDay((doy + arcSpread-1) % 365 + 1,        rMax, gMax, bMax);

    // Range bracket — sonar-scaled spread, fades in with tRange
    int range = constrain(_sonar.getRange(), RANGE_ACTIVE, RANGE_ATTRACT);
    float distanceFactor = (float)(range - RANGE_ACTIVE) / (float)(RANGE_ATTRACT - RANGE_ACTIVE); // 0.0 = at active edge (close), 1.0 = at attract edge (far)
    static float smoothDistanceFactor = 0.0f; // static: persists between calls so EMA has memory across ticks
    const float emaAlpha = 0.15f;              // EMA weight [0,1]: higher = faster response, lower = smoother
    smoothDistanceFactor = smoothDistanceFactor * (1.0f - emaAlpha) + distanceFactor * emaAlpha; // bracket position tracks range but can't spike instantly on a bad sensor reading
    int rangeLED = (int)(smoothDistanceFactor * arcSpread); // days from today to place the bracket LED
    uint8_t rRng = (uint8_t)(0 * tRange);
    uint8_t gRng = (uint8_t)(40 * tRange);
    uint8_t bRng = (uint8_t)(80 * tRange);
    _ring.setDay((doy - rangeLED - 1 + 365) % 365 + 1, rRng, gRng, bRng);
    _ring.setDay((doy + rangeLED - 1) % 365 + 1,        rRng, gRng, bRng);
}


// Lerps the ring from _ringStart to _ringTarget in three sequential animation windows.
// Each Animation describes when its layer begins fading in and how long it takes.
// Called every fast tick during waking-up and going-to-sleep transitions.
void Logic::lerpRingAnimated(unsigned long elapsed, Animation firstDaysAnim, Animation solinoxAnim, Animation firstLastDaysAnim) {
    float tFirstDays     = localT(elapsed, firstDaysAnim);
    float tSolinox       = localT(elapsed, solinoxAnim);
    float tFirstLastDays = localT(elapsed, firstLastDaysAnim);

    for (int day = 1; day <= RING_PIXELS; day++) {
        float t = tFirstDays;
        if (day == 1 || day == 365) t = tFirstLastDays;
        for (int s = 0; s < CalendarData::SOLINOX_COUNT; s++) {
            if (day == CalendarData::SOLINOX_DOY[s]) { t = tSolinox; break; }
        }

        uint32_t fromColor = _ringStart[day - 1];
        uint32_t toColor   = _ringTarget[day - 1];
        uint8_t r = (uint8_t)(((fromColor >> 16) & 0xFF) * (1.0f - t) + ((toColor >> 16) & 0xFF) * t);
        uint8_t g = (uint8_t)(((fromColor >>  8) & 0xFF) * (1.0f - t) + ((toColor >>  8) & 0xFF) * t);
        uint8_t b = (uint8_t)(( fromColor        & 0xFF) * (1.0f - t) + ( toColor        & 0xFF) * t);
        _ring.setDay(day, r, g, b);
    }
}


// ============================== Base Calendar Layers ========================= //
// Coordinator: sets all base layers.
void Logic::setBaseLEDs() {
    setFirstDaysLEDs(); // add brighness input
    //setFirstLastDayLEDs();
    setSolinoxLEDs();
}

// Layer 1: first day of each month
void Logic::setFirstDaysLEDs() {
    for (int m = 1; m <= 12; m++) {
        _ring.setDay(_rtc.getFirstDay(m),
                     _calendar.firstDayColor.r,
                     _calendar.firstDayColor.g,
                     _calendar.firstDayColor.b);
    }
}

// Layer 2: Jan 1 and Dec 31 special markers (override month markers)
void Logic::setFirstLastDayLEDs() {
    _ring.setDay(1,   _calendar.jan1Color.r,  _calendar.jan1Color.g,  _calendar.jan1Color.b);
    _ring.setDay(365, _calendar.dec31Color.r, _calendar.dec31Color.g, _calendar.dec31Color.b);
}

// Layer 3: solstices and equinoxes (override first days if they coincide)
void Logic::setSolinoxLEDs() {
    for (int i = 0; i < CalendarData::SOLINOX_COUNT; i++) {
        _ring.setDay(CalendarData::SOLINOX_DOY[i],
                     _calendar.solinoxColor.r,
                     _calendar.solinoxColor.g,
                     _calendar.solinoxColor.b);
    }
}

// Reads back today's ring color after base layers are set and stores it.
// Must be called after setBaseLEDs() so any marker color (solinox, first day) is captured.
// setTodayPulse() uses these values to animate today's LED each fast tick.
void Logic::getTodayBaseColors() {
    uint32_t rgb = _ring.getDayColor(_rtc.dayOfYear());
    _todayBaseR = (rgb >> 16) & 0xFF;
    _todayBaseG = (rgb >>  8) & 0xFF;
    _todayBaseB =  rgb        & 0xFF;
}

// ----------- Range LED Functions ---------- //
// Must be called after setBaseLEDs() so marker colors are in the buffer.
void Logic::getRangeLEDBaseColors() {
    _rangeLEDBase[0] = _ring.getDayColor(_rtc.dayOfYear() - 1);
    _rangeLEDBase[1] = _ring.getDayColor(_rtc.dayOfYear() + 1);
}

// Sets RANGE_LED color and ring brightness based on sonar range.
// Color lerps blue (far) → green (close). Brightness lerps dim → max.
// Color blue for rangeInactive
// Green for rangeActive
// Constant brightness incresase
void Logic::setRangeLED1() {
    uint8_t r, g, b;
    int rangeMax = (int)_sonar.getRangeMax();
    int range = constrain(_sonar.getRange(), RANGE_MIN, rangeMax);
    float fraction = (float)(range - RANGE_MIN) / (float)(rangeMax - RANGE_MIN); // 0=close, 1=far

    int brightnessRaw = (int)((1.0f - fraction) * 255);
    bool nearMax = brightnessRaw < 20; // if brightnessRaw is less than 10, use 0.
    uint8_t brightness = nearMax ? 0 : (uint8_t)brightnessRaw;

    if (_rangeState == rangeStateAbsent) {
        r = 0;
        g = 0;
        b = brightness;
    } else if (_rangeState == rangeStateActive) {
        r = 0;
        g = brightness;
        b = 0;
    } else {
        r = 0;
        g = brightness;
        b = 0; // rangeStateMin — same as active
    }

    //_ring.setDay(RANGE_LED, r, g, b);
    // int oppositeTodayLED = (_rtc.dayOfYear() + (365/2)) % 365; // Arc opposite today's position
    // _ring.setDay(oppositeTodayLED, r, g, b);

    int day1 = _rtc.dayOfYear() - 1;
    uint8_t r1 = (_rangeLEDBase[0] >> 16) & 0xFF;
    uint8_t g1 = (_rangeLEDBase[0] >>  8) & 0xFF;
    uint8_t b1 =  _rangeLEDBase[0]        & 0xFF;
    r1 = (uint8_t)constrain((int)r1 + r, 0, 255);
    g1 = (uint8_t)constrain((int)g1 + g, 0, 255);
    b1 = (uint8_t)constrain((int)b1 + b, 0, 255);

    int day2 = _rtc.dayOfYear() + 1;
    uint8_t r2 = (_rangeLEDBase[1] >> 16) & 0xFF;
    uint8_t g2 = (_rangeLEDBase[1] >>  8) & 0xFF;
    uint8_t b2 =  _rangeLEDBase[1]        & 0xFF;
    r2 = (uint8_t)constrain((int)r2 + r, 0, 255);
    g2 = (uint8_t)constrain((int)g2 + g, 0, 255);
    b2 = (uint8_t)constrain((int)b2 + b, 0, 255);

    _ring.setDay(day1, r1, g1, b1);
    _ring.setDay(day2, r2, g2, b2);
}


// Sets TIMER_LED color based on time remaining — full green when timer starts, fades as time runs out.
void Logic::setTimerLED(int timeRemaining, int timerDuration) {
    float fraction = (float)timeRemaining / (float)timerDuration; // 1.0 = full time, 0.0 = expired
    uint8_t r = 0;
    uint8_t g = (uint8_t)(255 * fraction);
    uint8_t b = (uint8_t)((255 * fraction) / 2);
    _ring.setDay(TIMER_LED, r, g, b);
}


// Advances pulse counter, sets today LED, returns pulse value [0, 255].
// cos(radian) gives [-1, 1]; amplitude * cos + 0.5 maps to [0, 1].
// Full cycle completes every (indexMax * _tickTimer interval) ms.
int Logic::setTodayPulse() {
    const uint16_t indexMax  = 500;  // steps in a full cycle (affects speed)
    const float    amplitude = 0.5f; // [0, 1]
    const int      scalar    = 255;  // scales pulse to [0, 255]

    float radian = (_pulseCount % indexMax) * (2.0f * PI / (float)indexMax);
    _pulseCount++;
    int pulse = (int)(scalar * (amplitude * cosf(radian) + 0.5f)); // [0, 255]

    uint8_t r = (uint8_t)constrain((int)_todayBaseR + pulse * _calendar.todayColor.r / 255, 0, 255);
    uint8_t g = (uint8_t)constrain((int)_todayBaseG + pulse * _calendar.todayColor.g / 255, 0, 255);
    uint8_t b = (uint8_t)constrain((int)_todayBaseB + pulse * _calendar.todayColor.b / 255, 0, 255);
    _ring.setDay(_rtc.dayOfYear(), r, g, b);
    return pulse;
}


// stores current ring colors into the provided buffer for later lerping
void Logic::getRingColors(uint32_t* buffer) {
    for (int i = 1; i <= RING_PIXELS; i++) {
        buffer[i - 1] = _ring.getDayColor(i);
    }
}


// arcfunction
void Logic::arc(int pulse){
    int arcCenter = (_rtc.dayOfYear() + (365/2)) % 365; // Arc opposite today's position
    int arcLength = 60; // Length of the arc in days
    int arcStart = (arcCenter - arcLength / 2 + 365) % 365; // Start of the arc, wrapped around the year
    int arcEnd = (arcCenter + arcLength / 2) % 365; // End of the arc, wrapped

    uint8_t _arcBaseR = 0;
    uint8_t _arcBaseG = pulse/10; // Arc brightness[0:255] based on pulse, scaled down for visibility
    uint8_t _arcBaseB = 0;

    for (int i = arcStart; i <= arcEnd; i++) {
        _ring.setDay(i, _arcBaseR, _arcBaseG, _arcBaseB);
    }
}


// Returns [0.0, 1.0] progress of elapsed time within the Animation window (delay → delay+duration).
// Returns 0.0 before delay, 1.0 after delay+duration.
float Logic::localT(unsigned long elapsed, Animation a) {
    if (elapsed < a.delay) return 0.0f;
    float t = (float)(elapsed - a.delay) / (float)a.duration;
    return t > 1.0f ? 1.0f : t;
}


// Linearly interpolates each LED's RGB between two explicit buffers.
// t = [0.0:1.0] → [from:to]
void Logic::lerpRing(uint32_t* from, uint32_t* to, float t) {
    for (int i = 1; i <= RING_PIXELS; i++) {
        uint32_t fromColor = from[i - 1];
        uint32_t toColor   = to[i - 1];

        // Extract from R, G, B components from the 32-bit color to 8-bit integers
        uint8_t rFrom = (fromColor >> 16) & 0xFF;
        uint8_t gFrom = (fromColor >>  8) & 0xFF;
        uint8_t bFrom =  fromColor & 0xFF;

        // Extract to R, G, B components from the 32-bit color to 8-bit integers
        uint8_t rTo = (toColor >> 16) & 0xFF;
        uint8_t gTo = (toColor >>  8) & 0xFF;
        uint8_t bTo =  toColor & 0xFF;

        // Linearly interpolate each color component based on t
        uint8_t r = (uint8_t)(rFrom * (1.0f - t) + rTo * t);
        uint8_t g = (uint8_t)(gFrom * (1.0f - t) + gTo * t);
        uint8_t b = (uint8_t)(bFrom * (1.0f - t) + bTo * t);

        _ring.setDay(i, r, g, b);
    }
}
