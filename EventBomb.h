// EventBomb.h

#pragma once

// System includes
#include <string>

// Game includes
#include "Bomb.h"
#include "Event.h"

const std::string BOMB_EVENT = "BOMB_EVENT";

class EventBomb : public df::Event
{
private:
	Bomb* bomb; // Bomb that spawned this event
	bool isSpawn; // True if just spawned bomb, false if bomb exploding

public:
	EventBomb(Bomb* bomb, bool isSpawn); // Constructor
	
	// Variable getters
	Bomb* getBomb(); // Returns Bomb that spawned this event
	bool getIsSpawn(); // Returns whether or not Bomb that spawned this event just spawned or just exploded
};