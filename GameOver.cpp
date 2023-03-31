// GameOver.cpp

// System includes
#include <chrono>
#include <iostream>
#include <thread>
#include <stdlib.h>

// Engine includes
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "GameStart.h"
#include "ManagerMap.h"
#include "NetworkRole.h"

#include "GameOver.h"

GameOver::GameOver(int victorObjectID)
{
	// Set up variables
	this->time_to_live = 225; // Time until GameOver destroys itself
	this->nextFireworkStep = 20; // Time until GameOver spawns another firework graphic

	// Set object type
	setType("GameOver");

	// Register with events
	registerInterest(df::STEP_EVENT); // To count down time to live and time until spawning another firework graphic

	// Setup GameOver sprite
	df::Sprite *p_temp_sprite;
	if(victorObjectID == -1)
	{
		p_temp_sprite = RM.getSprite("gameoverTie");
	}
	else
	{
		p_temp_sprite = RM.getSprite("gameover");
	}
	if(!p_temp_sprite)
		LM.writeLog("GameOver::GameOver(): Warning! Sprite '%s' not found", "gameover");
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(15);
		setTransparency('#');
	}

	// Set position
	setLocation(df::CENTER_CENTER); // Center of window

	GS.setGameOver(true); // Set game over

	// Loop through and delete excess Objects
	df::ObjectList object_list = WM.getAllObjects(true);
	df::ObjectListIterator i(&object_list);
	for(i.first(); !i.isDone(); i.next())
	{
		df::Object *p_o = i.currentObject();
		if(canDeleteThisObject(p_o) == true)
		{
			if(victorObjectID == p_o->getId()) // Move winner of match to specific place
			{
				df::Vector new_pos(getPosition().getX() + 22, getPosition().getY() + 2);
				p_o->setSolidness(df::SPECTRAL);
				WM.moveObject(p_o, new_pos);
				p_o->setVisible(true);
				LM.writeLog("GameOver::GameOver() - Victor is %s", ((Person *) WM.objectWithId(victorObjectID))->getName().c_str());
			}
			else
			{
				if(NR.getServer()) p_o->setActive(false);
				else
				{
					p_o->setActive(false);
					if(p_o->getType().find("Wall") != std::string::npos)
					{
						p_o->setActive(false);
						//WM.markForDelete(p_o);
					}
				}
			}
		}
	}

	if(NR.getServer() && !GM.getGameOver()) NR.getServer()->endTimer(); // End game length timer
	if(NR.getClient()) NR.getClient()->writeResults(); // Write results of game to a file
}

void GameOver::draw() // Draws gameover animation
{
	df::Object::draw();
}

int GameOver::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		step();
		return 1;
	}

	return 0;
}

void GameOver::step() // Updates GameOver time to live per frame & time until spawning another firework graphic
{
	time_to_live--;
	if(time_to_live <= 0)
	{
		//WM.markForDelete(this);
		// Server shuts down AFTER all Clients are ready to shut down because of simulated lag needs the Server to keep working (can't shut down early) in NetworkLagSimulator
		if(!NR.getServer() && !GM.getGameOver())
		{
			// Tell Server that it (the Client) is ready to shut down
			std::string msg = "SHUTDOWN";
			const char* msgToSend = msg.c_str(); // 'SHUTDOWN' message
			LM.writeLog("GameOver::~GameOver(): sending custom message: '%s'", msgToSend);
			NR.getClient()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);
		}
	}
	else if(time_to_live % nextFireworkStep == 0)
	{
		fireworksShow();
		this->nextFireworkStep = 10 + (rand() % static_cast<int>(20 - 10 + 1)); // min + (rand() % static_cast<int>(max - min + 1)
	}
}

bool GameOver::canDeleteThisObject(df::Object* obj) // Returns whether or not this Object should be deleted on game over
{
	if(obj == this || obj->getType() == "GameStart" || obj->getType().find("Manager") != std::string::npos || obj->getType() == "NetworkNode" || obj->getType() == "Server" || obj->getType() == "Client")// ManagerBomb, ManagerPowerUp, ManagerMap, ManagerPerson
		return false;
	return true;
}

void GameOver::fireworksShow() // Spawns a firework graphic
{
	float possX = (float) (MM.getXBoundMin() + (rand() % static_cast<int>(WM.getBoundary().getHorizontal() - MM.getXBoundMin() + 1))); // Formula between ranges is min + (rand() % static_cast<int>(max - min + 1))
	float possY = (float) (MM.getYBoundMin() + (rand() % static_cast<int>(MM.getYBoundMax() - MM.getYBoundMin() + 1)));

	df::Color fireColor = df::Color(rand() % 10);
	df::Color sparkColor = df::Color(rand() % 10);

	df::addParticles(df::ParticleType::FIREWORKS, df::Vector(possX, possY), 3.0f, fireColor);
	df::addParticles(df::ParticleType::SMOKE, df::Vector(possX, possY), 1.0f, df::Color::CUSTOM);
	df::addParticles(df::ParticleType::SPARKS, df::Vector(possX, possY), 1.5f, sparkColor);
}

GameOver::~GameOver() // Destructor
{
	// Server shuts down AFTER all Clients are ready to shut down because of simulated lag needs the Server to keep working (can't shut down early) in NetworkLagSimulator
	if(!NR.getServer() && !GM.getGameOver())
	{	
		// Tell Server that it (the Client) is ready to shut down
		std::string msg = "SHUTDOWN";
		const char* msgToSend = msg.c_str(); // 'SHUTDOWN' message
		LM.writeLog("GameOver::~GameOver(): sending custom message: '%s'", msgToSend);
		NR.getClient()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);
	}
}