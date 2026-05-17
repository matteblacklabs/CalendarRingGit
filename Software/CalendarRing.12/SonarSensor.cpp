#include "SonarSensor.h"

SonarSensor::SonarSensor(int pin, float vcc, float adcResolution)
    : _pin(pin), 
    _vcc(vcc), 
    _adcResolution(adcResolution),
    _scalingFactor(vcc / 512.0f), 
    _range(0)
{}

void SonarSensor::begin() {
    pinMode(_pin, INPUT);
    analogSetPinAttenuation(_pin, ADC_11db); // ESP32: full 3.3V range

    // Seed _range with a real reading so EMA doesn't crawl up from zero
    float volt = smoothReadADC() * (_vcc / _adcResolution);
    _range = (int)(volt / _scalingFactor);

    for (int i = 0; i < 20; i++) {
        read();
        getRangeMax();
    }
}

int SonarSensor::smoothReadADC() {
    double total = 0;
    for (int i = 0; i < _adcSamples; i++) total += analogRead(_pin);
    return (int)(total / _adcSamples);
}

int SonarSensor::read() {
    float volt = smoothReadADC() * (_vcc / _adcResolution);
    int newReading = (int)(volt / _scalingFactor);
    if (newReading < 1 || newReading > 255) return _range; // discard sensor error values; MaxSonar valid range is ~1–255 in
    _range = (int)(_range * (1.0f - _emaAlpha) + newReading * _emaAlpha);
    return _range;
}

// Returns brightness [0,255] mapped inversely from range.
// Closer (range <= rangeMin) → bMax. Farther (range >= rangeActive) → bMin.
int SonarSensor::mapToBrightness(int rangeMin, int rangeActive, int bMax, int bMin) const {
    int b = map(_range, rangeMin, rangeActive, bMax, bMin);
    return constrain(b, 0, 255);
}

void SonarSensor::print() const {
    Serial.print("  range[in]: "); Serial.print(_range);
}

int SonarSensor::getRangeMax()
{
    if(_range > _rangeMax) _rangeMax = _range;
    return _rangeMax;
}

int SonarSensor::getRangeMin()
{
    if(_range < _rangeMin) _rangeMin = _range;
    return _rangeMin;
}