// MiniHP.cpp


// System includes
#include <iostream>

// Engine includes
#include "DisplayManager.h"
#include "EventOut.h"
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "NetworkRole.h"

#include "MiniHP.h"


MiniHP::MiniHP(bool isOnServer, df::Object* leader, const int xAway, const int yAway) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(leader, xAway, yAway);
	}
}

void MiniHP::initOnServer(df::Object* leader, const int xAway, const int yAway)
{
	// Set up variables
	this->leader = leader; // What object MiniHP follows
	this->leaderObjectID = leader->getId(); // ObjectID of what object MiniHP follows
	this->xAway = xAway; // X margin MiniHP follows leader
	this->yAway = yAway; // Y margin MiniHP follows leader

	// Set object type.
	setType("MiniHP");

	// Register with events
	registerInterest(df::STEP_EVENT); // To update position

	// Set solidness
	setSolidness(df::SPECTRAL); // Go through stuff

	// Setup MiniHP sprite.
	df::Sprite *p_temp_sprite = RM.getSprite("miniHP");
	if(!p_temp_sprite)
		LM.writeLog("MiniHP::MiniHP(): Warning! Sprite '%s' not found", "miniHP");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set altitude
	setAltitude(3); // Shows under explosions

	// Set position
	df::Vector temp_pos;
	temp_pos.setX(leader->getPosition().getX() + xAway);
	temp_pos.setY(leader->getPosition().getY() + yAway);
	WM.moveObject(this, temp_pos);
}

MiniHP::MiniHP(df::Object* leader, const int xAway, const int yAway) // Constructor
{
	// Set up variables
	this->leader = leader; // What object MiniHP follows
	this->leaderObjectID = leader->getId(); // ObjectID of what object MiniHP follows
	this->xAway = xAway; // X margin MiniHP follows leader
	this->yAway = yAway; // Y margin MiniHP follows leader

	// Set object type.
	setType("MiniHP");

	// Register with events
	registerInterest(df::STEP_EVENT); // To update position

	// Set solidness
	setSolidness(df::SPECTRAL); // Go through stuff

	// Setup MiniHP sprite.
	df::Sprite *p_temp_sprite = RM.getSprite("miniHP");
	if(!p_temp_sprite)
		LM.writeLog("MiniHP::MiniHP(): Warning! Sprite '%s' not found", "miniHP");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set altitude
	setAltitude(3); // Shows under explosions

	// Set position
	df::Vector temp_pos;
	temp_pos.setX(leader->getPosition().getX() + xAway);
	temp_pos.setY(leader->getPosition().getY() + yAway);
	WM.moveObject(this, temp_pos);
}

df::Object* MiniHP::getLeader() // Gets leader
{
	return this->leader;
}

// Return 0 if ignored, else 1
int MiniHP::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		tick();
		return 1;
	}

	return 0;
}

void MiniHP::tick() // Updates position to follow leader every frame
{
	if(this->leader != nullptr)
	{
		df::Vector temp_pos;
		temp_pos.setX(leader->getPosition().getX() + xAway);
		temp_pos.setY(leader->getPosition().getY() + yAway);
		WM.moveObject(this, temp_pos);
	}
}

std::string MiniHP::serialize(bool all) // Custom serialize for variables
{
	// Do main serialize from parent
	std::string s = Object::serialize(all);

	// Add MiniHP-specific attribute
	s += ("xAway:" + std::to_string(this->xAway) + ",");
	s += ("yAway:" + std::to_string(this->yAway) + ",");
	s += ("leaderObjectID:" + std::to_string(this->leaderObjectID) + ",");

	// Return full serialization
	return s;
}

int MiniHP::deserialize(std::string str) // Custom deserialize for variables
{
	// Do main deserialize from parent
	Object::deserialize(str);

	// Look for socket index
	std::string parseForStr = df::match("", "xAway");
	if(!parseForStr.empty())
	{
		this->xAway = stoi(parseForStr);
	}
	parseForStr = df::match("", "yAway");
	if(!parseForStr.empty())
	{
		this->yAway = stoi(parseForStr);
	}
	parseForStr = df::match("", "leaderObjectID");
	if(!parseForStr.empty())
	{
		this->leaderObjectID = stoi(parseForStr);
		if(this->leaderObjectID != -1)
		{
			this->leader = WM.objectWithId(this->leaderObjectID);
			//WM.moveObject(this, df::Vector(this->leader->getPosition().getX() + this->xAway, this->leader->getPosition().getY() + this->yAway));
		}
	}

	return 0;
}

MiniHP::~MiniHP() // Destructor
{
	// Send message to Server to delete this Object
	if(NR.getServer() && !GM.getGameOver()) // Only send delete message to Server
	{
		NR.getServer()->sendMessage(df::DELETE_OBJECT, this);
	}
}