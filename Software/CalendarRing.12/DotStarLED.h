#ifndef DotStarLED_h
#define DotStarLED_h

#include <Arduino.h>
#include <Adafruit_DotStar.h>
#include <SPI.h>

// ------------------ DotStarLED ------------------- //
// Wraps Adafruit_DotStar for the CalendarRing.
// Xiao: DI -> 10_MOSI, CL -> 8_SCK
// ESP8266: DI -> Silk D6, CL -> Silk D5

class mkp_dotStar {
  private:
    Adafruit_DotStar strip;
    int totalPixels;
    int ringShift;
    uint8_t brightness = 255;

    int shiftIndex(int index, int shift);

  public:
    mkp_dotStar(int numPixels, int dataPin, int clockPin, int shift = 182);

    void begin();
    void show();
    void clear();
    void turnOff();

    // Day-based pixel control (1-indexed, shifted around ring)
    void setDay(int day, uint8_t r, uint8_t g, uint8_t b);
    void setDay(int day, uint32_t color);
    void setFinalDay(uint8_t r = 50, uint8_t g = 0, uint8_t b = 0);

    // Read back a day's current color (1-indexed, ring shift applied)
    // Returns 0x00RRGGBB regardless of strip byte order.
    uint32_t getDayColor(int day);

    // Raw pixel control (0-indexed, no shift)
    void setPixel(int index, uint8_t r, uint8_t g, uint8_t b);
    void setPixel(int index, uint32_t color);

    // Fill helpers
    void fillAll(uint8_t r, uint8_t g, uint8_t b);
    void fillRange(int startDay, int endDay, uint8_t r, uint8_t g, uint8_t b);

    // Color packing utility
    static uint32_t color(uint8_t r, uint8_t g, uint8_t b);

    // Config
    void setRingShift(int shift);
    int  getRingShift();
    int  getPixelCount();
};

#endif
