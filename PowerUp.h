// PowerUp.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "Object.h"

// PowerUp definitions
#define TYPE_HP 0
#define TYPE_AMMO 1
#define TYPE_BOMBLENGTH 2
#define TYPE_SPEED 3
#define TYPE_NUKE 4

class PowerUp : public df::Object
{
private:
	df::Sprite* mainSprite = nullptr; // Sprite of PowerUp
	df::Sprite* blinkSprite = nullptr; // Blank sprite to simulate blinking

	int type; // Type of power up
	int powID; // ID of PowerUpSpawn, for purposes of identifying Bomb across Client & Server
	bool isInvincible = true; // Is PowerUp invincible? 
	int invincibleCounter; // How long left PowerUp is not invincible
	int maxInvincibleCounter = 100; // Maximum length of invincibility
	bool blink; // Blink while invincible

public:
	PowerUp(bool isOnServer, df::Vector pos); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos); // Create Object on Server

	PowerUp(const int powID, const float x, const float y, const int type); // Constructor

	// Variable getters
	int getPowerType(); // Returns the type of PowerUp
	int getPowID(); // Returns ID of PowerUp, for purposes of identifying Bomb across Client & Server
	bool getIsInvincible(); // Returns whether or not the PowerUp is currently invincible

	// Variable setters
	void setPowID(const int powID); // Sets ID of PowerUp, for purposes of identifying Bomb across Client & Server

	int eventHandler(const df::Event *p_e); // Handles events

	~PowerUp(); // Destructor
};