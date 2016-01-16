/*
 * Daystrom FWG1-M6-1701 Refit Systems Controller
 * Author: Frankie Winters
 * License: WTFPL
 */
#include "Curve.h"
#include "LEDFader.h"
#include "LedFlasher.h"
#include "LedStrobeFlasher.h"
#include "Adafruit_NeoPixel.h"
#include "NeoPixel_Animator.h"
#include "Adafruit_NeoPatterns.h"

// pin assignments
const byte DrivetrainPin = 12; // Digital IO pin connected to NeoPixels.
const byte FluxChillersPin = 11;  // Digital IO pin connected to NeoPixels.
const byte StrobesPin = 10; // Needs PWM
const byte NavigationPin = 9; // Needs PWM
const byte FloodlightsPin = 5; // Needs PWM
const byte ShuttleApproachPin = 4;
const byte ButtonPin = 2;  // Digital IO pin connected to the button.  This will be driven with a pull-up resistor so the switch should pull the pin to ground momentarily.  On a high -> low transition the button press logic will execute.
const byte DrivetrainPixelCount = 4;
const byte FluxChillerPixelCount = 30;
const byte ShuttleApproachPixelCount = 16;
// The offset for each component's pixels in the strip.
const byte ImpulseCrystalPixel = 0;
const byte ImpulseExhaustsPixel = 1; // 2 pixels
const byte DeflectorDishPixel = 3;

// Navigation markers flash on and off, strobes remain 5% or so even when off.
LedStrobeFlasher strobes = LedStrobeFlasher(StrobesPin,   100, 900, false);
LedFlasher navigationMarkers = LedFlasher(NavigationPin, 1000, 3000, false);
LEDFader floodlights = LEDFader(FloodlightsPin);

// A Neopixel Object to manage the series of lights that represent the impulse crystal, impulse exhausts, and deflector
Adafruit_NeoPixel drivetrain = Adafruit_NeoPixel(DrivetrainPixelCount, DrivetrainPin, NEO_RGB + NEO_KHZ800);
NeoPatterns shuttleApproach = NeoPatterns(ShuttleApproachPixelCount, ShuttleApproachPin, NEO_RGB + NEO_KHZ800, NULL);
// Neopatterns object to manage the Magnatomic Flux Chiller Grills.
NeoPatterns fluxChillers = NeoPatterns(FluxChillerPixelCount, FluxChillersPin, NEO_RGB + NEO_KHZ800, &fluxChillersComplete);
// Animators for each component represented by a range of pixels in the drivetrain.
                                                    // (NeoPixel strip, firstPixelIndex, pixelCount, callback);
NeoPixel_Animator impulseCrystal = NeoPixel_Animator(drivetrain, ImpulseCrystalPixel, 1, &impulseCrystalComplete);
NeoPixel_Animator impulseExhausts = NeoPixel_Animator(drivetrain, ImpulseExhaustsPixel, 2, &impulseExhaustsComplete);
NeoPixel_Animator deflectorDish = NeoPixel_Animator(drivetrain, DeflectorDishPixel, 2, &deflectorDishComplete);
// NeoPixel_Animator shuttleApproachStarboard = NeoPixel_Animator(shuttleApproach, 0, 8, &shuttleApproachComplete);
// NeoPixel_Animator shuttleApproachPort = NeoPixel_Animator(shuttleApproach, 0, 16, &shuttleApproachComplete);

// Colors - Note that these are not TODO: What? Both strips are initialized with NEO_RGB.
uint32_t drivetrainBlack = drivetrain.Color(0,0,0);
uint32_t impulseWhite = drivetrain.Color(230, 255, 0);
uint32_t drivetrainRed = drivetrain.Color(20, 248, 0);
uint32_t warpBlue = drivetrain.Color(128, 0, 153);
uint32_t fluxChillerBlue = fluxChillers.Color( 0,159, 255);

// Pieces of the finite state machine.
enum ShipStates {
  offline,
  wantStandby,
  wantNavigation,
  wantStrobes,
  wantImpulse,
  wantWarp,
  steadyAsSheGoes
};
ShipStates shipStatus = offline;
uint16_t lastStateChange = 0;
uint16_t timeInThisState = 1000;
uint8_t howMuchMoreOfThisSheCanTake = 0; // which is to say we go to impulse mode before warp
bool lastButtonState = HIGH;

