// Bomb.cpp

// System includes
#include <stdlib.h> // for rand()

// Engine includes
#include "EventOut.h"
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "EventBomb.h"
#include "EventCollision.h"
#include "EventNuke.h"
#include "EventVictor.h"
#include "Explosion.h"
#include "GameStart.h"
#include "ManagerMap.h"
#include "NetworkRole.h"
#include "Wall.h"

#include "Bomb.h"

Bomb::Bomb(bool isOnServer, df::Vector pos, Person* owner, const int fuseTime, const int stayTime, const int bombPower) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos, owner, fuseTime, stayTime, bombPower);
	}
}

void Bomb::initOnServer(df::Vector pos, Person* owner, const int fuseTime, const int stayTime, const int bombPower)
{
	// Set up variables
	this->owner = owner; // Which Person placed this Bomb
	this->bombID = this->getId(); // ID of Bomb, for purposes of identifying Bomb across Client & Server
	this->maxFuseTime = fuseTime; // How long Bomb lasts before exploding
	this->fuseTime = fuseTime; // How long until Bomb explodes
	this->stayTime = stayTime; // How long Explosion effect lasts
	this->bombPower = bombPower; // Power of Bomb (length of Explosion arms)
	this->shortFuse = false; // Whether or not sprite is showing fuse

	this->kickedDir = df::Vector(); // Move in this direction after kick
	this->kickedPower = 0; // How long (far) to move in kickedDir direction

	// Set object type
	setType("Bomb");

	// Spawn EventBomb on spawn
	EventBomb bombSpawn(this, true);
	WM.onEvent(&bombSpawn);

	// Register with events
	registerInterest(df::STEP_EVENT); // To update time till Bomb explodes
	registerInterest(VICTOR_EVENT); // To prevent Explosion spawn on game end
	registerInterest(NUKE_EVENT); // To instantly explode

	// Set solidness
	setSolidness(df::SOFT); // Go through stuff, still have collision event if needed

	// Setup Bomb sprite
	df::Sprite *p_temp_sprite = RM.getSprite("bomb");
	if(!p_temp_sprite)
		LM.writeLog("Bomb::Bomb(): Warning! Sprite '%s' not found", "bomb");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set position
	WM.moveObject(this, pos);
}

Bomb::Bomb(df::Vector pos, Person* owner, const int bombID, const int fuseTime, const int stayTime, const int bombPower)
{
	// Set up variables
	this->owner = owner; // Which Person placed this Bomb
	this->bombID = bombID; // ID of Bomb, for purposes of identifying Bomb across Client & Server
	this->maxFuseTime = fuseTime; // How long Bomb lasts before exploding
	this->fuseTime = fuseTime; // How long until Bomb explodes
	this->stayTime = stayTime; // How long Explosion effect lasts
	this->bombPower = bombPower; // Power of Bomb (length of Explosion arms)
	this->shortFuse = false; // Whether or not sprite is showing fuse

	this->kickedDir = df::Vector(); // Move in this direction after kick
	this->kickedPower = 0; // How long (far) to move in kickedDir direction

	// Set object type
	setType("Bomb");

	// Spawn EventBomb on spawn
	EventBomb bombSpawn(this, true);
	WM.onEvent(&bombSpawn);

	// Register with events
	registerInterest(df::STEP_EVENT); // To update time till Bomb explodes
	registerInterest(VICTOR_EVENT); // To prevent Explosion spawn on game end
	registerInterest(NUKE_EVENT); // To instantly explode

	// Set solidness
	setSolidness(df::SOFT); // Go through stuff, still have collision event if needed

	// Setup Bomb sprite
	df::Sprite *p_temp_sprite = RM.getSprite("bomb");
	if(!p_temp_sprite)
		LM.writeLog("Bomb::Bomb(): Warning! Sprite '%s' not found", "bomb");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Set position
	WM.moveObject(this, pos);
}

int Bomb::getBombID() // Returns ID of Bomb, for purposes of identifying Bomb across Client & Server
{
	return this->bombID;
}

int Bomb::getBombPower() // Returns the power of the Bomb (length of Explosion "arms")
{
	return this->bombPower;
}

