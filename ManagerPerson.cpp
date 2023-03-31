// ManagerPerson.cpp

// System includes
#include <iostream>
#include <fstream>
#include <sstream>

// Engine includes
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "BotSimple.h"
#include "BotSmart.h"
#include "BotAdvanced.h"
#include "BotPathfind.h"
#include "EventPersonDeath.h"
#include "EventVictor.h"
#include "GameOver.h"
#include "GameStart.h"
#include "ManagerBomb.h"
#include "ManagerPowerUp.h"
#include "NetworkRole.h"
#include "PersonDisplay.h"
#include "Player.h"
#include "PlayerPointToHighlight.h"

#include "ManagerPerson.h"

ManagerPerson::ManagerPerson() // Constructor, private for singleton
{
	// Set up variables
	this->nextID = 0; // Used to ensure a unique ID for every player
	this->freezeCount = 0; // Used to have a "freeze-frame" moment on game end

	// Set object type
	setType("ManagerPerson");

	// Register with events
	registerInterest(df::STEP_EVENT); // To check if game has ended
	registerInterest(PERSONDEATH_EVENT); // To update who dies
	registerInterest(VICTOR_EVENT); // To go to end game screen
}

static ManagerPerson* p_personManager = nullptr;
ManagerPerson& ManagerPerson::getInstance() // Get the one and only instance of the ManagerPerson
{
	if(p_personManager == nullptr)
	{
		p_personManager = new ManagerPerson();
	}

	return *p_personManager;
}

std::vector<df::Vector> ManagerPerson::getSpawns() // Returns vector of spawn points on current map
{
	return this->spawns;
}

std::vector<Person*> ManagerPerson::getPeople() // Returns vector of all Persons in game
{
	if(NR.getServer()) return this->people;
	else
	{
		this->people.clear();

		for(int i = 0; i < this->peopleObjectIds.size(); i++)
		{
			Person* p = (Person*) WM.objectWithId(this->peopleObjectIds.at(i));

			if(p) this->people.push_back(p);
		}

		return this->people;
	}
}

int ManagerPerson::getPersonCount() // Returns the number of Persons that played in game
{
	return this->numPersons;
}

