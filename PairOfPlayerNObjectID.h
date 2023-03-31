// PairOfPlayerNObjectID.h

#pragma once

// Engine includes
#include "Object.h"

class PairOfPlayerNObjectID : public df::Object
{
private:
	bool isPlayer; // Is it a Player? Or a bot?
	int objectID = -1; // What id is the Player/Bot?
	int braveVal;
	int smartVal;

public:
	// Constructors
	PairOfPlayerNObjectID(const bool isPlayer, const int objectID);
	PairOfPlayerNObjectID(const bool isPlayer);
	PairOfPlayerNObjectID(const bool isPlayer, const int braveVal, const int smartVal);
	PairOfPlayerNObjectID(const int objectID);

	// Variable getters
	bool getIsPlayer(); // Gets isPlayer
	int getObjectID(); // Gets Object ID
	int getBraveVal(); // Gets brave value of bot
	int getSmartVal(); // Gets smart value of bot

	// Variable setters
	void setIsPlayer(const bool isPlayer); // Sets isPlayer
	void setObjectID(const int objectID); // Sets Object ID
	void setBraveVal(const int braveVal); // Sets brave value of bot
	void setSmartVal(const int smartVal); // Sets smart value of bot

};