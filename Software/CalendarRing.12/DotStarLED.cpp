#include <Arduino.h>
#include "DotStarLED.h"


// --------------- Constructor --------------- //
mkp_dotStar::mkp_dotStar(int numPixels, int dataPin, int clockPin, int shift)
    : strip(numPixels, dataPin, clockPin, DOTSTAR_BGR),
      totalPixels(numPixels),
      ringShift(shift)
{}


// --------------- begin --------------- //
void mkp_dotStar::begin() {
    strip.begin();
    strip.setBrightness(brightness);
    strip.clear();
    strip.show();
}


// --------------- show / clear --------------- //
void mkp_dotStar::show() {
    strip.show();
}

void mkp_dotStar::clear() {
    strip.clear();
}

void mkp_dotStar::turnOff() {
    strip.setBrightness(0);
    strip.show();
}


// --------------- shiftIndex --------------- //
// @brief: maps a 0-based index around the ring with wraparound
int mkp_dotStar::shiftIndex(int index, int shift) {
    int indexMax = totalPixels - 1;
    int indexNew = index + shift;
    if (indexNew < 0)         { indexNew = indexMax + indexNew + 1; }
    if (indexNew > indexMax)  { indexNew = indexNew - indexMax - 1; }
    return indexNew;
}


// --------------- setDay --------------- //
// @brief: set a day-of-year pixel (1-indexed) with ring shift applied
void mkp_dotStar::setDay(int day, uint8_t r, uint8_t g, uint8_t b) {
    int index = shiftIndex(day - 1, ringShift);
    strip.setPixelColor(index, r, g, b);
}

void mkp_dotStar::setDay(int day, uint32_t color) {
    int index = shiftIndex(day - 1, ringShift);
    strip.setPixelColor(index, color);
}


// --------------- setFinalDay --------------- //
void mkp_dotStar::setFinalDay(uint8_t r, uint8_t g, uint8_t b) {
    setDay(totalPixels, r, g, b);
}


// --------------- setPixel (raw, 0-indexed) --------------- //
void mkp_dotStar::setPixel(int index, uint8_t r, uint8_t g, uint8_t b) {
    strip.setPixelColor(index, r, g, b);
}

void mkp_dotStar::setPixel(int index, uint32_t color) {
    strip.setPixelColor(index, color);
}


// --------------- fillAll --------------- //
void mkp_dotStar::fillAll(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < totalPixels; i++) {
        strip.setPixelColor(i, r, g, b);
    }
}


// --------------- fillRange --------------- //
// @brief: fill a range of days (1-indexed, inclusive) with ring shift applied
void mkp_dotStar::fillRange(int startDay, int endDay, uint8_t r, uint8_t g, uint8_t b) {
    for (int day = startDay; day <= endDay; day++) {
        setDay(day, r, g, b);
    }
}


// --------------- color (static utility) --------------- //
uint32_t mkp_dotStar::color(uint8_t r, uint8_t g, uint8_t b) {
    return Adafruit_DotStar::Color(r, g, b);
}


// --------------- getDayColor --------------- //
// Returns the current color at a day position (1-indexed, ring shift applied).
// Return format is 0x00RRGGBB regardless of strip byte order.
uint32_t mkp_dotStar::getDayColor(int day) {
    int index = shiftIndex(day - 1, ringShift);
    return strip.getPixelColor(index);
}


// --------------- Config --------------- //
void mkp_dotStar::setRingShift(int shift) {
    ringShift = shift;
}

int mkp_dotStar::getRingShift() {
    return ringShift;
}

int mkp_dotStar::getPixelCount() {
    return totalPixels;
}
