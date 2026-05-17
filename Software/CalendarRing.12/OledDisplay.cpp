#include "OledDisplay.h"

OledDisplay::OledDisplay()
    : _display(WIDTH, HEIGHT, &Wire, -1)
{}

// begin and show project info
void OledDisplay::begin(const char* name, const char* author, const char* location) {
    _display.begin(SSD1306_SWITCHCAPVCC, ADDRESS);
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(WHITE);
    _display.setCursor(0, 0);
    _display.println(name);
    _display.println(author);
    _display.println(location);
    _display.display();
    Serial.println("OledDisplay: ready");
}

void OledDisplay::printInfo(const char* dateTimeStr, int DOY, int range, const char* rangeState, const char* systemState) {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(WHITE);
    _display.setCursor(0, 0);
    _display.println(dateTimeStr);
    _display.print("DOY: ");    _display.println(DOY);
    _display.print("Range: ");  _display.print(range); _display.println(" in");
    _display.print("Range: ");  _display.println(rangeState);
    _display.print("State: ");  _display.println(systemState);
    _display.display();
}

void OledDisplay::appendMessage(const char* msg) {
    _display.setCursor(0, 40);  // below 5 info lines (each 8px tall)
    _display.println(msg);
    _display.display();
}
