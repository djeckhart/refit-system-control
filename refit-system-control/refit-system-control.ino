/*
 * Refit System Control
 * Author: Frankie Winters
 * License: WTFPL
 */
#include "Curve.h"
#include "LEDFader.h"
#include "LedFlasher.h"
#include "LedStrobeFlasher.h"
#include "Adafruit_NeoPixel.h"
#include "NeoPixel_Animator.h"

// pin assignments
const byte FloodlightsPin = 11; // PWM
const byte StrobesPin = 10; // PWM
const byte NavigationPin = 9;
const byte ButtonPin = 2;  // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.
const byte WarpDrivetrainPin = 6; // Digital IO pin connected to the NeoPixels.
const byte WarpDrivetrainPixelCount = 4;
// The offset for each component's pixels in the strip.
const byte ImpulseCrystalPixel = 0;
const byte ImpulseExhaustsPixel = 1; // 2 pixels
const byte DeflectorDishPixel = 3;

// Navigation markers flash on and off, strobes remain 5% or so even when off.
LEDFader floodlights = LEDFader(FloodlightsPin);
LedFlasher navigationMarkers = LedFlasher(NavigationPin, 1000, 3000, false);
LedStrobeFlasher strobes = LedStrobeFlasher(StrobesPin,   100, 900, false);
// A Neopixel Object to manage the series of lights that represent the warp drive, impulse engines, etc..
Adafruit_NeoPixel drivetrain = Adafruit_NeoPixel(WarpDrivetrainPixelCount, WarpDrivetrainPin, NEO_RGB + NEO_KHZ800);
// Animators for each component represented by a range of pixels in the drivetrain.
NeoPixel_Animator impulseCrystal = NeoPixel_Animator(drivetrain, ImpulseCrystalPixel, 1, &impulseCrystalComplete);
NeoPixel_Animator impulseExhausts = NeoPixel_Animator(drivetrain, ImpulseExhaustsPixel, 2, &impulseExhaustsComplete);
NeoPixel_Animator deflectorDish = NeoPixel_Animator(drivetrain, DeflectorDishPixel, 2, &deflectorDishComplete);

// Colors
uint32_t black = drivetrain.Color(0,0,0);
uint32_t impulseWhite = drivetrain.Color(230, 255, 0);
uint32_t red = drivetrain.Color(20, 248, 0);
uint32_t turquoise = drivetrain.Color(128, 0, 153);

// Pieces of the finite state machine.
typedef enum {
  initialState,
  wantStandby,
  wantNavigation,
  wantStrobes,
  wantImpulse,
  wantWarp,
  steadyAsSheGoes
} states;
states shipStatus = initialState;
unsigned long lastStateChange = 0;
unsigned long timeInThisState = 1000;
int canSheTakeAnyMore = 0; // which is to say we start in impulse mode
bool lastButtonState = HIGH;

void readButton()
{
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
  pinMode(FloodlightsPin, OUTPUT);
  pinMode(NavigationPin, OUTPUT);
  pinMode(StrobesPin, OUTPUT);
  pinMode(ButtonPin, INPUT_PULLUP);
  strobes.begin();
  strobes.off();
  navigationMarkers.begin();
  navigationMarkers.off();
  drivetrain.begin();
  //analogWrite(FloodlightsPin, 252);
  floodlights.set_value(255);
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
  navigationMarkers.update();
  floodlights.update();
  strobes.update();
  impulseCrystal.Update();
  impulseExhausts.Update();
  deflectorDish.Update();
  drivetrain.show();
}  // end of loop

void doStateChange ()
{
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
      shipStatus = steadyAsSheGoes;
      break;

    case wantWarp:
      transitionToWarpPower();
      shipStatus = steadyAsSheGoes;
      break;

    case wantImpulse:
      transitionToImpulsePower();
      shipStatus = steadyAsSheGoes;
      break;

    case wantStandby:
      transitionToStandby();
      shipStatus = steadyAsSheGoes;
      break;

    case steadyAsSheGoes:
      //transitionToImpulsePower();
      break;
  }  // end of switch on shipStatus
}  // end of doStateChange

void advanceState()
{
  canSheTakeAnyMore++;
  if (canSheTakeAnyMore == 1) {
    shipStatus = wantImpulse;
  } else if (canSheTakeAnyMore == 2){
    shipStatus = wantWarp;
  } else {
    shipStatus = wantStandby;
    canSheTakeAnyMore = 0;
  }
  doStateChange();
 }

void beginMatterAntimatterReaction()
{
  impulseCrystal.Fade(black,
                      impulseWhite,
                      255,
                      10,
                      FORWARD);
  deflectorDish.Fade(black,
                       impulseWhite,
                       155,
                       10,
                      FORWARD);
  floodlights.fade(254, 1200);
}

void transitionToStandby()
{
  Serial.println("Standing By. Shuttle Approach Ready.");
  floodlights.fade(0, 1200);
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel),
                      black,
                      155,
                      10,
                      FORWARD);
  impulseCrystal.Fade(drivetrain.getPixelColor(ImpulseCrystalPixel),
                      impulseWhite,
                      155,
                      10,
                      FORWARD);
  deflectorDish.Fade(drivetrain.getPixelColor(DeflectorDishPixel),
                       impulseWhite,
                       155,
                       10,
                      FORWARD);

}

void transitionToImpulsePower()
{
  Serial.println("\"Impulse engines engaged, Captain.\"");
  floodlights.fade(254, 750);
  // Turn the Impulse Exhausts Red
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel),
                      red,
                      155,
                      10,
                      FORWARD);
  // Turn the Impulse Crystal Impulse Yellow
  impulseCrystal.Fade(drivetrain.getPixelColor(ImpulseCrystalPixel),
                      impulseWhite,
                      155,
                      10,
                      FORWARD);
  deflectorDish.Fade(drivetrain.getPixelColor(DeflectorDishPixel),
                       impulseWhite,
                       155,
                       10,
                      FORWARD);
}

void transitionToWarpPower()
{
  Serial.println("\"Warp speed at your command.\"");
  floodlights.fade(254, 750);
  impulseCrystal.Fade(drivetrain.getPixelColor(ImpulseCrystalPixel),
                       turquoise,
                       155,
                       10,
                      FORWARD);
  deflectorDish.Fade(drivetrain.getPixelColor(DeflectorDishPixel),
                       turquoise,
                       155,
                       10,
                      FORWARD);
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel),
                      black,
                      155,
                      10,
                      FORWARD);
}

void impulseCrystalComplete()
{
    // Serial.println("impulseCrystalComplete()");
    impulseCrystal.ActivePattern = NONE;
}

void impulseExhaustsComplete()
{
  // Serial.println("impulseExhaustsComplete()");
  impulseExhausts.ActivePattern = NONE;
}

void deflectorDishComplete()
{
  // Serial.println("deflectorDishComplete()");
  deflectorDish.ActivePattern = NONE;
}
