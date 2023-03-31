// Server.cpp

// System includes 
#include <filesystem>
#include <sstream>
#include <string>

// Engine includes
#include "EventCustomNetwork.h"
#include "EventKeyboardNetwork.h"
#include "EventMouseNetwork.h"
#include "EventNetwork.h"
#include "EventStep.h"
#include "GameManager.h"
#include "InputManager.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "ViewObject.h"
#include "WorldManager.h"

// Game includes
#include "BotAdvanced.h"
#include "GameStart.h"
#include "ManagerMap.h"
#include "Person.h"
#include "Player.h"

#include "Server.h"

Server::Server(const int mapID, const int numClientsForAutoStart) // Constructor
{	
	// Set up variables
	this->mapID = mapID;
	this->numClientsForAutoStart = numClientsForAutoStart;

	// Set object type
	setType("Server");

	// Set Server and max Clients
	if(numClientsForAutoStart != -1) MAX_CONNECTIONS = numClientsForAutoStart;
	LM.writeLog("Server::Server(): Setup for %d connections.", MAX_CONNECTIONS);
	NM.setMaxConnections(MAX_CONNECTIONS);

	// Register with events
	registerInterest(df::STEP_EVENT); // Sync objects every frame

	LM.writeLog("Server::Server(): Server successfully started.");
}

void Server::spawnMap() // Calls ManagerMap to draw the map
{
	// Random implementation
	// int mapID = min + (rand() % static_cast<int>(max - min + 1))

	LM.writeLog("Server::spawnMap() : Setting map as %d.", this->mapID);
	MM.setMapNum(this->mapID);
	int firstWallID = MM.drawMap();

	std::string msg = "SPAWN MAP|";
	msg += std::to_string(this->mapID);
	msg += "|";
	msg += std::to_string(firstWallID);
	const char* msgToSend = msg.c_str(); // 'SPAWN MAP'|MAP_ID|FIRST_WALL_ID message

	LM.writeLog("Server::spawnMap(): sending custom message: '%s'", msgToSend);
	if(!GM.getGameOver()) sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn same map

	startTimer(); // Start the timer for game length
}

void Server::startTimer() // Start game timer
{
	this->start = clock(); // Start of game

	sendMatchID(); // Tell Clients the ID of the game match
}

void Server::endTimer() // End game timer
{
	this->end = clock(); // End of game

	long gameLengthMS;
	gameLengthMS = (long)(((double) this->end - this->start) / CLOCKS_PER_SEC * 1000); // Calculate game time in milliseconds

	std::string msg = "GAME TIME|";
	msg += std::to_string(gameLengthMS);
	const char* msgToSend = msg.c_str(); // 'GAME TIME'|gameLengthMS message

	LM.writeLog("Server::endTimer(): sending custom message: '%s'", msgToSend);
	if(!GM.getGameOver()) sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to update game length stat
}

void Server::sendMatchID() // Sends ID of game match to Clients to be saved
{
	// Save game match ID as the starting time of the Server
	time_t timestamp;
	time(&timestamp); // Grab current time
	std::stringstream ss;
	ss << timestamp;
	std::string matchID = ss.str(); // Convert to string

	std::string msg = "MATCHID|";
	msg += matchID;
	const char* msgToSend = msg.c_str(); // 'MATCHID'|matchID message

	LM.writeLog("Server::sendMatchID(): sending custom message: '%s'", msgToSend);
	if(!GM.getGameOver()) sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Clients to update matchID stat
}

int Server::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT && NM.isConnected())
	{
		syncObjects();

		NetworkLagSimulator::sendQueuedMessages(); // See if possible to send messages in queue (that holds lagged messages)

		return 1;
	}

	if(p_e->getType() == df::NETWORK_CUSTOM_EVENT) // Received custom message from Client
	{
		this->mostRecentClientSocket = ((df::EventNetwork *) p_e)->getSocketIndex(); // Save the socket ID of the Client that sent this message
	
		if(this->map_Socket_to_PlayerID.find(this->mostRecentClientSocket) == this->map_Socket_to_PlayerID.end()) // Haven't spawned this Client yet
		{
			parseCustomMsg(((df::EventCustomNetwork *) p_e)->getMessage()); // Parse custom message

			std::string msg = "SET SOCKET|";
			msg += std::to_string(this->mostRecentClientSocket);
			const char* msgToSend = msg.c_str(); // 'SET SOCKET'|clientSocket message

			LM.writeLog("Server::eventHandler(): sending custom message: '%s'", msgToSend);
			if(!GM.getGameOver()) sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend, this->mostRecentClientSocket); // Send message to Client to set its unique socket ID
		}
		else
		{
			parseCustomMsg(((df::EventCustomNetwork *) p_e)->getMessage()); // Parse custom message
		}

		return 1;
	}

	// Call parent event handler.
	return NetworkNode::eventHandler(p_e);
}

