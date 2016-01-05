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
NeoPixel_Animator ImpulseCrystal(warpDrivetrain, 0, 1, &ImpulseCrystalComplete);
NeoPixel_Animator ImpulseExhausts(warpDrivetrain, 1, 2, &ImpulseExhaustsComplete);
// NeoPixel_Animator deflectorDish = NeoPixel_Animator(warpDrivetrain, 3, 1, NULL);

LedStrobeFlasher strobes(StrobesPin,   100, 900, true);
LedFlasher navigationMarkers(NavigationPin, 1000, 3000, true);

// Colors
uint32_t black = warpDrivetrain.Color(0,0,0);
uint32_t lightYellow = warpDrivetrain.Color(85, 160, 160);
uint32_t red = warpDrivetrain.Color(248, 7, 4);
uint32_t turquoise = warpDrivetrain.Color(0, 155, 185);

  // states for the finite stae machine
typedef enum {
  initialState,
  wantNavigation,
  wantStrobes,
  wantImpulse,
  wantWarp,
  standby
} states;
states shipStatus = initialState;
unsigned long lastStateChange = 0;
unsigned long timeInThisState = 1000;
bool canSheTakeAnyMore = true;
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
      beginMatterAntimatterReaction();
      shipStatus = wantImpulse;
      break;

    case wantWarp:
      warpPower();
      shipStatus = standby;
      break;

    case wantImpulse:
      impulsePower();
      shipStatus = standby;
      break;

    case standby:
      //impulsePower();
      break;
  }  // end of switch on shipStatus
}  // end of doStateChange

void advanceState()
{
  Serial.println("advance state");
  if (canSheTakeAnyMore == true) {
    shipStatus = wantWarp;
  } else {
    shipStatus = wantImpulse;
  }
   doStateChange();
   canSheTakeAnyMore = !canSheTakeAnyMore;
}

void beginMatterAntimatterReaction()
{
  ImpulseCrystal.Fade(black,
                      lightYellow,
                      255,
                      10,
                      FORWARD);
}

void impulsePower()
{
  Serial.println("Impulse engines engaged, Captain.");
  ImpulseCrystal.Fade(warpDrivetrain.getPixelColor(0),
                      lightYellow,
                      255,
                      10,
                      FORWARD);
  ImpulseExhausts.Fade(warpDrivetrain.getPixelColor(1),
                      red,
                      155,
                      10,
                      FORWARD);
}

void ImpulseCrystalComplete()
{
    // Serial.println("ImpulseCrystalComplete()");
    ImpulseCrystal.ActivePattern = NONE;
}

void ImpulseExhaustsComplete()
{
  // Serial.println("ImpulseExhaustsComplete()");
  ImpulseExhausts.ActivePattern = NONE;
}

void warpPower()
{
  Serial.println("Warp speed at your command.");;
  // warpDrivetrain.setPixelColor(0, 0, 155, 184);
  ImpulseCrystal.Fade(warpDrivetrain.getPixelColor(0),
                       turquoise,
                       155,
                       10,
                      FORWARD);
  ImpulseExhausts.Fade(warpDrivetrain.getPixelColor(1),
                      black,
                      155,
                      10,
                      FORWARD);
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
  // warpDrivetrain.show(); // Initialize all pixels to 'off'
}

void loop ()
{
  if (millis () - lastStateChange >= timeInThisState)
  {
    doStateChange ();
  }
  // update faders, flashers
  readButton();
  navigationMarkers.update ();
  strobes.update ();
  ImpulseCrystal.Update();
  ImpulseExhausts.Update();
  warpDrivetrain.show();
}  // end of loop