df::Vector Bomb::getCenterPos() // Returns position of bomb (not the middle of the sprite including the fuse)
{
	// O~*
	// ^ That part of sprite
	return df::Vector(this->getPosition().getX() - 1, this->getPosition().getY());
}

void Bomb::setBombID(const int bombID) // Sets ID of Bomb, for purposes of identifying Bomb across Client & Server
{
	this->bombID = bombID;
}

// Return 0 if ignored, else 1
int Bomb::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT) // Every frame
	{
		if(MM.isInBounds(getPosition()) == false) WM.markForDelete(this); // Explode if out of arena

		tick(); // Counts down time until Bomb explodes

		kickedPower--; // Decrement how long left to move in direction Bomb was kicked
		if(kickedPower <= 0)
		{
			kickedPower = 0;

			if(NR.getServer()) // Update bomb position on Clients
			{
				std::string msg = "UPDATE BOMB|";
				msg += std::to_string(this->bombID);
				msg += "|";
				msg += std::to_string((int) this->getPosition().getX());
				msg += "|";
				msg += std::to_string((int) this->getPosition().getY());
				const char* msgToSend = msg.c_str(); // 'UPDATE BOMB'|bombID|xPos|yPos message

				NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to all Clients to have bomb move to proper end kicked position
			}
		}
		if(!(kickedPower <= 0))
		{ 
			moveByKick(); // Move in direction Bomb was kicked
		}

		return 1;
	}

	if(p_e->getType() == VICTOR_EVENT) // Explode on the end of the game
	{
		setOffBomb();
		return 1;
	}

	if(p_e->getType() == NUKE_EVENT) // Explode all Bombs on getting nuke power up
	{
		setOffBomb();
		return 1;
	}

	return 0;
}

void Bomb::tick() // Called every frame, counts down time until bomb explodes
{
	this->fuseTime--; // Fuse countdown
	if(fuseTime < 0) // Time to explode
	{
		WM.markForDelete(this);
	}
	else if(shortFuse == false && fuseTime < maxFuseTime * .2) // Less than 20% of fuse time left, change sprite from long fuse to short fuse
	{
		// Switch sprite to Bomb with short fuse
		df::Sprite *p_temp_sprite = RM.getSprite("bombAlmost");
		if(!p_temp_sprite)
			LM.writeLog("Bomb::tick(): Warning! Sprite '%s' not found", "bombAlmost");
		else
		{
			setSprite(p_temp_sprite);
			setSpriteSlowdown(4);
		}
		shortFuse = true;
		LM.writeLog("Bomb::tick() : SWITCH SPRITE to %s", getSpriteName());
	}
}

bool Bomb::isThereSolidAt(df::Vector pos) // Checks if there is a solid object at position
{
	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for(i.first(); !i.isDone(); i.next())
	{
		df::Object *p_o = i.currentObject();
		if(p_o->getSolidness() == df::HARD && p_o->getPosition() == pos)//(this->owner->isThisSolid(*p_o) && p_o->getPosition() == pos)
		{
			return true;
		}
	}
	return false;
}

void Bomb::getKicked(const float prevX, const float prevY, const int power) // Calculates the direction & distance to travel the Bomb is kicked in
{
	// Calculates direction by taking into account the last position of the Person that kicked the Bomb
	this->kickedDir = df::Vector(-1 * (prevX - getCenterPos().getX()), -1 * (prevY - getCenterPos().getY()));
	this->kickedDir.normalize();
	this->kickedDir = df::Vector(this->kickedDir.getX() * 2, this->kickedDir.getY());
	this->kickedPower = power;

	LM.writeLog("Bomb::getKicked() : prev(%f, %f) | centerBomb(%f, %f) | kickedDir (%f, %f)", prevX, prevY, getCenterPos().getX(), getCenterPos().getY(), kickedDir.getX(), kickedDir.getY());	
}