std::map<int, ClientInfo*> Server::getClientPersons() // Returns pair of Client socket ID, and corresponding Object ID
{
	return this->map_Socket_to_PlayerID;
}

int Server::getSocketIDFromPersonObjectID(const int objectID) // Returns socket of Client that controls Person with Object ID
{
	for(auto const& pairSocketNObjectID : this->map_Socket_to_PlayerID)
	{
		if(pairSocketNObjectID.second->getObjectID() == objectID) // Player
		{
			return pairSocketNObjectID.first;
		}
	}

	return -1;
}

int Server::getNumAcceptedClients() // Returns # Clients that connected successfully to Server
{
	return this->numAcceptedClients;
}

int Server::getNumStartedClients()// Returns # Clients that received 'START GAME' message and replied to Server
{
	return this->numStartedClients;
}

void Server::parseCustomMsg(const void* msg) // Parses custom message from Client and does something on Server
{
	std::string msgStr((char*) msg);

	LM.writeLog(" Server::parseCustomMsg() : Received message -> %s", msgStr.c_str());

	if(msgStr.find("PLAYER") != std::string::npos) // 'SPAWN PLAYER'|lagInMS
	{
		std::stringstream ss(msgStr.substr(13)); // Ignore SPAWN PLAYER|

		int lag;
			ss >> lag; // lag in milliseconds

		// Maps socket to Client controlled Player
		this->map_Socket_to_PlayerID[this->mostRecentClientSocket] = new ClientInfo(true); // Player
		this->map_Socket_to_PlayerID[this->mostRecentClientSocket]->setLagInMS(lag); // Has Server remember Client's simulated lag
	}
	else if(msgStr.find("BOT") != std::string::npos) // 'SPAWN BOT'|lagInMS|braveVal|smartVal
	{
		std::stringstream ss(msgStr.substr(10)); // Ignore SPAWN BOT|

		int lag;
			ss >> lag; // lag in milliseconds
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int braveVal;
			ss >> braveVal; // braveVal
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int smartVal;
			ss >> smartVal; // smartVal

		// Maps socket to Client controlled Bot
		this->map_Socket_to_PlayerID[this->mostRecentClientSocket] = new ClientInfo(false, braveVal, smartVal); // Bot
		this->map_Socket_to_PlayerID[this->mostRecentClientSocket]->setLagInMS(lag); // Has Server remember Client's simulated lag
	}
	else if(msgStr.find("MOVEXY(NO_LAGCOMP)") != std::string::npos) // 'MOVEXY(NO_LAGCOMP)'|moveX|moveY|oldX|oldY
	{
		std::stringstream ss(msgStr.substr(19)); // Ignore MOVEXY(NO_LAGCOMP)|

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

		int personID = this->map_Socket_to_PlayerID[this->mostRecentClientSocket]->getObjectID(); // Get Object ID of Person controlled by Client
		Person* p = ((Person*) WM.objectWithId(personID));
		if(p)
		{
			//WM.moveObject(p, df::Vector((float) oldX, (float) oldY));
			df::Vector oldPos = p->getPosition();
				oldX = (int) oldPos.getX();
				oldY = (int) oldPos.getY();
			p->moveXYTrue(moveX, moveY); // Attempt Move

			// Send message to Clients to move the Person
			std::string msg = "MOVEXY|";
			msg += std::to_string(personID);
			msg += "|";
			msg += std::to_string(moveX);
			msg += "|";
			msg += std::to_string(moveY);
			msg += "|";
			msg += std::to_string(oldX);
			msg += "|";
			msg += std::to_string(oldY);
			msg += "|";
			msg += std::to_string((int) p->getPosition().getX());
			msg += "|";
			msg += std::to_string((int) p->getPosition().getY());
			const char* msgToSend = msg.c_str(); // 'MOVEXY'|personID|moveX|moveY|oldX|oldY|destX|destY message

			sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to move Person moving
		}
	}
	else if(msgStr.find("MOVEXY(LAGCOMP)") != std::string::npos) // 'MOVEXY(LAGCOMP)'|moveX|moveY|oldX|oldY|destX|destY
	{
		if(GS.getIsGameOver() || GM.getGameOver()) return;

		std::stringstream ss(msgStr.substr(16)); // Ignore MOVEXY(LAGCOMP)|

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
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'

		int destX;
			ss >> destX; // Destination pos X
			if(ss.peek() == '|') ss.ignore(); // Ignore '|'
		int destY;
			ss >> destY; // Destination pos Y

		int personID = this->map_Socket_to_PlayerID[this->mostRecentClientSocket]->getObjectID(); // Get Object ID of Person controlled by Client
		Person* p = ((Person*) WM.objectWithId(personID)); // Get Person
		if(p)
		{
			if(p->getFlagIgnoreMove() == true) // Ignore move commands unless originating position is from last valid position (because of bounceback)
			{
				if(!(oldX == p->getLastValidX() && oldY == p->getLastValidY())) // Last valid position does not match position of Person giving move command
				{
					return; // So ignore move command
				}
			}
			
			WM.moveObject(p, df::Vector((float) oldX, (float) oldY)); // Make sure Person at right starting position

			p->moveXYTrue(moveX, moveY); // Attempt Move
			df::Vector newServerPos = p->getPosition(); // Position of Person after Server moves
			df::Vector newClientPos = df::Vector((float) destX, (float) destY); // Position of Person after Client moves

			p->setLastValidPos(newServerPos); // Sets the Client's last valid position as the position the Server moved to

			if(newServerPos != newClientPos && this->map_Socket_to_PlayerID[this->mostRecentClientSocket]->getLagInMS() > 0) // Server and Client disagree on move, so Client needs to be rebounded
			{
				p->setFlagIgnoreMove(true); // Set flag for Server to ignore Client moves until they start from the last valid position

				std::string msg = "REBOUNDXY|";
				msg += std::to_string(personID);
				msg += "|";
				msg += std::to_string(p->getLastValidX());
				msg += "|";
				msg += std::to_string(p->getLastValidY());
				const char* msgToSend = msg.c_str(); // 'REBOUNDXY'|personID|lastValidX|lastValidY message

				sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to all Clients to reset Person moving to last valid position
			}
			else // Server and Client agree on move, so have Server propogate move to all other Clients
			{
				p->setFlagIgnoreMove(false); // Set flag for Server to accept Client moves

				// Send message to all OTHER Clients to move the moving Person on their side
				std::string msg = "MOVEXY|";
				msg += std::to_string(personID);
				msg += "|";
				msg += std::to_string(moveX);
				msg += "|";
				msg += std::to_string(moveY);
				msg += "|";
				msg += std::to_string(oldX);
				msg += "|";
				msg += std::to_string(oldY);
				msg += "|";
				msg += std::to_string(destX);
				msg += "|";
				msg += std::to_string(destY);
				const char* msgToSend = msg.c_str(); // 'MOVEXY'|personID|moveX|moveY|oldX|oldY|destX|destY message

				if(!GM.getGameOver())
				{
					LM.writeLog("Server::parseCustomMsg(): sending custom message: '%s'", msgToSend);
					int maxSocket = MAX_CONNECTIONS;
					for(int i = 0; i < maxSocket; i++)
					{
						if(i != this->mostRecentClientSocket) // Only update Clients that didn't move the Person and need to be synced
						{
							sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend, i); // Send message to Client to move Person moving
						}
					}
				}
			}
			
		}
	}
	else if(msgStr.find("PLACEBOMB") != std::string::npos) // 'PLACEBOMB'
	{
		int personID = this->map_Socket_to_PlayerID[this->mostRecentClientSocket]->getObjectID(); // Get object id of Person controlled by Client
		if(personID != -1 && WM.objectWithId(personID))
		{
			((Person*) WM.objectWithId(personID))->placeBomb();
		}
	}
	else if(msgStr.find("READY FOR AUTOSTART") != std::string::npos) // 'READY FOR AUTOSTART'
	{
		this->numStartedClients++;
		if(this->numClientsForAutoStart != -1 && this->numClientsForAutoStart == this->numStartedClients) // Auto start game (without waiting to press 'P' on the title screen)
		{
			this->numClientsForAutoStart = -1;
			GS.start();
		}
	}
	else if(msgStr.find("STARTED GAME") != std::string::npos) // 'STARTED GAME'
	{
		if(this->numStartedClients == this->map_Socket_to_PlayerID.size()) // All Clients received a message to start game and have sent their reply
		{
			GS.setGameOver(false); // Start Game

			// Send message to every Client to allow their Persons to start moving
			std::string msg = "PERMIT ACTIONS";
			const char* msgToSend = msg.c_str(); // 'PERMIT ACTIONS' message

			LM.writeLog("Server::parseMessage(): sending custom message: '%s'", msgToSend);
			if(!GM.getGameOver()) sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);
		}
	}
	else if(msgStr.find("SHUTDOWN") != std::string::npos) // 'SHUTDOWN'
	{
		this->map_Socket_to_PlayerID[this->mostRecentClientSocket]->setReadyToShutdown(true); // Set this Client as ready to shutdown

		bool shutDownServer = true;
		for(auto const& sock_player : this->map_Socket_to_PlayerID)
		{
			if(sock_player.second->getReadyToShutdown() == false)
			{
				shutDownServer = false;
				break;
			}
		}

		// Server shuts down AFTER all Clients are ready to shut down because of simulated lag in NetworkLagSimulator
		if(shutDownServer == true)
		{
			GM.setGameOver(); // All Clients have shut down, so it is safe to shutdown
			sendMessage(df::SET_GAME_OVER);
		}
	}
}

