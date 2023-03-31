// ClientInfo.cpp

// Engine includes
#include <iostream>

// Game includes
#include "ClientInfo.h"

// Constructors
ClientInfo::ClientInfo(const bool isPlayer, const int objectID)
{
	// Set up variables
	this->isPlayer = isPlayer;
	this->objectID = objectID;
}
ClientInfo::ClientInfo(const bool isPlayer)
{
	// Set up variables
	this->isPlayer = isPlayer;
	this->objectID = -1;
}
ClientInfo::ClientInfo(const bool isPlayer, const int braveVal, const int smartVal)
{
	// Set up variables
	this->isPlayer = isPlayer;
	this->objectID = -1;
	this->braveVal = braveVal;
	this->smartVal = smartVal;
}
ClientInfo::ClientInfo(const int objectID)
{
	// Set up variables
	this->isPlayer = false;
	this->objectID = objectID;
}

bool ClientInfo::getIsPlayer() // Gets isPlayer
{
	return this->isPlayer;
}

int ClientInfo::getObjectID() // Gets Object ID
{
	return this->objectID;
}

int ClientInfo::getBraveVal() // Gets brave value of bot
{
	return this->braveVal;
}

int ClientInfo::getSmartVal() // Gets smart value of bot
{
	return this->smartVal;
}

int ClientInfo::getLagInMS() // Gets lag of Client
{
	return this->lagInMS;
}

bool ClientInfo::getReadyToShutdown() // Gets whether Client is ready to shutdown
{
	return this->readyToShutdown;
}

void ClientInfo::setIsPlayer(const bool isPlayer) // Sets isPlayer
{
	this->isPlayer = isPlayer;
}

void ClientInfo::setObjectID(const int objectID) // Sets Object ID
{
	this->objectID = objectID;
}

void ClientInfo::setBraveVal(const int braveVal) // Sets brave value of bot
{
	this->braveVal = braveVal;
}

void ClientInfo::setSmartVal(const int smartVal) // Sets smart value of bot
{
	this->smartVal = smartVal;
}

void ClientInfo::setLagInMS(const int lagInMS) // Sets lag of Client
{
	this->lagInMS = lagInMS;
}

void ClientInfo::setReadyToShutdown(const bool shutdown) // Sets whether Client is ready to shutdown
{
	this->readyToShutdown = shutdown;
}