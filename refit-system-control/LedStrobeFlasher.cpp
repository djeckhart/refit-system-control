/*
 Based upon LedFlasher by Nick Gammon
 Date: 23 December 2012
 FEW Modifed Dec 2015
*/

#include "LedStrobeFlasher.h"

// constructor
LedStrobeFlasher::LedStrobeFlasher (const byte pin, const unsigned long timeOff, const unsigned long timeOn, const bool active) :
                   pin_ (pin), timeOff_ (timeOff), timeOn_ (timeOn)
   {
   currentInterval_ = timeOff_;
   startTime_ = 0;
   active_ = active;
   oscillator_ = false;
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
  if (!active_) {
    analogWrite(pin_, 255);
    return;
  }

  unsigned long now = millis ();
  // if time to do something, do it
  if (now - startTime_ >= currentInterval_)
    {
    if (oscillator_ == true)
      {
      analogWrite(pin_, 85);
      currentInterval_ = timeOff_;
      oscillator_ = false;
      }
    else
      {
      analogWrite(pin_, 250);
      currentInterval_ = timeOn_;
      oscillator_ = true;
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
