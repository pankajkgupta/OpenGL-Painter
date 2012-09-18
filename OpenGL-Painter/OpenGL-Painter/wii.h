#include "wiimote.h"

// configs:
#define USE_BEEPS_AND_DELAYS			// undefine to test library works without them
#define LOOK_FOR_ADDITIONAL_WIIMOTES	// tries to connect any extra wiimotes

// print-related defines (so we don't clutter the .cpp)
#define _SETCOL(flags) SetConsoleTextAttribute(console, flags)

#define WHITE			_SETCOL(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)
#define BRIGHT_WHITE	_SETCOL(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY)
#define RED				_SETCOL(FOREGROUND_RED)
#define BRIGHT_RED		_SETCOL(FOREGROUND_RED|FOREGROUND_INTENSITY)
#define BLUE			_SETCOL(FOREGROUND_BLUE)
#define BRIGHT_BLUE		_SETCOL(FOREGROUND_BLUE|FOREGROUND_INTENSITY)
#define GREEN			_SETCOL(FOREGROUND_GREEN)
#define BRIGHT_GREEN	_SETCOL(FOREGROUND_GREEN|FOREGROUND_INTENSITY)
#define YELLOW			_SETCOL(FOREGROUND_RED|FOREGROUND_GREEN)
#define BRIGHT_YELLOW	_SETCOL(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY)
#define CYAN			_SETCOL(FOREGROUND_BLUE|FOREGROUND_GREEN)
#define BRIGHT_CYAN		_SETCOL(FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY)
#define PURPLE			_SETCOL(FOREGROUND_RED|FOREGROUND_BLUE)
#define BRIGHT_PURPLE	_SETCOL(FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY)

#define BLANK_LINE \
 _T("                                                                             \n")

// ------------------------------------------------------------------------------------
//  state-change callback example (we use polling for everything else):
// ------------------------------------------------------------------------------------
void on_state_change (wiimote			  &remote,
					  state_change_flags  changed,
					  const wiimote_state &new_state)
	{
	// we use this callback to set report types etc. to respond to key events
	//  (like the wiimote connecting or extensions (dis)connecting).
	
	// NOTE: don't access the public state from the 'remote' object here, as it will
	//		  be out-of-date (it's only updated via RefreshState() calls, and these
	//		  are reserved for the main application so it can be sure the values
	//		  stay consistent between calls).  Instead query 'new_state' only.

	// the wiimote just connected
	if(changed & CONNECTED)
		{
		// ask the wiimote to report everything (using the 'non-continous updates'
		//  default mode - updates will be frequent anyway due to the acceleration/IR
		//  values changing):

		// note1: you don't need to set a report type for Balance Boards - the
		//		   library does it automatically.
		
		// note2: for wiimotes, the report mode that includes the extension data
		//		   unfortunately only reports the 'BASIC' IR info (ie. no dot sizes),
		//		   so let's choose the best mode based on the extension status:
		if(new_state.ExtensionType != wiimote::BALANCE_BOARD)
			{
			if(new_state.bExtension)
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT); // no IR dots
			else
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);		//    IR dots
			}
		}
	// a MotionPlus was detected
	if(changed & MOTIONPLUS_DETECTED)
		{
		// enable it if there isn't a normal extension plugged into it
		// (MotionPlus devices don't report like normal extensions until
		//  enabled - and then, other extensions attached to it will no longer be
		//  reported (so disable the M+ when you want to access them again).
		if(remote.ExtensionType == wiimote_state::NONE) {
			bool res = remote.EnableMotionPlus();
			_ASSERT(res);
			}
		}
	// an extension is connected to the MotionPlus
	else if(changed & MOTIONPLUS_EXTENSION_CONNECTED)
		{
		// We can't read it if the MotionPlus is currently enabled, so disable it:
		if(remote.MotionPlusEnabled())
			remote.DisableMotionPlus();
		}
	// an extension disconnected from the MotionPlus
	else if(changed & MOTIONPLUS_EXTENSION_DISCONNECTED)
		{
		// enable the MotionPlus data again:
		if(remote.MotionPlusConnected())
			remote.EnableMotionPlus();
		}
	// another extension was just connected:
	else if(changed & EXTENSION_CONNECTED)
		{
#ifdef USE_BEEPS_AND_DELAYS
		Beep(1000, 200);
#endif
		// switch to a report mode that includes the extension data (we will
		//  loose the IR dot sizes)
		// note: there is no need to set report types for a Balance Board.
		if(!remote.IsBalanceBoard())
			remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
		}
	// extension was just disconnected:
	else if(changed & EXTENSION_DISCONNECTED)
		{
#ifdef USE_BEEPS_AND_DELAYS
		Beep(200, 300);
#endif
		// use a non-extension report mode (this gives us back the IR dot sizes)
		remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);
		}
	}

