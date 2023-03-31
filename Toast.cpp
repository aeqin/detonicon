// Toast.cpp

// Engine includes
#include "DisplayManager.h"
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "Toast.h"

Toast::Toast(const int x, const int y, ToastStr str) // Constructor
{
	// Set up variables
	this->timeToLive = 15;
	setViewAs(str);

	// Set object type
	setType("Toast");

	// Register with events
	registerInterest(df::STEP_EVENT); // To update when to fade away

	// Set position
	setPosition(df::Vector((float) x, (float) y));
}

int Toast::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		this->timeToLive--;
		if(this->timeToLive <= 0)
		{
			WM.markForDelete(this);
		}
		return 1;
	}
	return 0;
}

void Toast::setViewAs(ToastStr str) // Sets ViewObject string as an enum option
{
	switch(str)
	{
	case OW:
		setColor(df::RED);
		setViewString("OW");
		break;
	case HP_UP:
		setColor(df::WHITE);
		setViewString("HP UP");
		break;
	case HP_ALREADY_MAX:
		setColor(df::RED);
		setViewString("HP ALREADY MAX");
		break;
	case AMMO_UP:
		setColor(df::WHITE);
		setViewString("MAX AMMO UP");
		break;
	case AMMO_MAX:
		setColor(df::RED);
		setViewString("ALREADY MAX AMMO");
		break;
	case BOMBPWR_UP:
		setColor(df::WHITE);
		setViewString("BOMB POWER UP");
		break;
	case SPD_UP:
		setColor(df::WHITE);
		setViewString("SPEED UP");
		break;
	case SPD_ALREADY_MAX:
		setColor(df::RED);
		setViewString("SPEED ALREADY MAX");
		break;
	case NUKE:
		setColor(df::RED);
		setViewString("NUKE");
		break;
	default:
		setColor(df::WHITE);
	}

	setBorder(false);
	setDrawValue(false);
}

int Toast::getIntFromToast(ToastStr str)
{
	switch(str)
	{
	case OW:
		return 0;
	case HP_UP:
		return 1;
	case HP_ALREADY_MAX:
		return 2;
	case AMMO_UP:
		return 3;
	case AMMO_MAX:
		return 4;
	case BOMBPWR_UP:
		return 5;
	case SPD_UP:
		return 6;
	case SPD_ALREADY_MAX:
		return 7;
	case NUKE:
		return 8;
	case NONE:
		return 9;
	}

	return 0;
}

Toast::ToastStr Toast::getToastFromInt(const int val)
{
	switch(val)
	{
	case 0:
		return Toast::OW;
	case 1:
		return Toast::HP_UP;
	case 2:
		return Toast::HP_ALREADY_MAX;
	case 3:
		return Toast::AMMO_UP;
	case 4:
		return Toast::AMMO_MAX;
	case 5:
		return Toast::BOMBPWR_UP;
	case 6:
		return Toast::SPD_UP;
	case 7:
		return Toast::SPD_ALREADY_MAX;
	case 8:
		return Toast::NUKE;
	}

	return Toast::NONE;
}