int Server::handleAccept(const df::EventNetwork *p_e)
{
	// Set up variables
	this->numAcceptedClients++; // Increment # Clients that connected successfully to Server

	if(this->spawnOnce == true) return 1; // Already spawned Server, so don't reset the variables

	LM.writeLog("Server::handleAccept(): Server accepts client connection.");
	
	// Grab number of maps in maps directory
	int numMaps = 0;
	std::string mapDirectory = "maps/";
	for(const auto & entry : std::experimental::filesystem::directory_iterator(mapDirectory))
	{
		std::string path = entry.path().string();
		if(path.substr(5).find("map") != std::string::npos) // Substr to remove "maps/" directory from path
		{
			numMaps++;
		}
	}

	// Set up variables
	this->numMaps = numMaps; // Number of spawnable maps
	this->spawnOnce = true; // Only spawn once on first accept, instead of multiple on subsequent accepts

	GS; // Start GameStart
	if(this->numClientsForAutoStart != -1) GS.setGameAutoStart(true);

	return 1;
}

int Server::handleData(const df::EventNetwork *p_e)
{
	return NetworkNode::handleData(p_e); // If all else fails, return parent's
}

void Server::syncObjects() // Syncs Object across Clients
{
	df::ObjectList objectList = WM.getAllObjects();
	df::ObjectListIterator iterator(&objectList);
	for(iterator.first(); !iterator.isDone(); iterator.next())
	{
		Object* currObject = (Object *) iterator.currentObject();

		// Persons (Player, AdvancedBot) are synced when created, destroyed, or modified
		if(currObject->getType().find("Person->") != std::string::npos)
		{
			if(currObject->isModified(df::ID) || currObject->isModified(df::VISIBLE) || currObject->isModified(df::POSITION))
			{
				LM.writeLog("Server::doSync(): Synchronizing %s (id %d).", currObject->getType().c_str(), currObject->getId());
				sendMessage(df::SYNC_OBJECT, currObject);			
			}
		}

		// MiniHP are synced when created, modified, or destroyed
		else if(currObject->getType() == "MiniHP")
		{
			if(currObject->isModified(df::ID) || currObject->isModified(df::VISIBLE))// || currObject->isModified(df::POSITION))
			{
				LM.writeLog(2, "Server::doSync(): Synchronizing %s (id %d).", currObject->getType().c_str(), currObject->getId());
				sendMessage(df::SYNC_OBJECT, currObject);
			}
		}
		
		// PersonDisplay are synced when created, modified, or destroyed
		else if(currObject->getType() == "PersonDisplay")
		{
			if(currObject->isModified(df::ID))// || currObject->isModified(df::POSITION))
			{
				LM.writeLog(2, "Server::doSync(): Synchronizing %s (id %d).", currObject->getType().c_str(), currObject->getId());
				sendMessage(df::SYNC_OBJECT, currObject);
			}
		}
		
		// PersonDisplays (PersonDisplayBomb, PersonDisplayHealth, PersonDisplayPower) are synced when created, modified, or destroyed
		else if(currObject->getType() == "PersonDisplayPower")
		{
			df::ViewObject* view = (df::ViewObject*)currObject;
			if(view->isModified(df::VIEW_STRING))
			{
				LM.writeLog(2, "Server::doSync(): Synchronizing %s (id %d).", currObject->getType().c_str(), currObject->getId());
				sendMessage(df::SYNC_OBJECT, currObject);
			}
		}
	}
}

