/*
 Based upon LedFlasher by Nick Gammon
 Date: 23 December 2012
 FEW Modifed Dec 2015 to use analogWrite instead of digitalWrite so the 'off' state will still appear at half brightness.
*/

#include <LedStrobeFlasher.h>

// constructor
LedStrobeFlasher::LedStrobeFlasher (const byte pin, const unsigned long timeOff, const unsigned long timeOn, const bool active) :
                   pin_ (pin), timeOff_ (timeOff), timeOn_ (timeOn)
   {
   currentInterval_ = timeOff_;
   startTime_ = 0;
   active_ = active;
   }  // end of LedStrobeFlasher::LedStrobeFlasher

// set pin to output, get current time
void LedStrobeFlasher::begin ()
  {
  pinMode (pin_, OUTPUT);
  digitalWrite (pin_, LOW);
  startTime_ = millis ();
  }  // end of LedStrobeFlasher::begin

// call from loop to flash the LED
void LedStrobeFlasher::update ()
  {
  // do nothing if not active
  if (!active_)
    return;

  unsigned long now = millis ();
  // if time to do something, do it
  if (now - startTime_ >= currentInterval_)
    {
    if (digitalRead (pin_) == LOW)
      {
      digitalWrite(pin_, HIGH);
      currentInterval_ = timeOn_;
      }
    else
      {
      analogWrite(pin_, 128);
      currentInterval_ = timeOff_;
      }
    startTime_ = now;
    } // end of if

  } // end of LedStrobeFlasher::update

 // activate this LED
 void LedStrobeFlasher::on ()
   {
   active_ = true;
   startTime_ = millis ();
   currentInterval_ = timeOff_;
   }  // end of LedStrobeFlasher::on

 // deactivate this LED
 void LedStrobeFlasher::off ()
   {
   active_ = false;
   digitalWrite(pin_, LOW);
   }  // end of LedStrobeFlasher::off

 // is it active?
 bool LedStrobeFlasher::isOn () const
   {
   return active_;
   }  // end of LedStrobeFlasher::isOn
