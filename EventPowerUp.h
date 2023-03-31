// EventPowerUp.h

#pragma once

// System includes
#include <string>

// Game includes
#include "Event.h"
#include "PowerUp.h"

const std::string POWER_EVENT = "POWER_EVENT";

class EventPowerUp : public df::Event
{
private:
	PowerUp* pow; // PowerUp that spawned this event
	bool isSpawn; // True if just spawned PowerUp, false if PowerUp faded

public:
	EventPowerUp(PowerUp* pow, bool isSpawn); // Constructor
	
	// Variable getters
	PowerUp* getPower(); // Returns PowerUp that spawned this event
	bool getIsSpawn(); // Returns whether the PowerUp that spawned this event just spawned or just faded
};
