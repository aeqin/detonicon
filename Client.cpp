// Client.cpp

// System includes
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

// Engine includes
#include "Config.h"
#include "EventNetwork.h"
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "Bomb.h"
#include "BotAdvanced.h"
#include "BotPathfind.h"
#include "Explosion.h"
#include "EventNuke.h"
#include "GameStart.h"
#include "GameOver.h"
#include "ManagerMap.h"
#include "ManagerBomb.h"
#include "ManagerPerson.h"
#include "ManagerPowerUp.h"
#include "MiniExplosion.h"
#include "MiniHP.h"
#include "Person.h"
#include "PersonDisplay.h"
#include "PersonDisplayBomb.h"
#include "PersonDisplayDead.h"
#include "PersonDisplayHealth.h"
#include "PersonDisplayName.h"
#include "PersonDisplayPower.h"
#include "PersonDisplaySprite.h"
#include "Player.h"
#include "PlayerPointToHighlight.h"
#include "PowerUp.h"
#include "PowerUpSpawn.h"
#include "Toast.h"
#include "Wall.h"
#include "WallBreakable.h"

#include "Client.h"

Client::Client(std::string serverName, const int lagInMS, const bool isUsingLagCompensation, const bool isPlayer, const int braveVal, const int smartVal) // Constructor
{
	// Set up variables
	this->lagInMS = lagInMS; // Amount of lag to simulate in milliseconds
	this->isPlayer = isPlayer; // true if this Client player controlled else false if a bot
	this->braveVal = braveVal; // Used to specify bot brave value
	this->smartVal = smartVal; // Used to specify bot smart value
	this->isUsingLagCompensation = isUsingLagCompensation; // true if Client is using lag compensation technique (player client movement prediction)

	// Set object type
	setType("Client");
	NM.setServer(false);

	// Register with events
	registerInterest(df::STEP_EVENT); // Checks to send messages in queue to Server (to simulate latency)

	// Connect to server
	std::string serverPort = df::DRAGONFLY_PORT;
	LM.writeLog("Client::Client(): Connecting to Server %s at port %s.", serverName.c_str(), serverPort.c_str());
	if(NM.connect(serverName, serverPort) < 0)
		LM.writeLog("Client::Client(): ERROR, Client unable to connect to Server.");

	LM.writeLog("Client::Client(): Client started");
}

int Client::getPersonObjectID() // Gets Object ID of Person controlled by Client
{
	return this->personObjectID;
}

int Client::getClientSocketID() // Gets socket of Client given by Server
{
	return this->clientSocketID;
}

bool Client::getIsUsingLagCompensation() // Returns whether or not the Client is using lag compensation (client player side movement prediction)
{
	return this->isUsingLagCompensation;
}

void Client::spawnMap(const int mapID, const int firstWallID) // Calls ManagerMap to draw the map
{
	LM.writeLog("Client::spawnMap() : Setting map as %d.", mapID);
	MM.setMapNum(mapID);
	MM.drawMap(firstWallID);
}

int Client::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT) // Every frame
	{
		NetworkLagSimulator::sendQueuedMessages(); // See if possible to send messages in queue (that holds lagged messages)
		NetworkLagSimulator::receiveQueuedMessages(nullptr); // See if possible to receive messages in queue (that holds lagged messages)
		return 1;
	}

	if(p_e->getType() == df::NETWORK_CUSTOM_EVENT) // Received custom message from Server
	{
		//parseCustomMsg(((df::EventCustomNetwork *) p_e)->getMessage());
		NetworkLagSimulator::receiveQueuedMessages((df::EventCustomNetwork *) p_e);
		return 1;
	}

	// Call parent event handler.
	return NetworkNode::eventHandler(p_e);
}

