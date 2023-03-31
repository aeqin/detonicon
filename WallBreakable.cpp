// WallBreakable.cpp

// System includes
#include <stdlib.h>	// for rand()

// Engine includes
#include "EventOut.h"
#include "GameManager.h"
#include "LogManager.h"
#include "NetworkRole.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "EventVictor.h"
#include "GameStart.h"
#include "NetworkRole.h"
#include "PowerUp.h"
#include "Wall.h"
#include "WallBreakable.h"

WallBreakable::WallBreakable(bool isOnServer, df::Vector pos) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos);
	}
}

void WallBreakable::initOnServer(df::Vector pos) // Create Object on Server
{
	// Set object type
	setType("WallBreakable");

	// Set solidness
	setSolidness(df::HARD);

	// Setup WallBreakable sprite
	df::Sprite *p_temp_sprite = RM.getSprite("wallBreakable");
	if(!p_temp_sprite)
		LM.writeLog("WallBreakable::WallBreakable(): Warning! Sprite '%s' not found", "wallBreakable");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set Box
	this->setBox(df::Box());

	// Set position
	WM.moveObject(this, pos);

	LM.writeLog("WallBreakable::initOnServer() : Spawned WallBreakable with ID: %d, and coordinates (%.1f,%.1f)", getId(), getPosition().getX(), getPosition().getY());
}

WallBreakable::WallBreakable(const int x, const int y) // Constructor
{
	// Set up variables
	this->spawnPos = df::Vector((float) x, (float) y); // Position WallBreakable was spawned

	// Set object type
	setType("WallBreakable");

	// Set solidness
	setSolidness(df::HARD);

	// Setup WallBreakable sprite
	df::Sprite *p_temp_sprite = RM.getSprite("wallBreakable");
	if(!p_temp_sprite)
		LM.writeLog("WallBreakable::WallBreakable(): Warning! Sprite '%s' not found", "wallBreakable");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set Box
	this->setBox(df::Box());

	// Set position
	WM.moveObject(this, this->spawnPos);
}

void WallBreakable::setPositionDeserialized(df::Vector pos) // Sets position of Wall after deserialization
{
	this->spawnPos = pos;
	setPosition(pos);
}

WallBreakable::~WallBreakable() // Destructor, chance to spawn PowerUp
{
	if(NR.getServer() && !GM.getGameOver())
	{
		std::string msg = "DELETE WALLBREAKABLE|";
		msg += std::to_string((int) this->getPosition().getX());
		msg += "|";
		msg += std::to_string((int) this->getPosition().getY());
		const char* msgToSend = msg.c_str();
		LM.writeLog("WallBreakable::~WallBreakable(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Clients to delete WallBreakable
	}
}

