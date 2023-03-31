// PowerUpSpawn.h

#pragma once

// System
#include <string>

class PowerUpSpawn : public df::Object
{
private:
	int type; // Type of PowerUp that comes out of the PowerUpSpawn
	int powID; // ID of PowerUpSpawn, for purposes of identifying Bomb across Client & Server

public:
	PowerUpSpawn(const float x, const float y); // Constructor, only Server calls this
	PowerUpSpawn(const int powID, const float x, const float y, const int type); // Constructor

	// Variable getters
	int getPowID(); // Returns ID of PowerUp, for purposes of identifying Bomb across Client & Server

	// Variable setters
	void setPowID(const int powID); // Sets ID of PowerUp, for purposes of identifying Bomb across Client & Server

	int eventHandler(const df::Event *p_e); // Handles events

	~PowerUpSpawn(); // Destructor, spawns PowerUp
};