void ManagerPerson::newGameSpawn(const float xBound, const float yBound, std::vector<df::Vector> spawnLocations) // Spawns all the Persons on game begin
{
	if(!NR.getServer()) return; // Don't spawn anyone if not Server

	// Set up variables
	this->freezeCount = 0; // Used to have a "freeze-frame" moment on game end
	this->people.clear();
	this->spawns.clear();

	// Spawn Locations
	for(int i = 0; i < spawnLocations.size(); i++)
	{
		this->spawns.push_back(spawnLocations.at(i));
	}
	std::random_shuffle(this->spawns.begin(), this->spawns.end());

	// Person sprite colors
	std::vector<df::Color> spriteColors;
	spriteColors.push_back(df::Color::BLUE);
	spriteColors.push_back(df::Color::GREEN);
	spriteColors.push_back(df::Color::YELLOW);
	spriteColors.push_back(df::Color::CYAN);
	spriteColors.push_back(df::Color::WHITE);
	spriteColors.push_back(df::Color::MAGENTA);
	spriteColors.push_back(df::Color::RED);
	std::random_shuffle(spriteColors.begin(), spriteColors.end());

	// Blink Sprite (When invincible, need an invisible (black) sprite)
	// Setup Person sprites
	df::Sprite *p_blink = RM.getSprite("blinkPlayer");
	if(!p_blink)
		LM.writeLog("ManagerPerson::ManagerPerson(): Warning! Sprite '%s' not found", "blinkPlayer");

	// Person sprites
	std::vector<df::Sprite*> personSprites;
	for(int i = 0; i < 5; i++)
	{
		// Setup Person sprites
		std::string spriteName = std::string("player" + std::to_string(i));
		df::Sprite *p_temp_sprite = RM.getSprite(spriteName);
		if(!p_temp_sprite)
			LM.writeLog("ManagerPerson::ManagerPerson(): Warning! Sprite '%s' not found", spriteName.c_str());
		else
		{ // Add sprite with a random color to vector of sprites Person could be
			p_temp_sprite->setColor(spriteColors.back());
			spriteColors.pop_back();
			personSprites.push_back(p_temp_sprite);
		}
	}
	std::random_shuffle(personSprites.begin(), personSprites.end());

	// Bot names
	/*std::vector<std::string> botNames;
	botNames.push_back("Faker");
	botNames.push_back("12345678901234567890");
	botNames.push_back("PlayerBot");
	botNames.push_back("VVinner");
	botNames.push_back("Not A Bot");
	botNames.push_back("Watermelon");
	botNames.push_back("Sleep");
	botNames.push_back("Toast");
	botNames.push_back("Monitor");
	botNames.push_back("Thread");
	botNames.push_back("MONEY");
	botNames.push_back("Screwdriver");
	botNames.push_back("Water");
	botNames.push_back("Sponsored");
	std::random_shuffle(botNames.begin(), botNames.end());*/

	this->peopleStr = ""; // Msg holding object id's of Persons spawned by Server
	
	// HUD positioning
	PersonDisplay* pDisplay = nullptr;
	PersonDisplay* p1Display = nullptr;
	df::Sprite *p_temp_sprite = RM.getSprite("personDisplay"); if(!p_temp_sprite) LM.writeLog("ManagerPerson::newGameSpawn() : ERROR, personDisplay sprite not found.");
	int numPeople = (int) NR.getServer()->getClientPersons().size();
	float nextDisplayPos = (WM.getBoundary().getVertical() - (p_temp_sprite->getHeight() * numPeople)) / (numPeople + 1);
	const float spacingBetweenPersonDisplays = (float)(std::ceil(nextDisplayPos + p_temp_sprite->getHeight()));
	if(numPeople > 2)
	{
		nextDisplayPos += std::ceil(spacingBetweenPersonDisplays / (numPeople + 1));
		nextDisplayPos = std::ceil(nextDisplayPos);
	}
	else nextDisplayPos += (spacingBetweenPersonDisplays / (numPeople + 3));

	for(auto const& pairSocketNObjectID : NR.getServer()->getClientPersons())
	{
		if(pairSocketNObjectID.second->getIsPlayer() == true) // Player
		{
			// Spawn Player
			LM.writeLog("ManagerPerson::newGameSpawn(): Spawning player");
			Player* p1 = new Player(p1Display, ("Player_" + std::to_string(pairSocketNObjectID.first)), 3, 3, (int) this->spawns.back().getX(), (int) this->spawns.back().getY(), personSprites.back());
			pDisplay = new PersonDisplay(true, df::Vector(xBound, nextDisplayPos), p1); // Spawn HUD for player
			p1->setBlinkSprite(p_blink);
			this->people.push_back(p1);
			personSprites.pop_back();
			this->spawns.pop_back();

			// Save spawned player's object id for Server to remember
			pairSocketNObjectID.second->setObjectID(p1->getId());
			this->peopleStr += std::to_string(p1->getId());
			this->peopleStr += "|";
		}
		else // Spawn Bot
		{
			LM.writeLog("ManagerPerson::newGameSpawn(): Spawning bot with brave value %d and smart value %d", pairSocketNObjectID.second->getBraveVal(), pairSocketNObjectID.second->getSmartVal());
			PersonDisplay* pBotDisplay = nullptr;
			BotPathfind* pBot = new BotPathfind(pBotDisplay, ("Bot_" + std::to_string(pairSocketNObjectID.first)), 3, 3, (int) this->spawns.back().getX(), (int) this->spawns.back().getY(), personSprites.back(), pairSocketNObjectID.second->getBraveVal(), pairSocketNObjectID.second->getSmartVal());
			pBotDisplay = new PersonDisplay(true, df::Vector(xBound, nextDisplayPos), pBot); // Spawn HUD for bot
			pBot->setBlinkSprite(p_blink);
			this->people.push_back(pBot);
			personSprites.pop_back();
			this->spawns.pop_back();

			// Save spawned bot's object id for Server to remember
			pairSocketNObjectID.second->setObjectID(pBot->getId());
			this->peopleStr += std::to_string(pBot->getId());
			this->peopleStr += "|";
		}

		nextDisplayPos += (spacingBetweenPersonDisplays);
	}

	this->peopleStr = "PERSONIDS|" + std::to_string(this->people.size()) + "|" + this->peopleStr + ";"; // 'PERSONIDS'|numPersons|objectIdPerson1|objectIdPerson2|...'

	// Tries to update Client's ManagerPerson
	const char* msgToSend = this->peopleStr.c_str(); // numPersons|'PERSONIDS'|objectIdPerson1|objectIdPerson2|...'
	LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
	NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Server to update ManagerPerson with Persons in game

	// Send message to Clients so that they know the object id of the Person they control
	for(auto const& pairSocketNObjectID : NR.getServer()->getClientPersons())
	{
		std::string msg = "PERSONOBJECTID|";
		msg += std::to_string(pairSocketNObjectID.first); // Socket id of Client
		msg += "|";
		msg += std::to_string(pairSocketNObjectID.second->getObjectID()); // Object id of Person that Client controls
		const char* msgToSend = msg.c_str(); // 'PERSONOBJECTID'|SOCKET_ID|OBJECT_ID message

		LM.writeLog("Server::spawnMap(): sending custom message: '%s'", msgToSend);
		if(!GM.getGameOver()) NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client save its controlled Person's id
	}
}

