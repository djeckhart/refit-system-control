#include <LedFlasher.h>
#include <LedStrobeFlasher.h>
#include <Adafruit_NeoPixel.h>
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

// Flashers                    pin          off-time  on-time       on?
LedStrobeFlasher strobes     (StrobesPin,        900,       100,     false);
LedFlasher navigationMarkers (NavigationPin,     3000,     1000,     false);

// states for the finite stae machine
typedef enum
{
  initialState,
  wantNavigation,
  wantStrobes,
  standby
} states;

// shipStatus machine variables
states shipStatus = initialState;
unsigned long lastStateChange = 0;
unsigned long timeInThisState = 1000;

void setup ()
{
  Serial.begin(9600);
  pinMode (NavigationPin, OUTPUT);
  pinMode (StrobesPin, OUTPUT);
  strobes.begin ();
  navigationMarkers.begin ();
}

void doStateChange ()
{
  lastStateChange = millis ();    // when we last changed states
  timeInThisState = 1000;         // default one second between states

  switch (shipStatus)
  {
    case initialState:
      shipStatus = wantNavigation;
      break;

    case wantNavigation:
      navigationMarkers.on();
      Serial.print("Starting Navigation Markers.");
      // timeInThisState = 6000;
      shipStatus = wantStrobes;
      break;

    case wantStrobes:
      strobes.on();
      shipStatus = standby;
      break;

    // what next?
  }  // end of switch on shipStatus
}  // end of doStateChange

void loop ()
{
  if (millis () - lastStateChange >= timeInThisState)
    doStateChange ();
  // update faders, flashers
  navigationMarkers.update ();
  strobes.update ();
  // other stuff here like testing switches
}  // end of loop
