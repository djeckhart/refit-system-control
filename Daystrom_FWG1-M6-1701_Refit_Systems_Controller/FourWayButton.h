#pragma once
// jb_exe
// Class from the code of JEFF'S ARDUINO BLOG
// http://jmsarduino.blogspot.com/2009/10/4-way-button-click-double-click-hold.html


// How to use me :
// "FWB_Project.pde"
// #include "FWB.h"
// #define BP 0 //the pin where your button is connected
//
// FWB bp;
//
// void OnClick(int pin)
// {
// 	//Your code here
// }
//
// void setup()
// {
// 	bpUP.Configure(BP);
// 	bpUP.OnClick = OnClick;
// }
//
// void loop()
// {
//	// Test button state
//	bp.CheckBP();
// }


// #include "WProgram.h"

#define PULL_UP 1
#define PULL_DOWN 0

class FourWayButton
{
	private:
		int _pin;
		bool _pullMode;

		// Properties //
		////////////////

		// Debounce period to prevent flickering when pressing or releasing the button (in ms)
		int Debounce;
		// Max period between clicks for a double click event (in ms)
		int DblClickDelay;
		// Hold period for a long press event (in ms)
		int LongPressDelay;
		// Hold period for a very long press event (in ms)
		int VLongPressDelay;

		// Variables //
		///////////////

		// Value read from button
		bool _state;
		// Last value of button state
		bool _lastState;
		// whether we're waiting for a double click (down)
		bool _dblClickWaiting;
		// whether to register a double click on next release, or whether to wait and click
		bool _dblClickOnNextUp;
		// whether it's OK to do a single click
		bool _singleClickOK;

		// time the button was pressed down
		long _downTime;
		// time the button was released
		long _upTime;

		// whether to ignore the button release because the click+hold was triggered
		bool _ignoreUP;
		// when held, whether to wait for the up event
		bool _waitForUP;
		// whether or not the hold event happened already
		bool _longPressHappened;
		// whether or not the long hold event happened already
		bool _vLongPressHappened;

	public:
		FourWayButton();
		void Configure(int pin, int pullMode);
		void CheckBP(void);
		void (*OnClick)(int pin);
		void (*OnDblClick)(int pin);
		void (*OnLongPress)(int pin);
		void (*OnVLongPress)(int pin);

};
