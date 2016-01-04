#include "LedFlasher.h"
#include "LedStrobeFlasher.h"
#include "Adafruit_NeoPixel.h"
#include "NeoPixel_Animator.h"
/*
 Refit System Control
 Author: Frankie Winters
 License: WTFPL
 **Startup Schedule**
 - Mains, Life Support: ON
 - navigationMarkers Markers: ON
 - Deflector and Impulse Crystal: Fade to Amber
 - Strobes: ON
 **Warp**
 - Deflector and Impulse Crystal: Fade to Aqua
 - Warp Nacelle Flux Chiller Grills: ON
 **Impulse**
 - Deflector and Impulse Crystal: Fade to Aqua
 - Warp Nacelle Flux Chiller Grills: OFF
 - Impulse Exhaust: ON
*/
// pin assignments
const byte StrobesPin = 10; // PWM
const byte NavigationPin = 9;
const byte ButtonPin = 2;  // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.
const byte WarpDrivetrainPin = 6; // Digital IO pin connected to the NeoPixels.
const byte WarpDrivetrainPixelCount = 3;

// A Neopixel Object to manage the series of lights that represent the warp drive
Adafruit_NeoPixel warpDrivetrain = Adafruit_NeoPixel(WarpDrivetrainPixelCount, WarpDrivetrainPin, NEO_GRB);
// Animators to run the effects of each compnent
NeoPixel_Animator ImpulseCrystal(warpDrivetrain, 0, 1, NULL);
NeoPixel_Animator impulseExhausts = NeoPixel_Animator(warpDrivetrain, 1, 2, NULL);
NeoPixel_Animator deflectorDish = NeoPixel_Animator(warpDrivetrain, 3, 1, NULL);

LedStrobeFlasher strobes(StrobesPin,   100, 900, true);
LedFlasher navigationMarkers(NavigationPin, 1000, 3000, true);

  // states for the finite stae machine
typedef enum {
  initialState,
  wantNavigation,
  wantStrobes,
  wantWarp,
  standby
} states;
states shipStatus = initialState;
unsigned long lastStateChange = 0;
unsigned long timeInThisState = 1000;

void doStateChange () {
  lastStateChange = millis ();    // when we last changed states
  timeInThisState = 1000;         // default one second between states
  switch (shipStatus)
  {
    case initialState:
      shipStatus = wantNavigation;
      break;

    case wantNavigation:
      navigationMarkers.on();
      timeInThisState = 3000;
      shipStatus = wantStrobes;
      break;

    case wantStrobes:
      strobes.on();
      shipStatus = standby;
      break;

    case wantWarp:
      warpPower();
      break;

    case standby:
      impulsePower();
      break;
  }  // end of switch on shipStatus
}  // end of doStateChange

void advanceState() {
   switch (shipStatus)
   {
      case wantWarp:
      shipStatus = standby;
        break;

      case standby:
      shipStatus = wantWarp;
          break;

      default:
      // shipStatus = standby;
        break;
   }
   doStateChange();
}

void impulsePower() {

  // ImpulseCrystal.Fade(warpDrivetrain.getPixelColor(0),
  //  warpDrivetrain.Color(125, 100, 41), 500, 50, FORWARD);
  // warpDrivetrain.setPixelColor(0, 125, 100, 41);
  warpDrivetrain.setPixelColor(1, 248, 7, 4);
  warpDrivetrain.setPixelColor(2, 248, 7, 4);
  warpDrivetrain.show();
}

void warpPower() {
  warpDrivetrain.setPixelColor(0, 0, 155, 184);
  warpDrivetrain.setPixelColor(1, 0, 0, 0);
  warpDrivetrain.setPixelColor(2, 0, 0, 0);
  warpDrivetrain.show();
}

bool lastButtonState = HIGH;

void readButton() {

    // Get current button state.
    bool buttonState = digitalRead(ButtonPin);
    // Check if state changed from high to low (button press).
    if (buttonState == LOW && lastButtonState == HIGH) {
      // Short delay to debounce button.
      delay(20);
      // Check if button is still low after debounce.
      buttonState = digitalRead(ButtonPin);
      if (buttonState == LOW) {
        // advanceState();
        advanceState();
      }
    }
    // Set the last button state to the old state.
    lastButtonState = buttonState;
}

void setup ()
{
  Serial.begin(9600);
  pinMode (NavigationPin, OUTPUT);
  pinMode (StrobesPin, OUTPUT);
  pinMode(ButtonPin, INPUT_PULLUP);
  strobes.begin ();
  navigationMarkers.begin ();
  warpDrivetrain.begin();
  warpDrivetrain.show(); // Initialize all pixels to 'off'
}

void loop ()
{
  if (millis () - lastStateChange >= timeInThisState)
  {
    doStateChange ();
  }
  // update faders, flashers
  navigationMarkers.update ();
  strobes.update ();
  ImpulseCrystal.Update();
  readButton();
}  // end of loop
