/*
 * Daystrom FWG1-M6-1701 Refit Systems Controller
 * Author: Frankie Winters
 * License: WTFPL
 * ./arduino-builder -hardware ./hardware -tools ./hardware/tools/avr -tools ./tools-builder -libraries ./libraries -fqbn arduino:avr:nano ~/Documents/repositories/refit-system-control/Daystrom_FWG1-M6-1701_Refit_Systems_Controller/Daystrom_FWG1-M6-1701_Refit_Systems_Controller.ino
 */
#include "Curve.h"
#include "LEDFader.h"
#include "LedFlasher.h"
#include "LedStrobeFlasher.h"
#include "Adafruit_NeoPixel.h"
#include "NeoPixel_Animator.h"
#include "Adafruit_NeoPatterns.h"

// Pin assignments
const byte FloodlightsPin = 11;    // Needs PWM connected to MOSFET
const byte StrobesPin = 10;        // Needs PWM connected to MOSFET
const byte NavigationPin = 9;      // Needs PWM connected to MOSFET
const byte FluxChillersPin = 5;    // Digital IO pin connected to NeoPixels.
const byte ShuttleApproachPin = 4; // Digital IO pin connected to the button.
const byte DrivetrainPin = 3;      // Digital IO pin connected to NeoPixels.
const byte ButtonPin = 2;          // Digital IO pin connected to NeoPixels.

// The offsets for each component's pixels in the strip. (number of pixels for each component)
const byte ImpulseCrystalPixel = 0;   // (1 pixel)
const byte ImpulseExhaustsPixel = 1;  // (2 pixel)
const byte DeflectorDishPixel = 3;    // (1 pixel)

// Length of the Neopixel Chains
const byte DrivetrainLength = 4;
const byte FluxChillersLength = 30;
const byte ShuttleApproachLength = 32;

// Controllers for circuits of dumb old LEDs.
// - Strobes remain 5% or so even when off.
LedStrobeFlasher strobes = LedStrobeFlasher(StrobesPin,  100, 900, false);
// - Navigation markers flash on and off
LedFlasher navigationMarkers = LedFlasher(NavigationPin, 3000, 1000, false);
// - I haven't been bothered to mess about with sequential floodlights
LEDFader floodlights = LEDFader(FloodlightsPin);

// Controllers for the strands of Neopixels connected to the Arduino.
Adafruit_NeoPixel drivetrain = Adafruit_NeoPixel(DrivetrainLength, DrivetrainPin, NEO_RGB + NEO_KHZ800);
NeoPatterns shuttleApproach = NeoPatterns(ShuttleApproachLength, ShuttleApproachPin, NEO_RGB + NEO_KHZ800, &shuttleApproachComplete);
NeoPatterns fluxChillers = NeoPatterns(FluxChillersLength, FluxChillersPin, NEO_RGB + NEO_KHZ800, &fluxChillersComplete);

// Controllers for “logical” subcomponents of the drivetrain and shuttle approach.
NeoPixel_Animator impulseCrystal = NeoPixel_Animator(drivetrain, ImpulseCrystalPixel, 1, &impulseCrystalComplete);
NeoPixel_Animator impulseExhausts = NeoPixel_Animator(drivetrain, ImpulseExhaustsPixel, 2, &impulseExhaustsComplete);
NeoPixel_Animator deflectorDish = NeoPixel_Animator(drivetrain, DeflectorDishPixel, 2, &deflectorDishComplete);

// Colors
uint32_t drivetrainBlack = drivetrain.Color(0,0,0);
uint32_t impulseWhite = drivetrain.Color(230, 255, 0);
uint32_t drivetrainRed = drivetrain.Color(20, 248, 0);
uint32_t warpBlue = drivetrain.Color(128, 0, 153);
uint32_t fluxChilllerViolet = fluxChillers.Color(0, 159, 255);