void Bomb::moveByKick() // Moves the Bomb in the direction is was kicked in
{
	int x = (int) (this->kickedDir.getX() / 2);
	for(int i = 0; i < 2; i++)
	{
		int y = 0;
		if(this->kickedDir.getY() > 0) y = (int) (this->kickedDir.getY() - i);
		else if(this->kickedDir.getY() < 0) y = (int) (this->kickedDir.getY() + i);

		df::Vector new_pos(getPosition().getX() + x, getPosition().getY() + y);

		if((new_pos.getX() > 0 && new_pos.getX() < WM.getBoundary().getHorizontal()) && ((new_pos.getY() > 0) && (new_pos.getY() < WM.getBoundary().getVertical())))
		{ // Move both x & y
			if(NR.getServer() && this->owner->isThereSolidAt(df::Vector(new_pos.getX() - 1, new_pos.getY()))) return;
			if(!NR.getServer() && this->owner->isThereSolidHereExcludingSelf(df::Vector(new_pos.getX() - 1, new_pos.getY()))) return;

			if(WM.moveObject(this, new_pos) < 0) // Try moving diagonally, if hits wall then it returns -1
			{
				df::Vector new_posX(getPosition().getX() + x, getPosition().getY());
				if(new_posX.getX() > 0 && new_posX.getX() < WM.getBoundary().getHorizontal()) // Since it hit wall diagonally, try moving just x
				{
					if(WM.moveObject(this, new_posX) < 0) // Moving just x failed, try moving y
					{
						df::Vector new_posY(getPosition().getX(), getPosition().getY() + y);
						if((new_posY.getY() > 0) && (new_posY.getY() < WM.getBoundary().getVertical())) // Try moving just y
						{
							WM.moveObject(this, new_posY);
						}
					}
				}
			}
		}
	}
}

void Bomb::setOffBomb() // Explodes the Bomb
{
	WM.markForDelete(this);
}

std::string Bomb::serialize(bool all) // Custom serialize for variables
{
	// Do main serialize from parent
	std::string s = Object::serialize(all);

	// Add Bomb-specific attribute
	s += ("ownerID:" + std::to_string(this->owner->getId()) + ",");
	s += ("maxFuseTime:" + std::to_string(this->maxFuseTime) + ",");
	s += ("fuseTime:" + std::to_string(this->fuseTime) + ",");
	s += ("stayTime:" + std::to_string(this->stayTime) + ",");
	s += ("bombPower:" + std::to_string(this->bombPower) + ",");
	if(this->shortFuse == true) s += ("shortFuse:t");
	else s += ("shortFuse:f,");

	// Return full serialization
	return s;
}

int Bomb::deserialize(std::string str) // Custom deserialize for variables
{
	// Do main deserialize from parent
	Object::deserialize(str);

	// Look for socket index
	int ownerID;
	std::string parseForStr = df::match("", "ownerID");
	if(!parseForStr.empty())
	{
		ownerID = stoi(parseForStr); // TODO GRAB OWNER (PERSON) FROM OWNER ID
	}
	parseForStr = df::match("", "maxFuseTime");
	if(!parseForStr.empty())
	{
		this->maxFuseTime = stoi(parseForStr);
	}
	parseForStr = df::match("", "fuseTime");
	if(!parseForStr.empty())
	{
		this->fuseTime = stoi(parseForStr);
	}
	parseForStr = df::match("", "stayTime");
	if(!parseForStr.empty())
	{
		this->stayTime = stoi(parseForStr);
	}
	parseForStr = df::match("", "bombPower");
	if(!parseForStr.empty())
	{
		this->bombPower = stoi(parseForStr);
	}
	parseForStr = df::match("", "shortFuse");
	if(!parseForStr.empty())
	{
		if(parseForStr.at(0) == 'f') this->shortFuse = false;
		else this->shortFuse = true;
	}

	return 0;
}

Bomb::~Bomb() // Bomb exploded
{
	if(GM.getGameOver() || GS.getIsGameOver() == true) return; // If the game is over, don't spawn an Explosion

	// Spawn EventBomb on death
	EventBomb bombExplode(this, false);
	WM.onEvent(&bombExplode);

	// Spawn an Explosion (on Server)
	new Explosion(true, this->getCenterPos(), this->stayTime, this->bombPower);

	LM.writeLog("Bomb::~Bomb() : Bomb exploded, spawning Explosion.");
}