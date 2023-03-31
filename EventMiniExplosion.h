// EventMiniExplosion.h
#pragma once

// System includes
#include <string>

// Game includes
#include "Event.h"
#include "MiniExplosion.h"

const std::string MINIEXPLOSION_EVENT = "MINIEXPLOSION_EVENT";

class EventMiniExplosion : public df::Event
{
private:
	MiniExplosion* miniExplosion; // MiniExplosion that just spawned this event
	bool isSpawn; // True if just spawned MiniExplosion, false if MiniExplosion faded

public:
	EventMiniExplosion(MiniExplosion* mini, bool isSpawn); // Constructor
	
	// Variable getters
	MiniExplosion* getMiniExplosion(); // Returns MiniExplosion that spawned this event
	bool getIsSpawn(); // Returns whether the MiniExplosion that spawned this event just spawned or just faded
};
