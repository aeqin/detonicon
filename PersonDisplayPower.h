// PersonDisplayPower.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "Event.h"
#include "ViewObject.h"

#define POWER_STRING "Buffs: " // String to be displayed

class PersonDisplayPower : public df::ViewObject
{
private:
	std::string posBuffer; // String of constant max char length to display correctly regardless of size
	int bombPower; // Bomb power to be displayed
	int numBombs; // Number of bombs to be displayed
	int moveSpeed; // Movespeed to be displayed 

public:
	PersonDisplayPower(bool isOnServer, df::Vector pos, const int maxChars); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos, const int maxChars); // Create Object on Server

	PersonDisplayPower(const int x, const int y, const int maxChars); // Constructor
	void updatePowers(Person* person); // Updates display string to reflect new PowerUps
	void updatePowers(const int bombPwr, const int numBombs, const int moveSpeed); // Updates display string to reflect new PowerUps

	// Custom Networking
	std::string serialize(bool all = false); // Custom serialize for variables
	int deserialize(std::string str); // Custom deserialize for variables
};