// Server.h

#pragma once

// System includes
#include <map>
#include <string>
#include <time.h>

// Engine includes
#include "NetworkLagSimulator.h"

// Game includes
#include "ClientInfo.h"

class Server : public NetworkLagSimulator
{
private:
	bool spawnOnce = false;
	int mapID; // Map id
	int MAX_CONNECTIONS = 5; // Maximum number of Clients
	int numClientsForAutoStart; // Number of Clients needed to auto start the game (no need to press P at title screen) once all Clients are connected, or -1 if disabled
	int numMaps; // Number of spawnable maps
	int mostRecentClientSocket = -1; // Socket of most recent Client message Server received
	int numAcceptedClients = 0; // # Clients that connected successfully to Server
	int numStartedClients = 0; // # Clients that received 'START GAME' message and replied to Server
	int numShutdownClients = 0; // # Clients that Shutdown
	std::map<int, ClientInfo*> map_Socket_to_PlayerID; // Maps socket ID of a Client to an object containing important information on Client (personID, lagMS, etc.)
	clock_t start;
	clock_t end;

	void syncObjects(); // Synchronizes Objects and sends data to connected Clients

public:
	Server(const int mapID, const int numClientsForAutoStart = -1); // Constructor

	// Variable getters
	std::map<int, ClientInfo*> getClientPersons(); // Returns pair of Client socket ID, and corresponding Client information
	int getSocketIDFromPersonObjectID(const int objectId); // Returns socket of Client that controls Person with Object ID
	int getNumAcceptedClients(); // Returns # Clients that connected successfully to Server
	int getNumStartedClients(); // Returns # Clients that received 'START GAME' message and replied to Server

	void spawnMap(); // Calls ManagerMap to draw the map
	void startTimer(); // Start game timer
	void endTimer(); // End game timer
	void sendMatchID(); // Sends ID of game match to Clients to be saved

	int eventHandler(const df::Event *p_e); // Handles events

	// Custom networking
	void parseCustomMsg(const void* msg); // Parses custom message from Client and does something on Server
	int handleAccept(const df::EventNetwork *p_e); // Accepts Client request to connect to Server
	int handleData(const df::EventNetwork *p_e); // Accepts Client request to connect to Server
};