void Client::parseCustomMsg(const void* msg) // Parses custom message from Server and does something on Client
{
	std::string msgStr((char*) msg);
	LM.writeLog(" Client::parseCustomMsg() : Received message -> %s", msgStr.c_str());
	
	if(msgStr.find("SET SOCKET") != std::string::npos) // 'SET SOCKET'|socketID message
	{
		std::stringstream ss(msgStr.substr(11)); // Ignore SET SOCKET|

		ss >> this->clientSocketID; // Have Client remember its socket (from Server perspective)

		if(this->isPlayer == false) // Have bot automatically acknowledge for auto start game, players should press 'P' on the title screen
		{
			// Have Client tell the Server it's now ready to auto start game
			std::string msg = "READY FOR AUTOSTART";
			const char* msgToSend = msg.c_str(); // 'READY FOR AUTOSTART' message
			LM.writeLog("Client::parseMessage(): sending custom message: '%s'", msgToSend);
			sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);
		}
		else
		{
			GS.setTitleSprite();
		}
	}
	else if(msgStr.find("MAP") != std::string::npos) // 'SPAWN MAP'|MAP_ID|FIRST_WALL_ID message
	{
		std::stringstream ss(msgStr.substr(10)); // Ignore SPAWN MAP|
		
		ss >> this->idOfMap; // MAP_ID
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int parsedFirstWallID;
			ss >> parsedFirstWallID; // FIRST_WALL_ID

		spawnMap(this->idOfMap, parsedFirstWallID); // Spawn map
	}
	else if(msgStr.find("START") != std::string::npos) // 'START GAME'
	{   // Client received message from Server (in GameStart) to start game
		// Send message to Server that Client is ready to start game, wants Server to send back 'PERMIT ACTIONS'
		std::string msg = "STARTED GAME";
		const char* msgToSend = msg.c_str(); // 'STARTED GAME' message
		LM.writeLog("Client::parseMessage(): sending custom message: '%s'", msgToSend);
		sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);
	}
	else if(msgStr.find("PERMIT") != std::string::npos) // 'PERMIT ACTIONS'
	{
		GS.pauseTitleScreenMusic(); // Pause music
		GS.setGameOver(false); // Start game
		GS.setActive(false); // Remove loading screen

		lifeStart = Clock::now(); // Start timer for amount of time survived

		// Spawn arrows pointing to Client controlled Person to highlight what Person the Client controls
		Person* p = ((Person*) WM.objectWithId(this->personObjectID));
		if(p)
		{
			new PlayerPointToHighlight(p->getPosition().getX(), p->getPosition().getY(), p);
		}
	}
	else if(msgStr.find("EXPLOSION") != std::string::npos) // 'SPAWN EXPLOSION'|idOfObject|xCoordinate|yCoordinate|stayTime|bombPower message
	{
		std::stringstream ss(msgStr.substr(16)); // Ignore SPAWN EXPLOSION|

		int id;
			ss >> id;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int x;
			ss >> x; // xCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int y;
			ss >> y; // yCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int stayTime;
			ss >> stayTime; // stayTime
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int bombPower;
			ss >> bombPower; // bombPower

		// Spawn an explosion
		Explosion* ex = new Explosion(x - 1, y, stayTime, bombPower);
		ex->setId(id);
	}
	else if(msgStr.find("TOAST") != std::string::npos) // 'SPAWN TOAST'|idOfObject|toastAsInt|xCoordinate|yCoordinate message
	{
		std::stringstream ss(msgStr.substr(12)); // Ignore SPAWN TOAST|

		int id;
			ss >> id; // id
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int toastAsInt;
			ss >> toastAsInt; // toastAsInt
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int x;
			ss >> x; // xCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int y;
			ss >> y; // yCoordinate

		// Spawn a toast
		Toast* toast = new Toast(x, y, Toast::getToastFromInt(toastAsInt));
		toast->setId(id);
	}
	else if(msgStr.find("SHAKE") != std::string::npos) // 'SHAKE DISPLAY'|idOfObject|shakeCounterMax|shakeCounterCurr message
	{
		std::stringstream ss(msgStr.substr(14)); // Ignore SHAKE DISPLAY|

		int idOfHUDToShake;
			ss >> idOfHUDToShake; // idOfObject
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int shakeMax;
			ss >> shakeMax; // shakeCounterMax
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int shakeCurr;
			ss >> shakeCurr; // shakeCounterCurr

		PersonDisplay* hud = (PersonDisplay*)WM.objectWithId(idOfHUDToShake); 
		if(hud) // Tell synced PersonDisplay on Client side to shake
		{
			hud->setShakes(shakeMax, shakeCurr);
		}
	}
	else if(msgStr.find("BLINK") != std::string::npos) // 'BLINK'|personID message
	{
		std::stringstream ss(msgStr.substr(6)); // Ignore BLINK|

		int personID;
			ss >> personID; // personID (Get Object ID of Person to blink)
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'

		Person* p = ((Person*) WM.objectWithId(personID));
		if(p)
		{
			p->setIsInvincible(true);

			// Play taking damage sound
			df::Sound *p_sound = RM.getSound("takedmg");
			if(p_sound != nullptr && df::Config::getInstance().getHeadless() == false)
			{
				p_sound->play();
			}
		}
	}
	else if(msgStr.find("DISPLAYSPRITE") != std::string::npos) // 'SPAWN DISPLAYSPRITE'|idOfObject|xCoordinate|yCoordinate|spriteName message
	{
		std::stringstream ss(msgStr.substr(20)); // Ignore SPAWN DISPLAYSPRITE|

		int id;
			ss >> id; // id
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int xCoordinate;
			ss >> xCoordinate; // xCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int yCoordinate;
			ss >> yCoordinate; // yCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		std::string spriteName;
			ss >> spriteName; // spriteName

		// Spawn Person sprite on HUD
		PersonDisplaySprite* p = new PersonDisplaySprite(xCoordinate, yCoordinate, spriteName);
		p->setId(id);
	}
	else if(msgStr.find("DISPLAYNAME") != std::string::npos) // 'SPAWN DISPLAYNAME'|idOfObject|xCoordinate|yCoordinate|maxChars|personName message
	{
		std::stringstream ss(msgStr.substr(18)); // Ignore SPAWN DISPLAYNAME|

		int id;
			ss >> id; // id
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int xCoordinate;
			ss >> xCoordinate; // xCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int yCoordinate;
			ss >> yCoordinate; // yCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int maxChars;
			ss >> maxChars; // maxChars
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		std::string personName;
			ss >> personName; // spriteName

		// Spawn Person name on HUD
		PersonDisplayName* p = new PersonDisplayName(xCoordinate - 5, yCoordinate, personName, maxChars);
		p->setId(id);
	}
	else if(msgStr.find("SPAWN DISPLAYHEALTH") != std::string::npos) // 'SPAWN DISPLAYHEALTH'|healthID|currHP|xCoordinate|yCoordinate message
	{
		std::stringstream ss(msgStr.substr(20)); // Ignore SPAWN DISPLAYHEALTH|

		int healthID;
			ss >> healthID; // bombID
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int currHP;
			ss >> currHP; // currBombs
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int xCoordinate;
			ss >> xCoordinate; // xCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int yCoordinate;
			ss >> yCoordinate; // yCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'

		// Spawn Person bombs on HUD
		PersonDisplayHealth* p = new PersonDisplayHealth(xCoordinate - 5, yCoordinate, currHP, 24);
		p->setHealthID(healthID);
	}
	else if(msgStr.find("DISPLAYHEALTH UPDATE") != std::string::npos)// 'DISPLAYHEALTH UPDATE'|healthId|updatedStr message
	{
		std::stringstream ss(msgStr.substr(21)); // Ignore DISPLAYHEALTH UPDATE|

		int healthId;
			ss >> healthId; // healthId
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		std::string updatedStr = ss.str().substr(ss.str().find("|") + 1);

		df::ObjectList object_list = WM.objectsOfType("PersonDisplayHealth");
		df::ObjectListIterator i(&object_list);
		for(i.first(); !i.isDone(); i.next())
		{
			PersonDisplayHealth* h = (PersonDisplayHealth*) i.currentObject();

			if(h->getHealthID() == healthId)
			{
				h->setViewString(updatedStr);
				h->setColor(df::Color::WHITE);
				if((int) std::count(updatedStr.begin(), updatedStr.end(), '>') <= 1) h->setColor(df::Color::RED);
				break;
			}
		}
	}
	else if(msgStr.find("SPAWN DISPLAYBOMB") != std::string::npos) // 'SPAWN DISPLAYBOMB'|bombID|currBombs|xCoordinate|yCoordinate message
	{
		std::stringstream ss(msgStr.substr(18)); // Ignore SPAWN DISPLAYBOMB|

		int bombId;
			ss >> bombId; // bombID
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int currBombs;
			ss >> currBombs; // currBombs
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int xCoordinate;
			ss >> xCoordinate; // xCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int yCoordinate;
			ss >> yCoordinate; // yCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'

		// Spawn Person bombs on HUD
		PersonDisplayBomb* p = new PersonDisplayBomb(xCoordinate - 5, yCoordinate, currBombs, 24);
		p->setBombID(bombId);
	}
	else if(msgStr.find("DISPLAYBOMB UPDATE") != std::string::npos)// 'DISPLAYBOMB UPDATE'|bombId|updatedStr message
	{
		std::stringstream ss(msgStr.substr(19)); // Ignore DISPLAYBOMB UPDATE|

		int bombId;
			ss >> bombId; // bombID
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		std::string updatedStr = ss.str().substr(ss.str().find("|") + 1);

		df::ObjectList object_list = WM.objectsOfType("PersonDisplayBomb");
		df::ObjectListIterator i(&object_list);
		for(i.first(); !i.isDone(); i.next())
		{
			PersonDisplayBomb* b = (PersonDisplayBomb*) i.currentObject();

			if(b->getBombID() == bombId)
			{
				b->setViewString(updatedStr);
				b->setColor(df::Color::WHITE);
				if(updatedStr.find("RELOAD") != std::string::npos) b->setColor(df::Color::RED);
				break;
			}
		}
	}
	else if(msgStr.find("DISPLAYDEAD") != std::string::npos) // 'SPAWN DISPLAYDEAD'|idOfObject|xCoordinate|yCoordinate message
	{
		std::stringstream ss(msgStr.substr(18)); // Ignore SPAWN DISPLAYDEAD|

		int id;
			ss >> id; // id
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int xCoordinate;
			ss >> xCoordinate; // xCoordinate
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int yCoordinate;
			ss >> yCoordinate; // yCoordinate

		// Spawn red X that goes over HUD of dead Person
		PersonDisplayDead* p = new PersonDisplayDead(xCoordinate, yCoordinate);
		p->setId(id);
	}
	else if(msgStr.find("PERSON DEATH") != std::string::npos) // 'PERSON DEATH'|personID message
	{
		std::stringstream ss(msgStr.substr(13)); // Ignore PERSON DEATH|

		int personID;
			ss >> personID; // id

		if(this->personObjectID == personID) // Person that died is controlled by this Client
		{
			lifeDead = Clock::now(); // Time Person died
			timeSurvivedMS = (long) duration_cast<ms>(this->lifeDead - this->lifeStart).count(); // Amount of time Person survived
		}

		Person* p = ((Person*) WM.objectWithId(personID));
		if(p)
		{
			p->setCurrHP(0);
			p->setActive(false); // Set Person inactive

			// Find connecting miniHP and make sure they are deleted
			df::ObjectList object_list = WM.objectsOfType("MiniHP");
			df::ObjectListIterator i(&object_list);
			for(i.first(); !i.isDone(); i.next())
			{
				MiniHP* m = (MiniHP*) i.currentObject();
				if(m->getLeader() && m->getLeader()->isActive() == false)
				{
					m->setActive(false); // Set miniHP inactive
					//WM.markForDelete(m);
				}
			}

			// Spawn firework on death
			df::addParticles(df::ParticleType::FIREWORKS, df::Vector(p->getPosition().getX(), p->getPosition().getY()), 2.0f, df::Color::CUSTOM);
			df::addParticles(df::ParticleType::SMOKE, df::Vector(p->getPosition().getX(), p->getPosition().getY()), 0.75f, df::Color::CUSTOM);
			df::addParticles(df::ParticleType::SPARKS, df::Vector(p->getPosition().getX(), p->getPosition().getY()), 1.0f, df::Color::CUSTOM);
		}
	}
	else if(msgStr.find("PERSONIDS") != std::string::npos) // 'PERSONIDS'|numPersons|objectIdPerson1|objectIdPerson2|...'
	{
		MPer.setPeople(msgStr);
		this->numClients = MPer.getPersonCount(); // Save number of Clients
	}
	else if(msgStr.find("PERSONOBJECTID") != std::string::npos) // 'PERSONOBJECTID'|SOCKET_ID|OBJECT_ID message
	{
		std::stringstream ss(msgStr.substr(15)); // Ignore SPAWN PERSONOBJECTID|
		
		int socketID;
			ss >> socketID; // socketID
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'

		// Saves the object ID of the Person controlled by this Client
		if(this->clientSocketID == socketID)
		{
			ss >> this->personObjectID;
		}
	}
	else if(msgStr.find("PLACE BOMB") != std::string::npos) // 'PLACE BOMB'|bombID|xPosition|yPosition|personID|fuseTime|stayTime|bombPower message
	{
		std::stringstream ss(msgStr.substr(11)); // Ignore PLACE BOMB|

		int bombID;
			ss >> bombID; // bombID
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float xPos;
			ss >> xPos; // xPosition
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float yPos;
			ss >> yPos; // yPosition
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int personID;
			ss >> personID; // personID (Get Object ID of Person to place bomb)
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int fuseTime;
			ss >> fuseTime; // fuseTime
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int stayTime;
			ss >> stayTime; // stayTime
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int bombPower;
			ss >> bombPower; // stayTime
		
		if(this->personObjectID == personID) // This Client placed the bomb, so increment # bomb placed statistic
			this->numPlacedBombs++;

		Person* p = ((Person*) WM.objectWithId(personID));
		if(p)
		{
			new Bomb(df::Vector(xPos, yPos), p, bombID, fuseTime, stayTime, bombPower);
		}
	}
	else if(msgStr.find("KICK BOMB") != std::string::npos) // 'KICK BOMB'|bombID|prevX|prevY|power message
	{
		std::stringstream ss(msgStr.substr(10)); // Ignore KICK BOMB|

		int bombID;
			ss >> bombID;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float prevX;
			ss >> prevX;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float prevY;
			ss >> prevY;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int power;
			ss >> power;

		// Kick Bomb with bombID
		df::ObjectList object_list = WM.objectsOfType("Bomb");
		df::ObjectListIterator i(&object_list);
		for(i.first(); !i.isDone(); i.next())
		{
			Bomb* b = (Bomb*) i.currentObject();

			if(b->getBombID() == bombID)
			{
				b->getKicked(prevX, prevY, power);
				break;
			}
		}
	}
	else if(msgStr.find("UPDATE BOMB") != std::string::npos) // 'UPDATE BOMB'|bombID|xPos|yPos message
	{
		std::stringstream ss(msgStr.substr(12)); // Ignore UPDATE BOMB|

		int bombID;
			ss >> bombID;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float xPos;
			ss >> xPos;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float yPos;
			ss >> yPos;

		// Update position of Bomb with bombID (with end position after kick)
		df::ObjectList object_list = WM.objectsOfType("Bomb");
		df::ObjectListIterator i(&object_list);
		for(i.first(); !i.isDone(); i.next())
		{
			Bomb* b = (Bomb*) i.currentObject();

			if(b->getBombID() == bombID)
			{
				WM.moveObject(b, df::Vector(xPos, yPos));
				break;
			}
		}
	}
	else if(msgStr.find("SPAWN POWERSPAWN") != std::string::npos) // 'SPAWN POWERSPAWN'|powID|xPosition|yPosition|typeOfPowerUp message
	{
		std::stringstream ss(msgStr.substr(17)); // Ignore SPAWN POWERSPAWN|

		int powID;
			ss >> powID;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float xPos;
			ss >> xPos;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float yPos;
			ss >> yPos;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int type;
			ss >> type;

		// Spawn PowerUpSpawn
		new PowerUpSpawn(powID, xPos, yPos, type);
	}
	else if(msgStr.find("DELETE POWER") != std::string::npos) // 'DELETE POWER'|powID message
	{
		if(GM.getGameOver()) return;

		std::stringstream ss(msgStr.substr(13)); // Ignore DELETE POWER|

		float powID;
			ss >> powID;
			
		// Delete PowerUp with powID
		df::ObjectList object_list = WM.objectsOfType("PowerUp");
		df::ObjectListIterator i(&object_list);
		for(i.first(); !i.isDone(); i.next())
		{
			PowerUp* p = (PowerUp*) i.currentObject();

			if(p->getPowID() == powID)
			{
				if(!GM.getGameOver()) WM.markForDelete(p);
				//p->setActive(false);

				// Play gaining powerup sound
				df::Sound *p_sound = RM.getSound("powerup");
				if(p_sound != nullptr  && df::Config::getInstance().getHeadless() == false)
				{
					p_sound->play();
				}
			}
		}
	}
	else if(msgStr.find("DELETE WALLBREAKABLE") != std::string::npos) // 'DELETE WALLBREAKABLE'|xPosition|yPosition message
	{
		if(GM.getGameOver()) return;

		std::stringstream ss(msgStr.substr(21)); // Ignore DELETE WALLBREAKABLE|

		float xPos;
			ss >> xPos;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		float yPos;
			ss >> yPos;
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'

		// Delete WallBreakable at position
		df::Vector pos = df::Vector((float) xPos, (float) yPos);
		df::ObjectList object_list = WM.objectsAtPosition(pos);
		df::ObjectListIterator i(&object_list);
		for(i.first(); !i.isDone(); i.next())
		{
			df::Object *obj = i.currentObject();
			df::Vector objpos = obj->getPosition();

			if(obj->getType().find("WallBreakable") != std::string::npos && objpos == pos)
			{
				//obj->setActive(false);
				if(!GM.getGameOver()) WM.markForDelete(obj);
			}
		}
	}
	else if(msgStr.find("ERASE PERSON") != std::string::npos) // 'ERASE PERSON'|OBJECTID message
	{
		std::stringstream ss(msgStr.substr(13)); // Ignore ERASE PERSON|
		
		int id;
			ss >> id;

		MPer.removePerson(id); // Update ManagerPerson singleton
	}
	else if(msgStr.find("SPAWN NUKE") != std::string::npos) // 'SPAWN NUKE' message
	{
		// Spawn nuke event, which explodes all bombs on screen
		EventNuke nuke;
		WM.onEvent(&nuke);
	}
	else if(msgStr.find("SPEED UP") != std::string::npos) // 'SPEED UP'|personID message
	{
		std::stringstream ss(msgStr.substr(9)); // Ignore SPEED UP|

		int personID;
			ss >> personID; // personID
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int move_slowdown;
			ss >> move_slowdown; // move_slowdown

		Person* p = ((Person*) WM.objectWithId(personID));
		if(p)
		{
			p->setMoveSlowdown(move_slowdown);
		}
	}
	else if(msgStr.find("SPAWN GAMEOVER") != std::string::npos) // 'SPAWN GAMEOVER'|victorID message
	{
		std::stringstream ss(msgStr.substr(15)); // Ignore SPAWN GAMEOVER|

		int victorID;
			ss >> victorID; // ID of person that won

		if(this->personObjectID == victorID) this->isVictor = true; // Client won the game, update stat
		
		// Spawn GameOver screen
		new GameOver(victorID);
	}
	else if(msgStr.find("MOVEXY") != std::string::npos) // 'MOVEXY'|personID|moveX|moveY|oldX|oldY|destX|destY message
	{
		std::stringstream ss(msgStr.substr(7)); // Ignore MOVEXY|

		int personID;
			ss >> personID; // personID (Get Object ID of Person controlled by a different Client that moved)
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int moveX;
			ss >> moveX; // moveX
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int moveY;
			ss >> moveY; // moveY
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'

		int oldX;
			ss >> oldX; // oldX
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int oldY;
			ss >> oldY; // oldY

		// Assume move is legal, since authoritative Server sent it
		Person* p = ((Person*) WM.objectWithId(personID));
		if(p)
		{
			if(GS.getIsGameOver() == false && !GM.getGameOver())
			{
				WM.moveObject(p, df::Vector((float) oldX, (float) oldY)); // Move to current position
				p->moveXYTrue(moveX, moveY); // Move to new position without needing to send message to Server
			}
		}
	}
	else if(msgStr.find("REBOUNDXY") != std::string::npos) // 'REBOUNDXY'|personID|lastValidX|lastValidY
	{
		std::stringstream ss(msgStr.substr(10)); // Ignore REBOUNDXY|

		int personID;
			ss >> personID; // personID (Get Object ID of Person to have position reset)
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int lastValidX;
			ss >> lastValidX; // lastValidX
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int lastValidY;
			ss >> lastValidY; // lastValidY

		// Assume reset position is legal, since authoritative Server sent it
		Person* p = ((Person*) WM.objectWithId(personID));
		if(p)
		{
			// Initial Position
			float x1 = p->getPosition().getX();
			float y1 = p->getPosition().getY();

			if(GS.getIsGameOver() == false && !GM.getGameOver())
			{
				if(WM.moveObject(p, df::Vector((float) lastValidX, (float) lastValidY)) >= 0) // Move to last valid position
					if(personID == this->personObjectID)
					{
						incrNumTimesRebounded(); // Increment number of times rebounded for Person controlled by this Client

						// Compute rebound distance
						float x2 = (float) lastValidX;
						float y2 = (float) lastValidY;
						float dist = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
						incrNumSpacesRebounded((int) dist); // Increment number of spaces rebounded for Person controlled by this Client
					}
			}
		}
	}

	// Stats
	else if(msgStr.find("MATCHID") != std::string::npos) // 'MATCHID'|matchID
	{
		std::stringstream ss(msgStr.substr(8)); // Ignore MATCHID|

		ss >> this->matchID;
	}
	else if(msgStr.find("INCR HIT BY") != std::string::npos) // 'INCR HIT BY'
	{
		this->timesHitByExpl++;
	}
	else if(msgStr.find("INCR PICKUP") != std::string::npos) // 'INCR PICKUP'
	{
		this->numPickedUpPwrs++;
	}
	else if(msgStr.find("INCR BUMP") != std::string::npos) // 'INCR BUMP'
	{
		this->timesBumpedIntoWall++;
	}
	else if(msgStr.find("GAME TIME") != std::string::npos) // 'GAME TIME'|gameLengthMS
	{
		std::stringstream ss(msgStr.substr(10)); // Ignore GAME TIME|

		ss >> this->gameLengthMS;
	}
}

