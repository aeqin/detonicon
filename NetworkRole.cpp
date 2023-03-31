// NetworkRole.cpp

// Game includes
#include "NetworkRole.h"

NetworkRole::NetworkRole() // Constructor
{
	this->client = NULL;
	this->server = NULL;
}

static NetworkRole* p_networkRole = nullptr;
NetworkRole& NetworkRole::getInstance() // Get the one and only instance of the NetworkRole
{
	if(p_networkRole == nullptr)
	{
		p_networkRole = new NetworkRole();
	}

	return *p_networkRole;
}

Client* NetworkRole::getClient() // Gets client of game
{
	return this->client;
}

Server* NetworkRole::getServer() // Gets server of game
{
	return this->server;
}

void NetworkRole::setClient(Client* client) // Sets client of game
{
	this->client = client;
}

void NetworkRole::setServer(Server* server) // Sets server of game
{
	this->server = server;
}