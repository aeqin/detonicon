// PersonDisplayBomb.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "ViewObject.h"
#include "Event.h"

#define BOMB_STRING "Bombs: " // String to be displayed

class PersonDisplayBomb : public df::ViewObject
{
private:
	std::string posBuffer; // String of constant max char length to display correctly regardless of size
	int numBombs; // Number of bombs indicated
	int bombID; // Used to identify corresponding Client PersonDisplayBomb from Server's

public:
	PersonDisplayBomb(bool isOnServer, df::Vector pos, const int bombs, const int maxChars); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos, const int bombs, const int maxChars); // Create Object on Server

	PersonDisplayBomb(const int x, const int y, const int bombs, const int maxChars); // Constructor

	// Variable getters
	int getBombID(); // Returns bombID

	// Variable setters
	void setBombID(const int bombID); // Sets bombID

	void updateBombs(const int newBombs); // Updates display string to reflect new number of bombs
};