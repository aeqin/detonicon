// ClientInfo.h

#pragma once

// Engine includes
#include "Object.h"

class ClientInfo : public df::Object
{
private:
	bool isPlayer; // Is it a Player? Or a bot?
	int objectID = -1; // What id is the Player/Bot?
	int braveVal; // Brave Value of Bot
	int smartVal; // Smart Value of Bot
	int lagInMS = 0; // The amount of lag the Client has
	bool readyToShutdown = false; // This Client is ready to shutdown

public:
	// Constructors
	ClientInfo(const bool isPlayer, const int objectID);
	ClientInfo(const bool isPlayer);
	ClientInfo(const bool isPlayer, const int braveVal, const int smartVal);
	ClientInfo(const int objectID);

	// Variable getters
	bool getIsPlayer(); // Gets isPlayer
	int getObjectID(); // Gets Object ID
	int getBraveVal(); // Gets brave value of bot
	int getSmartVal(); // Gets smart value of bot
	int getLagInMS(); // Gets lag of Client
	bool getReadyToShutdown(); // Gets whether Client is ready to shutdown

	// Variable setters
	void setIsPlayer(const bool isPlayer); // Sets isPlayer
	void setObjectID(const int objectID); // Sets Object ID
	void setBraveVal(const int braveVal); // Sets brave value of bot
	void setSmartVal(const int smartVal); // Sets smart value of bot
	void setLagInMS(const int lagInMS); // Sets lag of Client
	void setReadyToShutdown(const bool shutdown); // Sets whether Client is ready to shutdown

};