int ManagerPerson::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		step();
		return 1;
	}
	if(p_e->getType() == PERSONDEATH_EVENT) // If Person dies
	{
		if(!NR.getServer()) return 1;

		int deadPos = 0;

		for(auto person : people)
		{
			if(person->getId() == ((EventPersonDeath*) p_e)->getID())
			{
				break;
			}
			else
			{
				deadPos++;
			}
		}

		if(this->people.size() > 0 && deadPos < this->people.size())
		{
			Person* personToErase = this->people.at(deadPos);
			this->people.erase(this->people.begin() + deadPos); // Remove dead Person from people vector
			LM.writeLog("ManagerPerson::eventHandler() : Removed Person from people vector, # people left is now: %d", people.size());

			if(NR.getServer()) // Send message to Client to update their ManagerPerson to erase dead Person
			{
				std::string msg = "ERASE PERSON|";
				msg += std::to_string(personToErase->getId()); // id of Person to remove
				const char* msgToSend = msg.c_str(); // 'ERASE PERSON'|OBJECT_ID message

				LM.writeLog("Server::spawnMap(): sending custom message: '%s'", msgToSend);
				if(!GM.getGameOver()) NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to update its ManagerPowerUp
			}
		}
	}
	if(p_e->getType() == VICTOR_EVENT)
	{
		if(!NR.getServer()) return 1;

		if(freezeCount == 40) // Counts so there is a little freeze-frame moment before victor screen
		{
			Person* victor = ((EventVictor*) p_e)->getVictor(); 
			
			if(victor != nullptr)
			{
				LM.writeLog("ManagerPerson::eventHandler() : Victor! AKA %s", victor->getName().c_str());

				if(NR.getServer()) // Send message to Client to spawn GameOver
				{
					new GameOver(victor->getId()); // Send Object ID of victor (On Server)

					std::string msg = "SPAWN GAMEOVER|";
					msg += std::to_string(victor->getId()); // id of victor
					const char* msgToSend = msg.c_str(); // 'SPAWN GAMEOVER'|victorID message

					LM.writeLog("Server::spawnMap(): sending custom message: '%s'", msgToSend);
					if(!GM.getGameOver()) NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to update its ManagerPowerUp
				}
			}
			else
			{
				LM.writeLog("No victor.");

				if(NR.getServer()) // Send message to Client to spawn GameOver
				{
					new GameOver(-1); 

					std::string msg = "SPAWN GAMEOVER|";
					msg += std::to_string(-1); // Tie, so send garbage Object ID
					const char* msgToSend = msg.c_str(); // 'SPAWN GAMEOVER'|victorID message

					LM.writeLog("Server::spawnMap(): sending custom message: '%s'", msgToSend);
					if(!GM.getGameOver()) NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to update its ManagerPowerUp
				}
			}

			freezeCount++;
		}
		else
		{
			this->freezeCount++;
		}
	}

	return 0;
}

