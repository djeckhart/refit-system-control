#include "NeoPixel_Animator.h"

// Constructor - calls base-class constructor to initialize strip
NeoPixel_Animator::NeoPixel_Animator(Adafruit_NeoPixel pixelStrip, uint16_t pixelIndex, uint16_t pixelCount, void (*callback)())
{
    strip = pixelStrip;
    PixelIndex = pixelIndex;
    PixelCount = pixelCount;
    OnComplete = callback;
  }

// Increment the Index and reset at the end
void NeoPixel_Animator::Increment()
{
    if (Direction == FORWARD)
    {
       Index++;
       if (Index >= TotalSteps)
        {
            Index = 0;
            if (OnComplete != NULL)
            {
                OnComplete(); // call the completion callback
            }
        }
    }
    else // Direction == REVERSE
    {
        --Index;
        if (Index <= 0)
        {
            Index = TotalSteps-1;
            if (OnComplete != NULL)
            {
                OnComplete(); // call the completion callback
            }
        }
    }
}

// Reverse pattern direction
void NeoPixel_Animator::Reverse()
{
    // if (Direction == FORWARD)
    // {
    //     Direction = REVERSE;
    //     Index = TotalSteps-1;
    // }
    // else
    // {
    //     Direction = FORWARD;
    //     Index = 0;
    // }
}

// Initialize for a RainbowCycle
void NeoPixel_Animator::RainbowCycle(uint8_t interval, direction dir = FORWARD)
{
    ActivePattern = RAINBOW_CYCLE;
    Interval = interval;
    TotalSteps = 255;
    Index = 0;
    Direction = dir;
}

// Update the Rainbow Cycle Pattern
void NeoPixel_Animator::RainbowCycleUpdate()
{
    for(int i=0; i< PixelCount; i++)
    {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + Index) & 255));
    }
    strip.show();
    Increment();
}

// Initialize for a Theater Chase
void NeoPixel_Animator::TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
{
    ActivePattern = THEATER_CHASE;
    Interval = interval;
    TotalSteps = PixelCount;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
}

// Update the Theater Chase Pattern
void NeoPixel_Animator::TheaterChaseUpdate()
{
    for(int i = 0; i < strip.numPixels(); i++)
    {
        if ((i + Index) % 3 == 0)
        {
            strip.setPixelColor(i, Color1);
        }
        else
        {
            strip.setPixelColor(i, Color2);
        }
    }
    strip.show();
    Increment();
}

// Initialize for a ColorWipe
void NeoPixel_Animator::ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
{
    ActivePattern = COLOR_WIPE;
    Interval = interval;
    TotalSteps = PixelCount;
    Color1 = color;
    Index = 0;
    Direction = dir;
}

// Update the Color Wipe Pattern
void NeoPixel_Animator::ColorWipeUpdate()
{
    strip.setPixelColor(Index, Color1);
    strip.show();
    Increment();
}

// Initialize for a SCANNNER
void NeoPixel_Animator::Scanner(uint32_t color1, uint8_t interval)
{
    ActivePattern = SCANNER;
    Interval = interval;
    TotalSteps = (strip.numPixels() - 1) * 2;
    Color1 = color1;
    Index = 0;
}

// Update the Scanner Pattern
void NeoPixel_Animator::ScannerUpdate()
{
    for (int i = 0; i < PixelCount; i++)
    {
        if (i == Index)  // Scan Pixel to the right
        {
             strip.setPixelColor(i+PixelCount, Color1);
        }
        else if (i == TotalSteps - Index) // Scan Pixel to the left
        {
             strip.setPixelColor(i+PixelCount, Color1);
        }
        else // Fading tail
        {
             strip.setPixelColor(i+PixelCount, DimColor(strip.getPixelColor(i)));
        }
    }
    strip.show();
    Increment();
}

// Initialize for a Fade
void NeoPixel_Animator::Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
{
    ActivePattern = FADE;
    Interval = interval;
    TotalSteps = steps;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
}

// Update the Fade Pattern
void NeoPixel_Animator::FadeUpdate()
{
    // Calculate linear interpolation between Color1 and Color2
    // Optimise order of operations to minimize truncation error
    uint8_t redComponent = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
    uint8_t greenComponent = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
    uint8_t blueComponent = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

    ColorSet(strip.Color(redComponent, greenComponent, blueComponent));
    // strip.show();
    Increment();
}

// Calculate 50% dimmed version of a color (used by ScannerUpdate)
uint32_t NeoPixel_Animator::DimColor(uint32_t color)
{
    // Shift R, G and B components one bit to the right
    uint32_t dimColor = strip.Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
    return dimColor;
}

// Set all pixels to a color (synchronously)
void NeoPixel_Animator::ColorSet(uint32_t color)
{
    for (int i = 0; i < PixelCount; i++)
    {
      int index = PixelIndex + i;
      strip.setPixelColor(index, color);
    }
}

// Returns the Red-component of a 32-bit color
uint8_t NeoPixel_Animator::Red(uint32_t color)
{
    return (color >> 16) & 0xFF;
}

// Returns the Green-component of a 32-bit color
uint8_t NeoPixel_Animator::Green(uint32_t color)
{
    return (color >> 8) & 0xFF;
}

// Returns the Blue-component of a 32-bit color
uint8_t NeoPixel_Animator::Blue(uint32_t color)
{
    return color & 0xFF;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t NeoPixel_Animator::Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85)
    {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
        WheelPos -= 170;
        return (WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

// Update the pattern
void NeoPixel_Animator::Update()
{
    if((millis() - lastUpdate) > Interval) // time to update
    {
        lastUpdate = millis();
        switch(ActivePattern)
        {
            case RAINBOW_CYCLE:
                RainbowCycleUpdate();
                break;
            case THEATER_CHASE:
                TheaterChaseUpdate();
                break;
            case COLOR_WIPE:
                ColorWipeUpdate();
                break;
            case SCANNER:
                ScannerUpdate();
                break;
            case FADE:
                FadeUpdate();
                break;
            default:
                break;
        }
    }
}
