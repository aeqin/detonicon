// Client.h

#pragma once

// System includes
#include <string>

// Engine includes
//#include "NetworkNode.h"
#include "NetworkLagSimulator.h"

class Client : public NetworkLagSimulator
{

private:
	bool isPlayer; // true if this Client player controlled else false if a bot
	int braveVal = -1; // Used to specify Bot brave value, -1 if Client controls player
	int smartVal = -1; // Used to specify Bot smart value, -1 if Client controls player
	int personObjectID; // Object ID of Client controlled Person
	int clientSocketID = -1; // Socket this Client was given by Server, unique across Clients
	std::string timeStamp; // TimeStamp
	time_point<Clock> lifeStart; // Time when spawned
	time_point<Clock> lifeDead; // Time when dead, used to calculate amount of time survived

	// Stats
	long matchID = 0; // ID of the game, from the Server taking the time
	long gameLengthMS = 0; // Length of game in milliseconds
	int idOfMap = 0; // Number of map the game was played in
	int numClients = 0; // Number of Clients in game
	long timeSurvivedMS = 0; // Milliseconds surviving in game
	int timesHitByExpl = 0; // Number of times this Client was hit by an explosion
	int timesBumpedIntoWall = 0; // Number of times this Client bumped into a wall
	int numPickedUpPwrs = 0; // Number of PowerUps this Client picked up
	int numPlacedBombs = 0; // Number of Bombs this Client placed
	int numSpacesMoved = 0; // Number of spaces moved
	int numMoveCmdsSent = 0; // Number of movement commands sent to Server
	int numTimesRebounded = 0; // How many times the Server forced the Client to bounceback (Client attempted to move to invalid position)
	int numSpacesRebounded = 0; // How many spaces in total did the Server make the Client rebound?
	bool isVictor = false; // Did this Client win the game?
	bool isUsingLagCompensation = false; // true if Client is using lag compensation technique (player client movement prediction)

public:
	Client(std::string serverName, const int lagInMS, const bool isUsingLagCompensation, const bool isPlayer, const int braveVal, const int smartVal); // Constructor

	// Variable getters
	int getPersonObjectID(); // Gets Object ID of Person controlled by Client
	int getClientSocketID(); // Gets socket of Client given by Server
	bool getIsUsingLagCompensation(); // Returns whether or not the Client is using lag compensation (client player side movement prediction)

	void spawnMap(const int mapID, const int firstWallID); // Calls ManagerMap to draw the map

	int eventHandler(const df::Event *p_e); // Handles events

	// Custom networking
	void parseCustomMsg(const void* msg); // Parses custom message from Server and does something on Client
	int handleConnect(const df::EventNetwork *p_e); // Connects Client to server
	int handleData(const df::EventNetwork *p_e); // Handles received messages
	df::Object *createObject(std::string objectType); // Creates specified game object
	void incrNumSpacesMoved(int spaces = 1); // Increments the numSpacesMoved statistic
	void incrNumTimesRebounded(); // Increments the numTimesRebounded statistic
	void incrNumSpacesRebounded(int spaces = 1); // Increments the numSpacesRebounded statistic
	void incrNumMoveCmdsSent(int moveCmd = 1); // Increments the numMoveCmdsSent statistic
	void writeResults(); // Writes the results of the game to a file

	~Client(); // Destructor
};