int Client::handleConnect(const df::EventNetwork *p_e) // Connects Client to server
{
	LM.writeLog("Client::handleConnect(): Client () connected!", NM.getSocket());

	std::string msg;
	if(this->isPlayer == true) // Spawn Player
	{
		msg = "SPAWN PLAYER|"; // 'SPAWN PLAYER'|lagInMS
		msg += std::to_string(this->lagInMS);
	}
	else // Spawn Bot
	{
		msg = "SPAWN BOT|"; // 'SPAWN BOT'|lagInMS|braveVal|smartVal
		msg += std::to_string(this->lagInMS);
		msg += "|";
		msg += std::to_string(this->braveVal);
		msg += "|";
		msg += std::to_string(this->smartVal);
	}
	const char* msgToSend = msg.c_str();
	LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
	sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Server to spawn Player or Bot

	// Grab timeStamp of connection to Server
	time_t timestamp;
	time(&timestamp);
	std::stringstream ss;
	ss << timestamp;
	this->timeStamp = ss.str(); // Save to string

	// Initialize singletons
	GS;
	MPer;
	MB;
	MPow;

	return 0;
}

int Client::handleData(const df::EventNetwork *p_e) // Handles received messages
{
	if(NetworkNode::handleData(p_e) == 1)
	{
		return 1;
	}
	
	return 0;
}

