#pragma once
// Derived from the NeoPattern class to operate on only a subset of a strip's pixels.
// https://learn.adafruit.com/multi-tasking-the-arduino-part-3/using-neopatterns
//
#include "Adafruit_NeoPatterns.h" // for patterns and direction definitions

// NeoPixel_Animator NeoPattern Class -
class NeoPixel_Animator
{
  public:
    NeoPixel_Animator(Adafruit_NeoPixel pixelStrip, uint16_t pixelIndex, uint16_t pixelCount, void (*callback)());

    // Member Variables:
    Adafruit_NeoPixel strip; //  host strip
    uint32_t PixelIndex, PixelCount;  // the range of pixels in the host strip

    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    void (*OnComplete)();  // Callback on completion of pattern

    void Update();
    void Increment();
    void Decrement();
    void Reverse();
    void RainbowCycle(uint8_t interval, direction dir);
    void RainbowCycleUpdate();
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir);
    void TheaterChaseUpdate();
    void ColorWipe(uint32_t color, uint8_t interval, direction dir);
    void ColorWipeUpdate();
    void Scanner(uint32_t color1, uint8_t interval);
    void ScannerUpdate();
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir);
    void FadeUpdate();
    void ColorSet(uint32_t color);
    uint32_t DimColor(uint32_t color);
    uint32_t Wheel(byte WheelPos);
    uint8_t Red(uint32_t color);
    uint8_t Blue(uint32_t color);
    uint8_t Green(uint32_t color);
};
