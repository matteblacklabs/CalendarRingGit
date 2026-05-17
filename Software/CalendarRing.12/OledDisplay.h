#ifndef OledDisplay_h
#define OledDisplay_h

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OledDisplay
// Wraps Adafruit SSD1306 128x64 OLED over I2C.
// Wire.begin() must be called before begin().

class OledDisplay {
public:
    OledDisplay();
    void begin(const char* name, const char* author, const char* location);
    void printInfo(const char* dateTimeStr, int doy, int range, const char* rangeState, const char* systemState);
    void appendMessage(const char* msg);

private:
    static const int WIDTH   = 128;
    static const int HEIGHT  = 64;
    static const int ADDRESS = 0x3C;
    Adafruit_SSD1306 _display;
};

#endif