// Pieces of the finite state machine and button business.
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
bool lastButtonState = HIGH;
uint8_t howMuchMoreOfThisSheCanTake = 0;

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
  // fluxChillers.setBrightness(127);
  fluxChillers.ColorSet(drivetrainBlack);
  shuttleApproach.begin();
  shuttleApproach.ColorSet(drivetrainBlack);
  drivetrain.begin();
  for (int pxl = 0; pxl < DrivetrainLength; pxl++) {
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
    // This will be driven INPUT so the switch should pull the pin to ground momentarily.  On a high -> low transition the button press logic will execute.
    // Get current button state.
    bool buttonState = digitalRead(ButtonPin);
    // Check if state changed from high to low (button press).
    if (buttonState == LOW && lastButtonState == HIGH) {
      // Short delay to debounce button.
      delay(20);
      // Check if button is still low after debounce.
      buttonState = digitalRead(ButtonPin);
      if (buttonState == LOW) {
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
  impulseCrystal.Fade(drivetrainBlack, impulseWhite, 255, 10, FORWARD);
  deflectorDish.Fade(drivetrainBlack, impulseWhite, 155, 10, FORWARD);
  floodlights.fade(254, 1200);
}

void transitionToStandby()
{
  Serial.println("\"Standing By. Shuttle Approach Ready.\"");
  deflectorDish.Fade(drivetrain.getPixelColor(DeflectorDishPixel), impulseWhite, 155, 10, FORWARD);
  fluxChillers.OnComplete = &disengageWarpDriveComplete;
  fluxChillers.Fade(fluxChillers.getPixelColor(0), fluxChilllerViolet, 80, 10, FORWARD);
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel), drivetrainBlack, 155, 10, FORWARD);
  impulseCrystal.Fade(drivetrain.getPixelColor(ImpulseCrystalPixel), impulseWhite, 155, 10, FORWARD);
}

void disengageWarpDriveComplete()
{
  fluxChillers.OnComplete = &fluxChillersComplete;
  fluxChillers.Fade(fluxChillers.getPixelColor(0), drivetrainBlack, 80, 10, FORWARD);
  floodlights.fade(0, 1200);
  shuttleApproach.OnComplete = NULL;
  shuttleApproach.ShuttleApproach(125);
}

void transitionToImpulsePower()
{
  Serial.println("\"Impulse engines engaged, Captain.\"");
  floodlights.fade(254, 750);
  shuttleApproach.OnComplete = &shuttleApproachComplete;
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel), drivetrainRed, 155, 10, FORWARD);
  impulseCrystal.Fade(drivetrain.getPixelColor(ImpulseCrystalPixel), impulseWhite, 155, 10, FORWARD);
  deflectorDish.Fade(drivetrain.getPixelColor(DeflectorDishPixel), impulseWhite, 155, 10, FORWARD);
}

void transitionToWarpPower()
{
  Serial.println("\"Warp speed at your command.\"");
  floodlights.fade(254, 750);
  fluxChillers.OnComplete = &engageWarpIntermixStage0Complete;
  fluxChillers.Fade(fluxChillers.getPixelColor(0), fluxChilllerViolet, 155, 10, FORWARD);
  impulseCrystal.Fade(drivetrain.getPixelColor(ImpulseCrystalPixel), warpBlue, 225, 10, FORWARD);
  deflectorDish.Fade(drivetrain.getPixelColor(DeflectorDishPixel), warpBlue, 225, 10, FORWARD);
  impulseExhausts.Fade(drivetrain.getPixelColor(ImpulseExhaustsPixel), drivetrainBlack, 75, 10, FORWARD);
}

void engageWarpIntermixStage0Complete()
{
  fluxChillers.OnComplete = &engageWarpIntermixStage1Complete;
  fluxChillers.Fade(fluxChillers.getPixelColor(0), fluxChillers.Color(255,255,255), 25, 5, FORWARD);
}

void engageWarpIntermixStage1Complete()
{
  fluxChillers.OnComplete = &engageWarpIntermixStage2Complete;
  fluxChillers.Fade(fluxChillers.getPixelColor(0), fluxChillers.Color(255,255,255), 25, 5, FORWARD);
}

void engageWarpIntermixStage2Complete()
{
  fluxChillers.OnComplete = &fluxChillersComplete;
  fluxChillers.Fade(fluxChillers.getPixelColor(0), warpBlue, 75, 5, FORWARD);
}

void fluxChillersComplete()
{
  fluxChillers.ActivePattern = NONE;
};

void shuttleApproachComplete()
{
  shuttleApproach.ActivePattern = NONE;
  shuttleApproach.ColorSet(drivetrainBlack);
};

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

// “I’m givin’ her all she’s got, Captain!”