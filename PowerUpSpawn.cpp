// PowerUpSpawn.cpp

// System includes
#include <stdlib.h>	// for rand()

// Engine includes
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "GameStart.h"
#include "NetworkRole.h"
#include "PowerUp.h"
#include "Wall.h"

#include "PowerUpSpawn.h"

// PowerUp definitions
#define TYPE_HP 0
#define TYPE_AMMO 1
#define TYPE_BOMBLENGTH 2
#define TYPE_SPEED 3
#define TYPE_NUKE 4

PowerUpSpawn::PowerUpSpawn(const float x, const float y) // Constructor, only Server calls this
{
	// Set object type
	setType("PwrUpSpawn");

	// Set up variables
	int roll = rand() % 1000;
	while(1)
	{
		if(false) {}
		else if(rand() % 20 == 0)
		{
			this->type = TYPE_HP;
			break;
		}
		else if(rand() % 4 == 0)
		{
			this->type = TYPE_AMMO;
			break;
		}
		else if(rand() % 3 == 0)
		{
			this->type = TYPE_BOMBLENGTH;
			break;
		}
		else if(rand() % 25 == 0)
		{
			this->type = TYPE_NUKE;
			break;
		}
		else if(rand() % 50 == 0)
		{
			this->type = TYPE_SPEED;
			break;
		}
	}
	this->powID = this->getId();

	// Register with events
	registerInterest(df::STEP_EVENT);

	// Set solidness
	setSolidness(df::SPECTRAL);

	// Set sprite
	df::Sprite *p_temp_sprite = RM.getSprite("powerUpSpawn");
	if(!p_temp_sprite)
		LM.writeLog("PowerUp::PowerUp(): Warning! Sprite '%s' not found", "powerUpSpawn");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(8);
		setTransparency();
	}

	// Set position
	WM.moveObject(this, df::Vector(x, y));

	// Send message to spawn on Client
	if(NR.getServer())
	{
		std::string msg = "SPAWN POWERSPAWN|"; // 'SPAWN POWERSPAWN'|powID|xPosition|yPosition|typeOfPowerUp
		msg += std::to_string(this->powID);
		msg += "|";
		msg += std::to_string((int) x);
		msg += "|";
		msg += std::to_string((int) y);
		msg += "|";
		msg += std::to_string(this->type);

		const char* msgToSend = msg.c_str();
		LM.writeLog("ManagerMap::spawnPowerUp(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Clients to spawn PowerUpSpawn
	}
}

PowerUpSpawn::PowerUpSpawn(const int powID, const float x, const float y, const int type) // Constructor
{
	// Set object type
	setType("PwrUpSpawn");

	// Set up variables
	this->type = type; // Type of PowerUp that comes out of the PowerUpSpawn
	this->powID = powID; // ID of PowerUpSpawn, for purposes of identifying Bomb across Client & Server

	// Register with events
	registerInterest(df::STEP_EVENT);

	// Set solidness
	setSolidness(df::SPECTRAL);

	// Set sprite
	df::Sprite *p_temp_sprite = RM.getSprite("powerUpSpawn");
	if(!p_temp_sprite)
		LM.writeLog("PowerUp::PowerUp(): Warning! Sprite '%s' not found", "powerUpSpawn");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(8);
		setTransparency();
	}

	// Set position
	WM.moveObject(this, df::Vector(x, y));
}

int PowerUpSpawn::getPowID() // Returns ID of PowerUp, for purposes of identifying Bomb across Client & Server
{
	return this->powID;
}

void PowerUpSpawn::setPowID(const int powID) // Sets ID of PowerUp, for purposes of identifying Bomb across Client & Server
{
	this->powID = powID;
}

int PowerUpSpawn::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		if(getSpriteIndex() == getSprite()->getFrameCount() - 1)
		{
			WM.markForDelete(this);
		}
		return 1;
	}
	return 0;
}

PowerUpSpawn::~PowerUpSpawn() // Destructor, spawns PowerUp
{
	if(GM.getGameOver() || GS.getIsGameOver() == true) return;

	new PowerUp(this->getPowID(), this->getPosition().getX(), this->getPosition().getY(), this->type);
}

