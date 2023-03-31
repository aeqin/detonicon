// EventPowerUp.cpp

// Game includes
#include "EventPowerUp.h"

EventPowerUp::EventPowerUp(PowerUp* pow, bool isSpawn) // Constructor
{
	// Set up variables
	this->pow = pow;
	this->isSpawn = isSpawn;

	// Set object type
	setType(POWER_EVENT);
}

PowerUp* EventPowerUp::getPower() // Returns PowerUp that spawned this event
{
	return this->pow;
}

bool EventPowerUp::getIsSpawn() // Returns whether the PowerUp that spawned this event just spawned or just faded
{
	return this->isSpawn;
}