df::Object *Client::createObject(std::string objectType) // Creates specified game object
{
	LM.writeLog("Client::createObject(): creating object type '%s'", objectType.c_str());

	Object *p_obj = NULL;

	if(objectType.find("Person->Player") != std::string::npos)
		p_obj = new Player(nullptr, "", 0, 0, 0, 0, nullptr);
	else if(objectType == "Person->BotPathfind")
		p_obj = new BotPathfind(nullptr, "", 0, 0, 0, 0, nullptr, 0, 0);
	else if(objectType == "MiniHP")
		p_obj = new MiniHP(false, nullptr, 0, 0);
	else if(objectType == "PersonDisplay")
		p_obj = new PersonDisplay(false, df::Vector(), nullptr);
	else if(objectType == "PersonDisplayPower")
		p_obj = new PersonDisplayPower(false, df::Vector(), 0);

	else
		LM.writeLog("Client::createObject(): unknown object type '%s'", objectType.c_str());

	return p_obj;
}

void Client::incrNumSpacesMoved(int spaces) // Increments the numSpacesMoved statistic
{
	this->numSpacesMoved += spaces;
}

void Client::incrNumTimesRebounded() // Increments the numTimesRebounded statistic
{
	this->numTimesRebounded++;
}

void Client::incrNumSpacesRebounded(int spaces) // Increments the numSpacesRebounded statistic
{
	this->numSpacesRebounded += spaces;
}

