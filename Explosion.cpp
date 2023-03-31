// Explosion.cpp

// System includes
#include <iostream>	
#include <stdlib.h>	// for rand()

// Engine includes
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "EventVictor.h"
#include "ManagerBomb.h"
#include "ManagerMap.h"
#include "MiniExplosion.h"
#include "Wall.h"
#include "WallBreakable.h"

#include "Explosion.h"


Explosion::Explosion(bool isOnServer, df::Vector pos, const int stayTime, const int power) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos, stayTime, power);
	}
}

void Explosion::initOnServer(df::Vector pos, const int stayTime, const int power)
{
	// Set up variables
	this->stayTime = stayTime; // How long MiniExplosions hangs around before fading
	this->x = (int) pos.getX(); // X position
	this->y = (int) pos.getY(); // Y position
	this->power = power; // Length of "arms"

	// Set up plus shaped "arms" based on given power
	std::string allBombsArePlus = "";
	allBombsArePlus += "+0";
	for(int i = 0; i < this->power; i++)
	{
		allBombsArePlus += "#";
	}
	allBombsArePlus += "|";

	std::string allBombsArePlusVertical = ""; // Vertical should be shorter
	allBombsArePlusVertical += "+0";
	for(int i = 0; i < (int) (this->power * .75); i++)
	{
		allBombsArePlusVertical += "#";
	}
	allBombsArePlusVertical += "|";

	// "Arms", a pattern of MiniExplosion that extends from center of Explosion in each cardinal direction
	this->armTop = allBombsArePlusVertical;
	this->armLeft = allBombsArePlus;
	this->armRight = allBombsArePlus;
	this->armBot = allBombsArePlusVertical;

	// Counters used to spawn arms in sequence
	this->topCounter = 0;
	this->leftCounter = 0;
	this->rightCounter = 0;
	this->botCounter = 0;

	// Set object type
	setType("Explosion");

	// Register with events
	registerInterest(df::STEP_EVENT);
	registerInterest(VICTOR_EVENT);

	// Set position
	WM.moveObject(this, df::Vector((float) x, (float) y));

	// Spawn center of Explosion
	new MiniExplosion(x, y, stayTime);
}

Explosion::Explosion(const int x, const int y, const int stayTime, const std::string armTop, std::string armLeft, const std::string armRight, const std::string armBot) // Constructor, custom explosion
{
	// Set up variables
	this->stayTime = stayTime; // How long MiniExplosions hangs around before fading
	this->x = x; // X position
	this->y = y; // Y position

	// "Arms", a pattern of MiniExplosion that extends from center of Explosion in each cardinal direction
	this->armTop = armTop;
	this->armLeft = armLeft;
	this->armRight = armRight;
	this->armBot = armBot;

	// Counters used to spawn arms in sequence
	this->topCounter = 0;
	this->leftCounter = 0;
	this->rightCounter = 0;
	this->botCounter = 0;

	// Set object type
	setType("Explosion");

	// Register with events
	registerInterest(df::STEP_EVENT);
	registerInterest(VICTOR_EVENT);

	// Set position
	WM.moveObject(this, df::Vector((float) x, (float) y));

	// Spawn center of Explosion
	new MiniExplosion(true, df::Vector((float) x, (float) y), stayTime);
}

Explosion::Explosion(const int x, const int y, const int stayTime, const int power)
{
	// Set up variables
	this->stayTime = stayTime; // How long MiniExplosions hangs around before fading
	this->x = x; // X position
	this->y = y; // Y position
	this->power = power; // Length of "arms"

	// Set up plus shaped "arms" based on given power
	std::string allBombsArePlus = "";
	allBombsArePlus += "+0";
	for(int i = 0; i < this->power; i++)
	{
		allBombsArePlus += "#";
	}
	allBombsArePlus += "|";

	std::string allBombsArePlusVertical = ""; // Vertical should be shorter
	allBombsArePlusVertical += "+0";
	for(int i = 0; i < (int) (this->power * .75); i++)
	{
		allBombsArePlusVertical += "#";
	}
	allBombsArePlusVertical += "|";

	// "Arms", a pattern of MiniExplosion that extends from center of Explosion in each cardinal direction
	this->armTop = allBombsArePlusVertical;
	this->armLeft = allBombsArePlus;
	this->armRight = allBombsArePlus;
	this->armBot = allBombsArePlusVertical;

	// Counters used to spawn arms in sequence
	this->topCounter = 0;
	this->leftCounter = 0;
	this->rightCounter = 0;
	this->botCounter = 0;

	// Set object type
	setType("Explosion");

	// Register with events
	registerInterest(df::STEP_EVENT);
	registerInterest(VICTOR_EVENT);

	// Set position
	WM.moveObject(this, df::Vector((float) x, (float) y));

	// Spawn center of Explosion
	new MiniExplosion(true, df::Vector((float) x, (float) y), stayTime);
}

