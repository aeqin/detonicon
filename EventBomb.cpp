// EventBomb.cpp

// Game includes
#include "EventBomb.h"

EventBomb::EventBomb(Bomb* bomb, bool isSpawn) // Constructor
{
	// Set up variables
	this->bomb = bomb;
	this->isSpawn = isSpawn;

	// Set object type
	setType(BOMB_EVENT);
}

Bomb* EventBomb::getBomb() // Returns Bomb that spawned this event
{
	return this->bomb;
}

bool EventBomb::getIsSpawn() // Returns whether or not Bomb that spawned this event just spawned or just exploded
{
	return this->isSpawn;
}