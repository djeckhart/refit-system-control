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
const byte ButtonPin = 2;  // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

const byte WarpDrivetrainPin = 6; // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 3

// Parameter 1 = number of pixels in warpDrivetrain,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED warpDrivetrain), correct for neopixel stick
Adafruit_NeoPixel warpDrivetrain = Adafruit_NeoPixel(PIXEL_COUNT, WarpDrivetrainPin, NEO_GRB);

// Flashers                    pin          on-time,  off-time, on?
LedStrobeFlasher strobes     (StrobesPin,   100, 900, true);
LedFlasher navigationMarkers (NavigationPin, 1000, 3000, true);

// states for the finite stae machine
typedef enum
{
  initialState,
  wantNavigation,
  wantStrobes,
  wantWarp,
  standby
} states;

// shipStatus machine variables
states shipStatus = initialState;
unsigned long lastStateChange = 0;
unsigned long timeInThisState = 1000;


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

void advanceState()
{
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
  warpDrivetrain.setPixelColor(0, 247, 200, 84);
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

}  // end of loop
