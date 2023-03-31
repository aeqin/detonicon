// EventVictor.h

#pragma once

// System includes
#include <string>

// Game includes
#include "Event.h"
#include "Person.h"

const std::string VICTOR_EVENT = "VICTOR_EVENT";

class EventVictor : public df::Event
{
private:
	Person* victor; // Returns Person that spawned this event

public:
	EventVictor(Person* victor); // Constructor

	// Variable getters
	Person* getVictor(); // Returns Person that spawned this event
};