void Client::incrNumMoveCmdsSent(int moveCmd) // Increments the incrNumMoveCmdsSent statistic
{
	this->numMoveCmdsSent += moveCmd;
}

void Client::writeResults() // Writes the results of the game to a file
{
	std::ofstream myfile;
	myfile.open("results/" + this->timeStamp + ".txt");
	myfile << "Match ID: " << this->matchID << std::endl; // Write ID of game match
	myfile << "Lag (MS): " << this->lagInMS << std::endl; // Write how much lag the Client had to the Server
	myfile << "Is Using Lag Compensation: " << ((this->isUsingLagCompensation == true) ? "true" : "false") << std::endl; // Write whether Client is using player client movement prediction
	myfile << "Client Socket ID: " << this->clientSocketID << std::endl; // Write Socket ID
	myfile << "Class: " << ((this->isPlayer == 1) ? "Player" : "Bot") << std::endl; // Write is Player or is Bot
	myfile << "Brave Value: " << this->braveVal << std::endl; // Write braveVal of Bot
	myfile << "Smart Value: " << this->smartVal << std::endl; // Write smartVal of Bot
	myfile << "Map ID: " << this->idOfMap << std::endl; // Write ID of map the game was played on
	myfile << "# of Clients: " << this->numClients << std::endl; // Write how many Clients played the game
	myfile << "Game Length (MS): " << this->gameLengthMS << std::endl; // Write length of game in milliseconds
	myfile << "Time Survived (MS): " << ((this->timeSurvivedMS <= 0) ? this->gameLengthMS : this->timeSurvivedMS) << std::endl; // Write length of survival in game in milliseconds, either never died (game length) or did die (time survived)
	myfile << "# of Times Hit by Explosion: " << this->timesHitByExpl << std::endl; // How many times this Client was hit by miniExplosion
	myfile << "# of Times Bumped into Wall: " << this->timesBumpedIntoWall << std::endl; // How many times this Client bumped into a wall
	myfile << "# of Powerups Picked Up: " << this->numPickedUpPwrs << std::endl; // How many powerups this Client picked up
	myfile << "# of Bombs Placed: " << this->numPlacedBombs << std::endl; // How many Bombs this Client placed
	myfile << "# of Spaces Moved: " << this->numSpacesMoved << std::endl; // How many spaces this Client moved
	myfile << "# of Move Cmds Sent: " << this->numMoveCmdsSent << std::endl; // How many move cmds this Client sent to Server
	myfile << "# of Times Rebounded: " << this->numTimesRebounded << std::endl; // How many times this Client rebounded (forced to return to a previous space by Server)
	myfile << "# of Spaces Rebounded: " << this->numSpacesRebounded << std::endl; // How many spaces this Client rebounded (sum of all distances from where Client was to where Client got snapped back to)
	myfile << "Is Victor: " << ((this->isVictor == true) ? "true" : "false") << std::endl; // Did this Client win the game
	myfile.close();
}

Client::~Client() // Destructor
{
	if(NM.isConnected()) // Close socket when Client destroyed
		NM.close();
}

