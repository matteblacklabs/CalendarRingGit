#ifndef SonarSensor_h
#define SonarSensor_h

#include <Arduino.h>

// SonarSensor
// Wraps an analog MaxSonar sensor.
// Reads voltage via smoothed ADC, converts to inches.
// mapToBrightness() is the main consumer of range data.

class SonarSensor {
public:
    SonarSensor(int pin, float vcc = 3300.0f, float adcResolution = 4095.0f);
    void begin();
    int  read();
    int  getRange() const { return _range; }
    int  mapToBrightness(int rangeMin, int rangeActive, int bMax, int bMin) const;
    void print() const;
    int getRangeMin();
    int getRangeMax();

private:
    int   _pin;
    float _vcc;
    float _adcResolution;
    float _scalingFactor;
    int   _range;
    int   _rangeMin = 9999;
    int   _rangeMax = 0;

    int   _adcSamples = 40;    // ADC reads averaged per sensor read (reduces electrical noise)
    float _emaAlpha   = 0.1f;  // EMA weight [0,1]: lower = smoother but slower to respond
    int smoothReadADC();
};

#endif
