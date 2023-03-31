// ManagerPowerUp.h

#pragma once

// System includes
#include <string>
#include <vector>

// Game includes
#include "PowerUp.h"

#define MPow ManagerPowerUp::getInstance() // Two-letter acronym for easier access to manager

class ManagerPowerUp : df::Object
{
private:
	std::vector<PowerUp*> powers; // Vector of all PowerUps in game
	std::vector<int> powerObjectIds; // Vector of all object ids of PowerUps in game

	ManagerPowerUp(); // Constructor, private for singleton

public:
	static ManagerPowerUp &getInstance(); // Get the one and only instance of the ManagerPerson

	// Variable getters
	int getPowerCount(); // Get number of powers in game
	std::vector<PowerUp*> getPowers(); // Get vector of all powers in game

	int eventHandler(const df::Event *p_e); // Handles events

	// Custom Networking
	void addPower(int powID); // Add PowerUp to ManagerPowerUp on Client
	void removePower(int powID); // Remove PowerUp from ManagerPowerUp on Client
};