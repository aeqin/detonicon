// Bomb.cpp

// System includes
#include <stdlib.h>	// for rand()

// Engine includes
#include "Config.h"
#include "DisplayManager.h"
#include "EventCollision.h"
#include "EventOut.h"
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "Bomb.h"
#include "EventMiniExplosion.h"
#include "Explosion.h"
#include "NetworkRole.h"
#include "Person.h"
#include "Player.h"
#include "PowerUp.h"

#include "MiniExplosion.h"

MiniExplosion::MiniExplosion(bool isOnServer, df::Vector pos, const int stayTime) // Constructor, only creates Object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos, stayTime);
	}
}

void MiniExplosion::initOnServer(df::Vector pos, const int stayTime) // Create Object on Server
{
	// Set up variables
	this->maxStayTime = stayTime; // How long MiniExplosion lasts before fading
	this->stayTime = stayTime; // Current time until MiniExplosion fades

	// Set object type
	setType("MiniExplosion");

	// Spawn EventMiniExplosion on spawn
	EventMiniExplosion miniExplosionSpawn(this, true);
	WM.onEvent(&miniExplosionSpawn);

	// Register with events
	registerInterest(df::STEP_EVENT); // For updating time until fading

	// Set solidness
	setSolidness(df::SOFT); // Go through stuff, still have collision event if needed

	// Setup MiniExplosion sprite.
	df::Sprite *p_temp_sprite = RM.getSprite("miniExplosion");
	if(!p_temp_sprite)
		LM.writeLog("MiniExplosion::MiniExplosion(): Warning! Sprite '%s' not found", "miniExplosion");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set altitude
	setAltitude(4);

	// Set Box
	this->setBox(df::Box());// df::Vector(), (float) this->getSprite()->getWidth() / 3, (float) this->getSprite()->getHeight() / 3));

	// Set position
	WM.moveObject(this, pos);

	// Play spawning "fireworks pop" sound
	df::Sound *p_sound = RM.getSound("pop");
	if(p_sound != nullptr && df::Config::getInstance().getHeadless() == false) p_sound->play();
}

MiniExplosion::MiniExplosion(const int x, const int y, const int stayTime) // Constructor
{
	// Set up variables
	this->maxStayTime = stayTime; // How long MiniExplosion lasts before fading
	this->stayTime = stayTime; // Current time until MiniExplosion fades

	// Set object type
	setType("MiniExplosion");

	// Register with events
	registerInterest(df::STEP_EVENT); // For updating time until fading

	// Set solidness
	setSolidness(df::SOFT); // Go through stuff, still have collision event if needed

	// Setup MiniExplosion sprite.
	df::Sprite *p_temp_sprite = RM.getSprite("miniExplosion");
	if(!p_temp_sprite)
		LM.writeLog("MiniExplosion::MiniExplosion(): Warning! Sprite '%s' not found", "miniExplosion");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set altitude
	setAltitude(4);

	// Set Box
	this->setBox(df::Box());// df::Vector(), (float) this->getSprite()->getWidth() / 3, (float) this->getSprite()->getHeight() / 3));
	
	// Set position
	WM.moveObject(this, df::Vector((float) x, (float) y));

	// Spawn EventMiniExplosion on spawn
	EventMiniExplosion miniExplosionSpawn(this, true);
	WM.onEvent(&miniExplosionSpawn);

	// Play spawning "fireworks pop" sound
	df::Sound *p_sound = RM.getSound("pop");
	if(p_sound != nullptr && df::Config::getInstance().getHeadless() == false) p_sound->play();
}

// Return 0 if ignored, else 1
int MiniExplosion::eventHandler(const df::Event *p_e) // Handles event
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		tick();
		return 1;
	}

	if(p_e->getType() == df::COLLISION_EVENT)
	{
		const df::EventCollision *p_collision_event = dynamic_cast <df::EventCollision const *> (p_e);
		collision(p_collision_event);
		return 1;
	}

	return 0;
}

void MiniExplosion::tick() // Count down bomb fuse
{
	// Time to stay countdown
	this->stayTime--;
	if(stayTime < 0) // Time to fade
		WM.markForDelete(this);
}

void MiniExplosion::collision(const df::EventCollision *p_collision_event) // Handles collisions
{
	df::Object* obj1 = p_collision_event->getObject1();
	std::string type1 = obj1->getType();
	df::Object* obj2 = p_collision_event->getObject2();
	std::string type2 = obj2->getType();

	LM.writeLog("MiniExplosion::collision(): obj1 %s collided with obj2 %s", type1.c_str(), type2.c_str());

	// WALLS
	if(type1 == "Wall" && type2 == "MiniExplosion")
	{
		WM.markForDelete(obj2); // Destroy mini explosion
	}
	else if(type1 == "MiniExplosion" && type2 == "Wall")
	{
		WM.markForDelete(obj1); // Destroy mini explosion
	}
	else if(type1 == "WallBreakable" && type2 == "MiniExplosion")
	{
		WM.markForDelete(obj1); // Destroy breakable wall
	}
	else if(type1 == "MiniExplosion" && type2 == "WallBreakable")
	{
		WM.markForDelete(obj2); // Destroy breakable wall
	}

	// BOMB
	else if(type1 == "MiniExplosion" && type2 == "Bomb")
	{
		 ((Bomb*) (obj2))->setOffBomb(); // Explosion touches a bomb, immediately explode it
	}
	else if(type1 == "Bomb" && type2 == "MiniExplosion")
	{
		((Bomb*) (obj1))->setOffBomb(); // Explosion touches a bomb, immediately explode it
	}

	// PERSON
	else if(type1 == "MiniExplosion" && type2.find("Person") != std::string::npos)
	{
		if(((Person*) (obj2))->getIsInvincible() == false && NR.getServer() && !GM.getGameOver()) // Send message to Client to increment # times hit stat
		{
			std::string msg = "INCR HIT BY"; // 'INCR HIT BY'
			const char* msgToSend = msg.c_str();
			LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
			NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend, NR.getServer()->getSocketIDFromPersonObjectID(obj2->getId())); // Send message to Client to increment # times hit stat
		}

		((Person*) (obj2))->takeDamage(1); // Explosion touches a Person, do damage
	}
	else if(type1.find("Person") != std::string::npos && type2 == "MiniExplosion")
	{
		if(((Person*) (obj1))->getIsInvincible() == false && NR.getServer() && !GM.getGameOver()) // Send message to Client to increment # times hit stat
		{
			std::string msg = "INCR HIT BY"; // 'INCR HIT BY'
			const char* msgToSend = msg.c_str();
			LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
			NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend, NR.getServer()->getSocketIDFromPersonObjectID(obj1->getId())); // Send message to Client to increment # times hit stat
		}

		((Person*) (obj1))->takeDamage(1); // Explosion touches a Person, do damage
	}

	// POWERUP
	else if(type1 == "MiniExplosion" && type2.find("PowerUp") != std::string::npos)
	{
		if(((PowerUp*) (obj2))->getIsInvincible() == true) return;

		WM.markForDelete(obj2); // Explosion touches a PowerUp, destroy it
	}
	else if(type1.find("PowerUp") != std::string::npos && type2 == "MiniExplosion")
	{
		if(((PowerUp*) (obj1))->getIsInvincible() == true) return;

		WM.markForDelete(obj1); // Explosion touches a PowerUp, destroy it
	}
}

MiniExplosion::~MiniExplosion() // Destructor
{
	// Spawn EventMiniExplosion on MiniExplosion fade
	EventMiniExplosion miniExplosionFade(this, false);
	WM.onEvent(&miniExplosionFade);
}