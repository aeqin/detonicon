// Bomb.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "Object.h"

// Game includes
#include "Person.h"

class Bomb : public df::Object
{
private:
	Person* owner; // Which Person placed this Bomb
	int bombID; // ID of Bomb, for purposes of identifying Bomb across Client & Server
	int maxFuseTime; // How long Bomb lasts before exploding
	int fuseTime; // How long until Bomb explodes
	int stayTime; // How long Explosion effect lasts
	int bombPower; // Power of Bomb (length of Explosion arms)
	bool shortFuse; // Whether or not sprite is showing the long fuse (not about to explode)

	df::Vector kickedDir; // Move in this direction after kick
	int kickedPower; // How long (far) to move in kickedDir direction

	// Explosion "arms" that originate from center
	std::string armTop;
	std::string armLeft;
	std::string armRight;
	std::string armBot;

public:
	Bomb(bool isOnServer, df::Vector pos, Person* owner, const int fuseTime, const int stayTime, const int bombPower); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos, Person* owner, const int fuseTime, const int stayTime, const int bombPower); // Create Object on Server

	Bomb(df::Vector pos, Person* owner, const int bombID, const int fuseTime, const int stayTime, const int bombPower); // Constructor

	// Variable getters
	int getBombID(); // Returns ID of Bomb, for purposes of identifying Bomb across Client & Server
	int getBombPower(); // Returns the power of the Bomb (length of Explosion "arms")
	df::Vector getCenterPos(); // Returns position of bomb (not the middle of the sprite including the fuse)

	// Variable setters
	void setBombID(const int bombID); // Sets ID of Bomb, for purposes of identifying Bomb across Client & Server

	int eventHandler(const df::Event *p_e); // Handles events
	void tick(); // Called every frame, counts down time until bomb explodes
	bool isThereSolidAt(df::Vector pos); // Checks if there is a solid object at position
	void getKicked(const float prevX, const float prevY, const int power); // Calculates the direction & distance to travel the Bomb is kicked in
	void moveByKick(); // Moves the Bomb in the direction is was kicked in
	void setOffBomb(); // Explodes the Bomb

	// Custom networking
	std::string serialize(bool all = false); // Custom serialize for variables
	int deserialize(std::string str); // Custom deserialize for variables

	~Bomb(); // Destructor, spawns Explosion
};