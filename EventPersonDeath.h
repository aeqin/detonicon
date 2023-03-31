// EventPersonDeath.h

#pragma once

// System includes
#include <string>

// Game includes
#include "Event.h"

const std::string PERSONDEATH_EVENT = "PERSONDEATH_EVENT";

class EventPersonDeath : public df::Event
{
private:
	std::string name; // Name of Person that spawned this event
	int id; // ID of person that spawned this event

public:
	EventPersonDeath(std::string name, const int id); // Constructor
	
	// Variable getters
	std::string getName(); // Returns name of Person that spawned this event
	int getID(); // Returns ID of Person that spawned this event
};