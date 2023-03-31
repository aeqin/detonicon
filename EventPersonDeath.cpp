// EventPersonDeath.cpp

// Game includes
#include "EventPersonDeath.h"

EventPersonDeath::EventPersonDeath(std::string name, const int id) // Constructor
{
	// Set up variables
	this->name = name;
	this->id = id;

	// Set object type
	setType(PERSONDEATH_EVENT);
}

std::string EventPersonDeath::getName() // Returns name of Person that spawned this event
{
	return this->name;
}

int EventPersonDeath::getID() // Returns ID of Person that spawned this event
{
	return this->id;
}