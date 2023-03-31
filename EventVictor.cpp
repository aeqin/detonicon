// EventVictor.cpp

// Game includes
#include "EventVictor.h"

EventVictor::EventVictor(Person* victor) // Constructor
{
	// Set up variables
	this->victor = victor;

	// Set object type
	setType(VICTOR_EVENT);
}

Person* EventVictor::getVictor() // Returns Person that spawned this event
{
	return this->victor;
}