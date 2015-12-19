#include <LedFlasher.h>

// The Enterprise

// pin assignments
const byte StrobesPin = 10; // PWM
const byte NavigationPin = 9;
const byte CabinPin = 1;

// Flashers                pin          off-time  on-time       on?
LedFlasher strobes    (StrobesPin,        900,       100,     false);
LedFlasher navigation (NavigationPin,     3000,     1000,     false);

// states for the state machine
typedef enum
{
  initialState,
  wantCabin,                 // ALWAYS ON
  wantNavigation,            // ALWAYS ON
  wantStrobes,               // ALWAYS ON
  standby
} states;

// state machine variables
states state = initialState;
unsigned long lastStateChange = 0;
unsigned long timeInThisState = 1000;

void setup ()
{
  pinMode (NavigationPin, OUTPUT); // FIXME
  pinMode (StrobesPin, OUTPUT);

  strobes.begin ();
  navigation.begin ();
}  // end of setup

void doStateChange ()
{
  lastStateChange = millis ();    // when we last changed states
  timeInThisState = 1000;         // default one second between states

  switch (state)
  {
    case initialState:
      state = wantCabin;
      break;

    case wantCabin:
      digitalWrite (CabinPin, HIGH);
      state = wantStrobes;
      break;

    case wantStrobes:
      strobes.on();
      state = wantNavigation;
      break;

    case wantNavigation:
      navigation.on();
      state = standby;
      break;
    // what next?
  }  // end of switch on state
}  // end of doStateChange

void loop ()
{
  if (millis () - lastStateChange >= timeInThisState)
    doStateChange ();
  // update faders, flashers
  navigation.update ();
  strobes.update ();
  // other stuff here like testing switches
}  // end of loop
