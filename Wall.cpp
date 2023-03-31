// Wall.cpp

// System includes
#include <iostream>
#include <stdlib.h>	// for rand()
#include <string>

// Engine includes
#include "EventOut.h"
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ManagerMap.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "NetworkRole.h"

#include "Wall.h"

Wall::Wall(bool isOnServer, std::string spriteName, df::Vector pos, const int id) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(spriteName, pos, id);
	}
}

void Wall::initOnServer(std::string spriteName, df::Vector pos, const int id)
{
	// Set object ID
	if(id != -1)
		setId(id);

	// Set object type
	setType("Wall");

	// Register with events
	registerInterest(df::STEP_EVENT); // To attempt to delete Walls that haven't been synced properly

	// Set solidness
	setSolidness(df::HARD);

	// Setup Wall sprite
	df::Sprite *p_temp_sprite = RM.getSprite(spriteName);
	if(!p_temp_sprite)
		LM.writeLog("Wall::Wall(): Warning! Sprite '%s' not found", spriteName);
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set Box
	this->setBox(df::Box());

	// Set position
	WM.moveObject(this, pos);

	LM.writeLog("Wall::initOnServer() : Spawned Wall with ID: %d, and coordinates (%.1f,%.1f)", getId(), getPosition().getX(), getPosition().getY());
}

Wall::Wall(const int x, const int y)
{
	// Set up variables
	this->givenPos = df::Vector((float) x, (float) y); // Position it was meant to be spawned in
	this->atGivenPos = false; // Is Wall at the position it was meant to be spawned in?

	// Set object type
	setType("Wall");

	// Set solidness
	setSolidness(df::HARD);

	// Setup Wall sprite
	df::Sprite *p_temp_sprite = RM.getSprite("wall");
	if(!p_temp_sprite)
		LM.writeLog("Wall::Wall(): Warning! Sprite '%s' not found", "wall");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set Box
	setBox(df::Box());

	// Set position
	WM.moveObject(this, df::Vector((float) x, (float) y));
	if(getPosition() == this->givenPos) this->atGivenPos = true;
}

Wall::Wall(const int x, const int y, std::string spriteName)
{
	// Set up variables
	this->givenPos = df::Vector((float) x, (float) y); // Position it was meant to be spawned in
	this->atGivenPos = false; // Is Wall at the position it was meant to be spawned in?

	// Set object type
	setType("Wall");

	// Set solidness
	setSolidness(df::HARD);

	// Setup Wall sprite.
	df::Sprite *p_temp_sprite = RM.getSprite(spriteName);
	if(!p_temp_sprite)
		LM.writeLog("Wall::Wall(): Warning! Sprite '%s' not found", spriteName.c_str());
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set Box
	setBox(df::Box());

	// Set position
	WM.moveObject(this, df::Vector((float) x, (float) y));
}

Wall::~Wall() // Destructor
{}