void setup ()
{
  Serial.begin(9600);
  pinMode(FloodlightsPin, OUTPUT);
  pinMode(NavigationPin, OUTPUT);
  pinMode(StrobesPin, OUTPUT);
  pinMode(FluxChillersPin, OUTPUT);
  pinMode(DrivetrainPin, OUTPUT);
  pinMode(ShuttleApproachPin, OUTPUT);
  pinMode(ButtonPin, INPUT_PULLUP);
  strobes.begin();
  strobes.off();
  navigationMarkers.begin();
  navigationMarkers.off();
  floodlights.set_value(255);
  floodlights.set_curve(Curve::exponential);
  fluxChillers.begin();
  fluxChillers.ColorSet(drivetrainBlack);
  shuttleApproach.begin();
  shuttleApproach.ColorSet(drivetrainBlack);
  // shuttleApproachStarboard.ColorSet(drivetrainBlack);
  // shuttleApproachPort.Direction = REVERSE;
  drivetrain.begin();
  for (int pxl = 0; pxl < DrivetrainPixelCount; pxl++) {
      drivetrain.setPixelColor(pxl, drivetrainBlack);
  }
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
  fluxChillers.Update();
  // shuttleApproachStarboard.Update();
  shuttleApproach.Update();
  drivetrain.show();
}  // end of loop

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

void doStateChange ()
{
  lastStateChange = millis ();    // when we last changed states
  timeInThisState = 1000;         // default one second between states
  switch (shipStatus)
  {
    case offline:
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
  howMuchMoreOfThisSheCanTake++;
  if (howMuchMoreOfThisSheCanTake == 1) {
    shipStatus = wantImpulse;
  } else if (howMuchMoreOfThisSheCanTake == 2){
    shipStatus = wantWarp;
  } else {
    shipStatus = wantStandby;
    howMuchMoreOfThisSheCanTake = 0;
  }
  doStateChange();
 }

void beginMatterAntimatterReaction()
{
  Serial.println("\"Initiating deuterium-antideuterium reacton. Warp core online.\"");
  impulseCrystal.Fade(drivetrainBlack,
                      impulseWhite,
                      255,
                      10,
                      FORWARD);
  deflectorDish.Fade(drivetrainBlack,
                       impulseWhite,
                       155,
                       10,
                      FORWARD);
  floodlights.fade(254, 1200);
}

void transitionToStandby()
{
  Serial.println("\"Standing By. Shuttle Approach Ready.\"");
  floodlights.fade(0, 1200);
  fluxChillers.Fade(fluxChillers.getPixelColor(0), drivetrainBlack, 125, 5, FORWARD);
  shuttleApproach.Scanner(shuttleApproach.Color(55,55,55), 50);
  // shuttleApproachPort.Scanner(impulseWhite, 50);
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel),
                      drivetrainBlack,
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
  // shuttleApproachStarboard.Fade(shuttleApproach.getPixelColor(0), drivetrainBlack, 125, 5, FORWARD);
  shuttleApproach.ColorWipe(drivetrainBlack, 50, FORWARD);

  // Turn the Impulse Exhausts Red
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel),
                      drivetrainRed,
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
  fluxChillers.OnComplete = &fluxChillersStage1Complete;
  fluxChillers.Fade(drivetrainBlack, fluxChillerBlue, 75, 15, FORWARD);
  impulseCrystal.Fade(drivetrain.getPixelColor(ImpulseCrystalPixel),
                       warpBlue,
                       225,
                       10,
                      FORWARD);
  deflectorDish.Fade(drivetrain.getPixelColor(DeflectorDishPixel),
                       warpBlue,
                       225,
                       10,
                      FORWARD);
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel),
                      drivetrainBlack,
                      75,
                      10,
                      FORWARD);
}

void fluxChillersStage1Complete() {
  fluxChillers.OnComplete = &fluxChillersStage2Complete;
  fluxChillers.Fade(fluxChillers.getPixelColor(0), fluxChillers.Color(255,255,255), 25, 5, FORWARD);
}

void fluxChillersStage2Complete() {
  fluxChillers.OnComplete = &fluxChillersComplete;
  fluxChillers.Fade(fluxChillers.getPixelColor(0), warpBlue, 75, 5, FORWARD);
}

void fluxChillersComplete(){
  fluxChillers.ActivePattern = NONE;
};

void shuttleApproachComplete(){};

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