void ManagerPerson::step() // Checks for game end each frame 
{
	if(NR.getServer()) // Server
	{
		// Checks for game end each frame 
		if(this->people.size() == 1)
		{
			EventVictor victor(this->people.at(0));
			WM.onEvent(&victor);
			GS.setGameOver(true); // Let GameStart know that game is over
		}
		else if(this->people.size() == 0)
		{
			EventVictor victor(nullptr);
			WM.onEvent(&victor);
			GS.setGameOver(true); // Let GameStart know that game is over
		}
		
		// Make sure Client ManagerPerson is updated from game spawn
		const char* msgToSend = this->peopleStr.c_str(); // numPersons|'PERSONIDS|objectIdPerson1|objectIdPerson2|...'
		LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Server to update ManagerPerson with Persons in game
	}
}

Person* ManagerPerson::getPerson(int id) // Returns Person that matches given ID
{
	int pos = 0;
	for(auto person : getPeople())
	{
		if(person->getId() == id)
		{
			break;
		}
		pos++;
	}
	return people.at(pos);
}

Person* ManagerPerson::getPerson(std::string name) // Returns Person that matches given name
{
	int pos = 0;
	for(auto person : getPeople())
	{
		if(person->getName().compare(name) == 0)
		{
			break;
		}
		pos++;
	}
	return people.at(pos);
}

Person* ManagerPerson::getClosestPerson(Person* start) // Returns closest Person to given Person
{
	if (this->people.size() < 2 || start == nullptr) {
		if (this->people.size() < 2) LM.writeLog("ManagerPerson::getClosestPerson() There is only 1 player right now!");
		else LM.writeLog("ManagerPerson::getClosestPerson() Argument is a null pointer!");
		return start;
	}
	Person* closest = nullptr;
	float minDist = 9999;
	for(auto person : people)
	{
		if(person == nullptr) continue;

		if(person->getId() == start->getId()) continue;

		float dist = getDistance(start, person);

		if(dist < minDist)
		{
			minDist = dist;
			closest = person;
		}
	}

	if(closest == nullptr)
	{
		return start;
	}
	return closest;
}

Person* ManagerPerson::getRandomPerson(Person* start) // Returns a random Person that is not given Person
{
	getPeople();
	if (this->people.size() < 2 || start == nullptr) return start;

	Person* random = nullptr;

	while (random == nullptr) {
		int randID = rand() % 5;
		if (randID == start->getId()) continue; // Ignore if it's the same start
		for (auto person : people)
		{
			if (person == nullptr) continue;
			if (person->getId() == start->getId()) continue;
			if (person->getId() == randID) {
				random = person;
				break;
			}
		}
	}
	
	if(random == nullptr) return start;
	return random;
}

float ManagerPerson::getDistance(Person* start, Person* end) // Returns distance between two Persons
{
	if(start == nullptr || end == nullptr) return 0;

	df::Vector startPos = start->getPosition();
	df::Vector endPos = end->getPosition();
	float x1 = startPos.getX();
	float y1 = startPos.getY();
	float x2 = endPos.getX();
	float y2 = endPos.getY();
	float dist = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));

	return dist;
}

df::Vector ManagerPerson::getClosestBomb(Person* player)  // Returns closest Bomb to given Person
{
	// Store the list of bomb location
	std::vector<Bomb*> bombList = MB.getBombs();

	// Save the location of the player
	float playerX = player->getPosition().getX();
	float playerY = player->getPosition().getY();

	df::Vector closest;
	float minDist = 9999;

	// Run through every bombs to check wheter the player is in danger
	for(int i = 0; i < MB.getBombCount(); i++)
	{
		float bombX = bombList[i]->getCenterPos().getX();
		float bombY = bombList[i]->getCenterPos().getY();
		// Get distant between player and bomb center
		float dist = sqrt(pow((bombX - playerX), 2) + pow((bombY - playerY), 2));
		// Check whether player is in range of explosion
		if(dist < minDist)
		{
			minDist = dist;
			closest = bombList[i]->getCenterPos();
		}
	}

	return closest;
}

