// Explosion.h

#pragma once

// System includes
#include <string>

class Explosion : public df::Object
{
private:
	int stayTime; // How long MiniExplosions hangs around before fading
	int x; // X position
	int y; // Y position
	int power; // Length of "arms"

	// "Arms", a pattern of MiniExplosion that extends from center of Explosion in each cardinal direction
	std::string armTop;
	std::string armLeft;
	std::string armRight;
	std::string armBot;

	// Counters used to spawn arms in sequence
	int topCounter;
	int leftCounter;
	int rightCounter;
	int botCounter;

	bool spawnArm(const std::string direction); // Spawns arm in direction ['t' (top), 'l' (left), 'r' (right), 'b' (bot)], returns true if successful, false otherwise
	
public:
	Explosion(bool isOnServer, df::Vector pos, const int stayTime, const int power); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos, const int stayTime, const int power); // Create Object on Server

	Explosion(const int x, const int y, const int stayTime, const std::string armTop, std::string armLeft, const std::string armRight, const std::string armBot); // Constructor, custom explosion
	Explosion(const int x, const int y, const int stayTime, const int power); // Constructor, plus shaped explosion
	
	int eventHandler(const df::Event *p_e); // Handles events
	void tick(); // Spawns more of Explosion each frame
	bool isThereMiniExplosionAt(df::Vector pos); // Returns whether there is a MiniExplosion at given position

	~Explosion(); // Destructor
};