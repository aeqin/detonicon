// NetworkRole.h

#pragma once

// Game includes
#include "Client.h"
#include "Server.h"

#define NR NetworkRole::getInstance() // Two-letter acronym for easier access to manager

class NetworkRole
{
private:
	NetworkRole(); // Constructor

	Server *server; // Is the game a server?
	Client *client; // Is the game a client?

public:
	static NetworkRole &getInstance(); // Get the one and only instance of the ManagerServer

	// Variables getters
	Client *getClient(); // Gets client of game
	Server *getServer(); // Gets server of game

	// Variable setters
	void setClient(Client *p_client); // Sets client of game
	void setServer(Server *p_server); // Sets server of game
};
