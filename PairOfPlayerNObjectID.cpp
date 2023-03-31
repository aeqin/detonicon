// PairOfPlayerNObjectID.cpp

// Engine includes
#include <iostream>

// Game includes
#include "PairOfPlayerNObjectID.h"

// Constructors
PairOfPlayerNObjectID::PairOfPlayerNObjectID(const bool isPlayer, const int objectID)
{
	// Set up variables
	this->isPlayer = isPlayer;
	this->objectID = objectID;
}
PairOfPlayerNObjectID::PairOfPlayerNObjectID(const bool isPlayer)
{
	// Set up variables
	this->isPlayer = isPlayer;
	this->objectID = -1;
}
PairOfPlayerNObjectID::PairOfPlayerNObjectID(const bool isPlayer, const int braveVal, const int smartVal)
{
	// Set up variables
	this->isPlayer = isPlayer;
	this->objectID = -1;
	this->braveVal = braveVal;
	this->smartVal = smartVal;
}
PairOfPlayerNObjectID::PairOfPlayerNObjectID(const int objectID)
{
	// Set up variables
	this->isPlayer = false;
	this->objectID = objectID;
}

bool PairOfPlayerNObjectID::getIsPlayer() // Gets isPlayer
{
	return this->isPlayer;
}

int PairOfPlayerNObjectID::getObjectID() // Gets Object ID
{
	return this->objectID;
}

int PairOfPlayerNObjectID::getBraveVal() // Gets brave value of bot
{
	return this->braveVal;
}

int PairOfPlayerNObjectID::getSmartVal() // Gets smart value of bot
{
	return this->smartVal;
}

void PairOfPlayerNObjectID::setIsPlayer(const bool isPlayer) // Sets isPlayer
{
	this->isPlayer = isPlayer;
}

void PairOfPlayerNObjectID::setObjectID(const int objectID) // Sets Object ID
{
	this->objectID = objectID;
}

void PairOfPlayerNObjectID::setBraveVal(const int braveVal) // Sets brave value of bot
{
	this->braveVal = braveVal;
}

void PairOfPlayerNObjectID::setSmartVal(const int smartVal) // Sets smart value of bot
{
	this->smartVal = smartVal;
}