// Return 0 if ignored, else 1
int Explosion::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		tick();
		return 1;
	}

	return 0;
}

void Explosion::tick() // Spawns more of Explosion each frame
{
	bool isThereMoreExplosion = false; // Is there more Explosion to spawn?

	if(topCounter < armTop.length()) // Spawn right
	{
		if(spawnArm("top") == true)
		{
			isThereMoreExplosion = true;
		}
	}
	if(leftCounter < armLeft.length()) // Spawn left
	{
		if(spawnArm("left") == true)
		{
			isThereMoreExplosion = true;
		}
	}
	if(rightCounter < armRight.length()) // Spawn right
	{
		if(spawnArm("right") == true)
		{
			isThereMoreExplosion = true;
		}
	}
	if(botCounter < armBot.length()) // Spawn bot
	{
		if(spawnArm("bot") == true)
		{
			isThereMoreExplosion = true;
		}
	}

	if(isThereMoreExplosion == false) // Destroy Explosion when there are no more MiniExplosions to spawn
	{
		WM.markForDelete(this);
	}
}

bool Explosion::isThereMiniExplosionAt(df::Vector pos) // Returns whether there is a MiniExplosion at given position
{
	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for(i.first(); !i.isDone(); i.next())
	{
		df::Object *p_o = i.currentObject();
		if(p_o->getType() == "MiniExplosion" && p_o->getPosition() == pos)
		{
			return true;
		}
	}
	return false;
}

// Returns true if successful, false otherwise
bool Explosion::spawnArm(const std::string direction) // Spawns arm in direction ['t' (top), 'l' (left), 'r' (right), 'b' (bot)]
{
	char cDir = direction[0]; // t (top), l (left), r (right), b (bot)
	int dir = 0;
	bool isThereMoreExplosion = false;
	if((cDir == 't' && y - 1 > 0) || (cDir == 'l' && x - 1 > 0) || (cDir == 'r' && x + 1 <= MM.getXBoundMax()) || (cDir == 'b' && y + 1 <= MM.getYBoundMax())) // Within bounds?
	{
		std::string str;
		int counter = 0;
		if(cDir == 't')
		{
			counter = topCounter;
			str = armTop;
			dir = -1;
		}
		else if(cDir == 'l')
		{
			counter = leftCounter;
			str = armLeft;
			dir = -1;
		}
		else if(cDir == 'r')
		{
			counter = rightCounter;
			str = armRight;
			dir = 1;
		}
		else if(cDir == 'b')
		{
			counter = botCounter;
			str = armBot;
			dir = 1;
		}

		std::string delimiter = "|";
		size_t pos = 0;
		std::string level;
		while((pos = str.find(delimiter)) != std::string::npos)
		{
			level = str.substr(0, pos);

			// Spawn correct MiniExplosion here
			char c;
			std::size_t exStart = level.find("#"); // Start of explosion part of arm
			std::size_t nullStart = level.find("_");
			if(nullStart != std::string::npos && nullStart < exStart) exStart = nullStart;
			int howFarBack = std::stoi(str.substr(0, exStart));

			if(((cDir == 't' || cDir == 'b') && x + howFarBack > 0 && x + howFarBack <= MM.getXBoundMax()) || ((cDir == 'l' || cDir == 'r') && y + howFarBack > 0 && y + howFarBack <= MM.getYBoundMax())) // Within bounds?
			{
				if(exStart + counter >= level.length()) break;

				isThereMoreExplosion = true;

				c = level[exStart + counter];
				if(c == '#') // if c == '_' means whitespace, don't spawn explosion there
				{

					if(cDir == 't' || cDir == 'b')
					{
						if(isThereMiniExplosionAt(df::Vector((float) x + howFarBack, (float) y + dir * counter)) == false)
							//new MiniExplosion(true, df::Vector((float) x + howFarBack, (float) y + dir * counter), stayTime);
							new MiniExplosion(x + howFarBack, y + dir * counter, stayTime);
					}
					else if(cDir == 'l' || cDir == 'r')
					{
						if(isThereMiniExplosionAt(df::Vector((float) x + dir * counter, (float) y + howFarBack)) == false)
							//new MiniExplosion(true, df::Vector((float) x + dir * counter, (float) y + howFarBack), stayTime);
							new MiniExplosion(x + dir * counter, y + howFarBack, stayTime);
					}
				}
			}

			str.erase(0, pos + delimiter.length());
			if(str.substr(0, str.length()).find(delimiter) == std::string::npos)
			{
				if(cDir == 't')
				{
					topCounter++;
				}
				else if(cDir == 'l')
				{
					leftCounter++;
				}
				else if(cDir == 'r')
				{
					rightCounter++;
				}
				else if(cDir == 'b')
				{
					botCounter++;
				}
			}
		}
	}
	return isThereMoreExplosion;
}

Explosion::~Explosion() // Destructor
{
	LM.writeLog("Explosion::~Explosion() : Explosion faded.");
}