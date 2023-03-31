// EventMiniExplosion.cpp

// Game includes
#include "EventMiniExplosion.h"

EventMiniExplosion::EventMiniExplosion(MiniExplosion* mini, bool isSpawn) // Constructor
{
	// Set up variables
	this->miniExplosion = mini;
	this->isSpawn = isSpawn;

	// Set object type
	setType(MINIEXPLOSION_EVENT);
}

MiniExplosion* EventMiniExplosion::getMiniExplosion() // Returns MiniExplosion that spawned this event
{
	return this->miniExplosion;
}

bool EventMiniExplosion::getIsSpawn() // Returns whether the MiniExplosion that spawned this event just spawned or just faded
{
	return this->isSpawn;
}