// PowerUp.cpp

// System includes
#include <iostream>
#include <stdlib.h>	// for rand()

// Engine includes
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "EventPowerUp.h"
#include "ManagerMap.h"
#include "NetworkRole.h"
#include "PowerUp.h"

PowerUp::PowerUp(bool isOnServer, df::Vector pos) // Constructor, only creates Object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos);
	}
}

void PowerUp::initOnServer(df::Vector pos) // Create Object on Server
{
	// Set up variables
	// Determine power up type
	std::string spriteStr;
	int roll = rand() % 1000;
	while(1)
	{
		if(false) {}
		else if(rand() % 20 == 0)
		{
			this->type = TYPE_HP;
			spriteStr = "powerUpHP";
			break;
		}
		else if(rand() % 4 == 0)
		{
			this->type = TYPE_AMMO;
			spriteStr = "powerUpAmmo";
			break;
		}
		else if(rand() % 3 == 0)
		{
			this->type = TYPE_BOMBLENGTH;
			spriteStr = "powerUpBombLength";
			break;
		}
		else if(rand() % 25 == 0)
		{
			this->type = TYPE_NUKE;
			spriteStr = "powerUpNuke";
			break;
		}
		else if(rand() % 50 == 0)
		{
			this->type = TYPE_SPEED;
			spriteStr = "powerUpSpeed";
			break;
		}
	}
	this->powID = this->getId(); // ID of PowerUpSpawn, for purposes of identifying Bomb across Client & Server
	this->isInvincible = true; // Is PowerUp invincible?
	this->blink = true; // Blink while invincible
	this->invincibleCounter = this->maxInvincibleCounter; // How long left PowerUp is not invincible

	// Set object type
	//setType(std::string("PowerUp->" + spriteStr));
	setType(std::string("PowerUp"));

	// Spawn EventPowerUp on spawn
	EventPowerUp powSpawn(this, true);
	WM.onEvent(&powSpawn);

	// Register with events
	registerInterest(df::STEP_EVENT);

	// Set solidness
	setSolidness(df::SOFT);

	// Set sprite
	df::Sprite *p_temp_sprite = RM.getSprite(spriteStr);
	if(!p_temp_sprite)
		LM.writeLog("PowerUp::PowerUp(): Warning! Sprite '%s' not found", spriteStr.c_str());
	else
	{
		df::Sprite *p_blink_sprite = RM.getSprite("powerUpBlink");
		if(!p_blink_sprite)
			LM.writeLog("PowerUp::PowerUp(): Warning! Sprite '%s' not found", "powerUpBlink");

		this->mainSprite = p_temp_sprite;
		this->blinkSprite = p_blink_sprite;
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set position
	WM.moveObject(this, pos);
}

PowerUp::PowerUp(const int powID, const float x, const float y, const int type) // Constructor
{
	// Set up variables
	// Determine power up type
	std::string spriteStr;
	this->type = type;
	switch(this->type)
	{
	case 0:
		spriteStr = "powerUpHP";
		break;
	case 1:
		spriteStr = "powerUpAmmo";
		break;
	case 2:
		spriteStr = "powerUpBombLength";
		break;
	case 3:
		spriteStr = "powerUpSpeed";
		break;
	case 4:
		spriteStr = "powerUpNuke";
		break;
	case 5:
		break;
	}
	this->powID = powID; // ID of PowerUpSpawn, for purposes of identifying Bomb across Client & Server
	this->isInvincible = true; // Is PowerUp invincible?
	this->blink = true; // Blink while invincible
	this->invincibleCounter = this->maxInvincibleCounter; // How long left PowerUp is not invincible

	// Set object type
	setType(std::string("PowerUp"));
	//setType(std::string("PowerUp->" + spriteStr));

	// Register with events
	registerInterest(df::STEP_EVENT);

	// Set solidness
	setSolidness(df::SOFT);

	// Set sprite
	df::Sprite *p_temp_sprite = RM.getSprite(spriteStr);
	if(!p_temp_sprite)
		LM.writeLog("PowerUp::PowerUp(): Warning! Sprite '%s' not found", spriteStr.c_str());
	else
	{
		df::Sprite *p_blink_sprite = RM.getSprite("powerUpBlink");
		if(!p_blink_sprite)
			LM.writeLog("PowerUp::PowerUp(): Warning! Sprite '%s' not found", "powerUpBlink");

		this->mainSprite = p_temp_sprite;
		this->blinkSprite = p_blink_sprite;
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set position
	WM.moveObject(this, df::Vector(x, y));

	// Spawn EventPowerUp on spawn
	EventPowerUp powSpawn(this, true);
	WM.onEvent(&powSpawn);
}

int PowerUp::getPowerType() // Returns the type of PowerUp
{
	return this->type;
}

int PowerUp::getPowID() // Returns ID of PowerUp, for purposes of identifying Bomb across Client & Server
{
	return this->powID;
}

bool PowerUp::getIsInvincible() // Returns whether or not the PowerUp is currently invincible
{
	return this->isInvincible;
}

void PowerUp::setPowID(const int powID) // Sets ID of PowerUp, for purposes of identifying Bomb across Client & Server
{
	this->powID = powID;
}

int PowerUp::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		if(MM.isInBounds(getPosition()) == false) WM.markForDelete(this); // Delete if out of arena
		
		this->invincibleCounter--;
		if(this->invincibleCounter < 0)
		{
			this->invincibleCounter = 0;
			this->isInvincible = false;
			this->blink = false;

			setVisible(true);
			setSpriteSlowdown(4);
		}
		else
		{
			if(this->blink == true)
			{
				setVisible(false);
				this->blink = false;
			}
			else if(invincibleCounter != 0 && invincibleCounter % 5 == 0)
			{
				setVisible(true);
				this->blink = true;
			}
		}
		return 1;
	}
	return 0;
}

PowerUp::~PowerUp() // Destructor
{
	// Spawn EventPowerUp on death
	EventPowerUp powDie(this, false);
	WM.onEvent(&powDie);
}

