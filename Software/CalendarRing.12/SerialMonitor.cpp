#include "SerialMonitor.h"

void SerialMonitor::begin(const char* name, const char* author, const char* location) {
    Serial.begin(BAUD_RATE);
    delay(200);
    Serial.println(name);
    Serial.println(author);
    Serial.println(location);
}

void SerialMonitor::printInfo(const char* dateTimeStr, int doy, int range, const char* rangeState, const char* systemState) {
    Serial.print(dateTimeStr);
    Serial.print("  DOY: ");         Serial.print(doy);
    Serial.print("  Range: ");       Serial.print(range); Serial.print(" in");
    Serial.print("  rangeState: ");  Serial.print(rangeState);
    Serial.print("  systemState: "); Serial.print(systemState);
    Serial.println();
}

void SerialMonitor::appendMessage(const char* msg) {
    Serial.println(msg);
}
