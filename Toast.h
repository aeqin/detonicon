// Toast.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "Event.h"
#include "ViewObject.h"

class Toast : public df::ViewObject
{
private:
	int timeToLive; // How long left to live before fading away

public:
	enum ToastStr {OW = 0, HP_UP, HP_ALREADY_MAX, AMMO_UP, AMMO_MAX, BOMBPWR_UP, SPD_UP, SPD_ALREADY_MAX, NUKE, NONE}; // Enum of Toast display options

	Toast(const int x, const int y, ToastStr str); // Constructor

	int eventHandler(const df::Event *p_e); // Handles events
	void setViewAs(ToastStr str); // Sets ViewObject string as an enum option
	int getIntFromToast(ToastStr str); // Returns int value of ToastStr
	static ToastStr getToastFromInt(const int val); // Returns ToastStr from int
};
