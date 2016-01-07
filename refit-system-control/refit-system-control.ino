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
Adafruit_NeoPixel warpDrivetrain = Adafruit_NeoPixel(WarpDrivetrainPixelCount, WarpDrivetrainPin, NEO_RGB + NEO_KHZ800);
// Animators to run the effects of each compnent
const byte ImpulseCrystalPixel = 0;
const byte ImpulseExhaustsPixel = 1;
NeoPixel_Animator ImpulseCrystal = NeoPixel_Animator(warpDrivetrain, ImpulseCrystalPixel, 1, &ImpulseCrystalComplete);
NeoPixel_Animator ImpulseExhausts = NeoPixel_Animator(warpDrivetrain, ImpulseExhaustsPixel, 2, &ImpulseExhaustsComplete);
// NeoPixel_Animator deflectorDish = NeoPixel_Animator(warpDrivetrain, 3, 1, NULL);

LedStrobeFlasher strobes(StrobesPin,   100, 900, false);
LedFlasher navigationMarkers(NavigationPin, 1000, 3000, false);

// Colors
uint32_t black = warpDrivetrain.Color(0,0,0);
uint32_t impulseWhite = warpDrivetrain.Color(240, 255, 0);
uint32_t red = warpDrivetrain.Color(20, 248, 0);
uint32_t turquoise = warpDrivetrain.Color(128, 0, 153);

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
bool canSheTakeAnyMore = false; // which is to say we start in impulse mode
void doStateChange () {
  lastStateChange = millis ();    // when we last changed states
  timeInThisState = 1000;         // default one second between states
  switch (shipStatus)
  {
    case initialState:
      beginMatterAntimatterReaction();
      timeInThisState = 2000;
      shipStatus = wantNavigation;
      break;

    case wantNavigation:
      navigationMarkers.on();
      timeInThisState = 2000;
      shipStatus = wantStrobes;
      break;

    case wantStrobes:
      strobes.on();
      shipStatus = standby;
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
                      impulseWhite,
                      255,
                      10,
                      FORWARD);
}

void impulsePower()
{
  Serial.println("\"Impulse engines engaged, Captain.\"");
  // Turn the Impulse Exhausts Red
  ImpulseExhausts.Fade(warpDrivetrain.getPixelColor(ImpulseExhaustsPixel),
                      red,
                      155,
                      10,
                      FORWARD);
  ImpulseCrystal.Fade(warpDrivetrain.getPixelColor(ImpulseCrystalPixel),
                      impulseWhite,
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
  Serial.println("\"Warp speed at your command.\"");;
  ImpulseCrystal.Fade(warpDrivetrain.getPixelColor(ImpulseCrystalPixel),
                       turquoise,
                       155,
                       10,
                      FORWARD);
  ImpulseExhausts.Fade(warpDrivetrain.getPixelColor(ImpulseExhaustsPixel),
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
  pinMode(NavigationPin, OUTPUT);
  pinMode(StrobesPin, OUTPUT);
  pinMode(ButtonPin, INPUT_PULLUP);
  strobes.begin();
  strobes.off();
  navigationMarkers.begin();
  navigationMarkers.off();
  warpDrivetrain.begin();

  // warpDrivetrain.show(); // Initialize all pixels to 'off'
  floodlights.set_curve(Curve::exponential);
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