df::Vector ManagerPerson::getClosestPowerUp(Person* player) // Returns closest PowerUp to given Person
{
	// Store the list of bomb location
	std::vector<PowerUp*> powerUpList = MPow.getPowers();

	LM.writeLog("ManagerPerson::getClosestPowerUp() Number of Powerup: %d", MPow.getPowerCount());

	// Save the location of the player
	float playerX = player->getPosition().getX();
	float playerY = player->getPosition().getY();

	df::Vector closest;
	float minDist = 9999;

	// Run through every bombs to check wheter the player is in danger
	for(int i = 0; i < MPow.getPowerCount(); i++)
	{
		float powerX = powerUpList[i]->getPosition().getX();
		float powerY = powerUpList[i]->getPosition().getY();
		// Get distant between player and bomb center
		float dist = sqrt(pow((powerX - playerX), 2) + pow((powerY - playerY), 2));
		// Check whether player is in range of explosion
		if(dist < minDist)
		{
			minDist = dist;
			closest = powerUpList[i]->getPosition();
		}
	}

	return closest;
}

bool ManagerPerson::checkBombDanger(df::Vector playerPos, int smartVal) // Returns whether or not a Person is in danger of explosion
{
	// Store the list of bomb location
	std::vector<Bomb*> bombList = MB.getBombs();

	// Save the location of the player
	float playerX = playerPos.getX();
	float playerY = playerPos.getY();

	// If no bomb at all, by default safe
	if (MB.getBombCount() == 0) return false;

	// Run through every bombs to check wheter the player is in danger
	for (int i = 0; i < MB.getBombCount(); i++) 
	{
		float bombX = bombList[i]->getCenterPos().getX();
		float bombY = bombList[i]->getCenterPos().getY();
		// Get distant between player and bomb center
		float dist = sqrt(pow((bombX - playerX), 2) + pow((bombY - playerY), 2));
		// Get range of bomb
		float range = 0.0f;
		if (rand() % 5 <= smartVal) range = (float)bombList[i]->getBombPower();
		else range = 6.0f;
		// Check whether player is in range of explosion
		if (dist <= range && (playerX == bombX || playerY == bombY)) 
		{
			return true;
		}
	}

	// Not in range of any bombs, so the player is safe
	return false;
}

bool ManagerPerson::setPeople(std::string peopleStr) // Used by Client to set People
{
	if(!NR.getClient() || this->peopleSet == true) return false; // Only set if Client

	this->peopleStr = peopleStr; // 'PERSONIDS'|numPersons|objectIdPerson1|objectIdPerson2|...'

	std::stringstream ss(this->peopleStr.substr(10)); // Ignore PERSONIDS|
	ss >> this->numPersons; // numPersons
	if(ss.peek() == '|') ss.ignore(); // Ignore '|'

	this->people.clear();
	this->peopleObjectIds.clear();
	while(ss)
	{
		int id;
		ss >> id; // id
		if(ss.peek() == '|') ss.ignore(); // Ignore '|'

		this->peopleObjectIds.push_back(id);
		try
		{
			Person* p_per = (Person*) WM.objectWithId(id);
			if(p_per)
			{
				this->people.push_back(p_per);
			}
			else
			{
				return false;
			}
		}
		catch(...)
		{
			return false;
		}

		if(ss.peek() == ';') break; // End marked with ';'
	}

	this->peopleSet = true;
	return true;
}

void ManagerPerson::removePerson(int personID) // Used by Client to remove Person that has died from people vector
{
	if(!NR.getClient()) return;

	for(int i = 0; i < this->peopleObjectIds.size(); i++)
	{
		if(this->peopleObjectIds.at(i) == personID)
		{
			if(this->peopleObjectIds.size() > 0)
			{
				this->peopleObjectIds.erase(this->peopleObjectIds.begin() + i);
			}
			break;
		}
	}

	int deadPos = 0;
	for(auto person : getPeople())
	{
		if(person->getId() == personID)
		{
			break;
		}
		else
		{
			deadPos++;
		}
	}

	if(this->people.size() > 0 && deadPos < this->people.size())
	{
		this->people.erase(this->people.begin() + deadPos); // Remove dead Person from people vector
		LM.writeLog("ManagerPerson::eventHandler() : Removed Person from people vector, # people left is now: %d", people.size());
	}
}