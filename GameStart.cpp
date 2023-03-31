// GameStart.cpp

// Engine includes
#include "Color.h"
#include "Config.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "ManagerBomb.h"
#include "ManagerMap.h"
#include "ManagerPerson.h"
#include "ManagerPowerUp.h"
#include "MiniExplosion.h"
#include "NetworkRole.h"
#include "PersonDisplay.h"
#include "PowerUp.h"
#include "NetworkRole.h"

#include "GameStart.h"

GameStart::GameStart() // Constructor, private for singleton
{
	// Set up variables
	this->isGameOver = true; // Is the game over?
	this->titleScreenMusic = RM.getMusic("titleScreen"); // Title screen music

	// Set object type
	setType("GameStart");

	// Register with events
	registerInterest(df::KEYBOARD_EVENT); // To take keyboard input

	// Setup GameStart sprite (instruction screen)
	df::Sprite *p_temp_sprite;

	p_temp_sprite = RM.getSprite("gamestart");
	if(!p_temp_sprite)
		LM.writeLog("GameStart::GameStart(): Warning! Sprite '%s' not found", "gamestart");
	else this->spriteStart = p_temp_sprite;

	p_temp_sprite = RM.getSprite("gameLoading");
	if(!p_temp_sprite)
		LM.writeLog("GameStart::GameStart(): Warning! Sprite '%s' not found", "gameLoading");
	else this->spriteLoading = p_temp_sprite;

	p_temp_sprite = RM.getSprite("gameInstructions");
	if(!p_temp_sprite)
		LM.writeLog("GameStart::GameStart(): Warning! Sprite '%s' not found", "gameInstructions");
	else this->spriteInstructions = p_temp_sprite;

	if (NR.getServer())
	{
		setSprite(this->spriteStart);
		setSpriteSlowdown(20);
	}
	else
	{
		setSprite(this->spriteLoading);
		setSpriteSlowdown(3);
	}

	// Set position
	setLocation(df::CENTER_CENTER); // Center of window

	if(df::Config::getInstance().getHeadless() == false)
	{
		playTitleScreenMusic(); // Play title music
	}
}

static GameStart* p_gameStart = nullptr;
GameStart& GameStart::getInstance() // Get the one and only instance of the GameStart
{
	if(p_gameStart == nullptr)
	{
		p_gameStart = new GameStart();
	}

	return *p_gameStart;
}

void GameStart::draw() // Draw GameStart object
{
	df::Object::draw();
}

int GameStart::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::KEYBOARD_EVENT && isMouseInWindow() == true) // Pressing "p" key starts game, pressing "q" key quits game
	{
		const df::EventKeyboard *p_keyboard_event = dynamic_cast <const df::EventKeyboard *>(p_e);
		switch(p_keyboard_event->getKey())
		{
		case df::Keyboard::P: // Play
			if(this->getSprite() == this->spriteLoading) return 1;

			if(NR.getServer() && this->isGameAutoStart == false) // Game is not auto start, so have to manually press 'P' to start game
			{
				start();
			}
			else if(NR.getClient())
			{
				// Setup GameStart sprite (loading screen)
				setSprite(this->spriteLoading);
				setSpriteSlowdown(3);

				// Have Client tell the Server it's now ready for auto start
				std::string msg = "READY FOR AUTOSTART";
				const char* msgToSend = msg.c_str(); // 'READY FOR AUTOSTART' message
				LM.writeLog("Client::parseMessage(): sending custom message: '%s'", msgToSend);
				NR.getClient()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);
			}
			break;
		case df::Keyboard::ESCAPE: // Quit
			if(this->getSprite() == this->spriteLoading) return 1;

			if(NR.getServer()) GM.setGameOver();
			else
			{
				if(!NR.getServer() && !GM.getGameOver())
				{
					NR.getClient()->writeResults(); // Write results of game to a file

					// Tell Server that it (the Client) is ready to shut down
					std::string msg = "SHUTDOWN";
					const char* msgToSend = msg.c_str(); // 'SHUTDOWN' message
					LM.writeLog("GameOver::~GameOver(): sending custom message: '%s'", msgToSend);
					NR.getClient()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);

					GM.setGameOver();
				}
			}
			break;
		case df::Keyboard::I: // Instructions
			if(this->getSprite() == this->spriteLoading) return 1;

			if(getSprite() != this->spriteInstructions)
			{
				setSprite(this->spriteInstructions);
				setSpriteSlowdown(15);
			}
			break;
		case df::Keyboard::R: // GameStart
			if(this->getSprite() == this->spriteLoading) return 1;

			if(getSprite() != this->spriteStart)
			{
				setSprite(this->spriteStart);
				setSpriteSlowdown(20);
			}
			break;
		default:
			break;
		}
		return 1;
	}

	return 0;
}

void GameStart::setTitleSprite() // Plays title music
{
	if(this->spriteStart != nullptr)
	{
		setSprite(this->spriteStart);
		setSpriteSlowdown(20);
	}
}

void GameStart::playTitleScreenMusic() // Plays title music
{
	if(this->titleScreenMusic != nullptr)
	{
		this->titleScreenMusic->play();
	}
}

void GameStart::pauseTitleScreenMusic() // Pause title music
{
	if(this->titleScreenMusic != nullptr)
	{
		this->titleScreenMusic->pause();
	}
}

bool GameStart::getIsGameOver() // Returns whether or not game is over
{
	return this->isGameOver;
}

void GameStart::setGameOver(const bool isGameOver) // Sets whether or not game is over
{
	this->isGameOver = isGameOver;
}

void GameStart::setMap(const int mapID) // Sets map
{
	this->mapID = mapID;
}

void GameStart::setGameAutoStart(const bool isGameAutoStart) // Sets whether or not the game is auto start
{
	this->isGameAutoStart = isGameAutoStart;
	if(this->isGameAutoStart == true)
	{
		df::Sprite *p_temp_sprite;
		
		if(NR.getServer())
		{
			p_temp_sprite = RM.getSprite("gamestartAuto");
			setSprite(p_temp_sprite);
			setSpriteSlowdown(15);

			if(!p_temp_sprite)
				LM.writeLog("GameStart::GameStart(): Warning! Sprite '%s' not found", "gamestartAuto");
		}
	}
}

void GameStart::start() // Sets up game for player
{	
	LM.writeLog("GameStart::GameStart(): Starting game.");

	// Set variables
	this->isGameOver = true; // Game is not started yet

	// Spawn ManagerMap, moved code to Server.cpp
	if(NR.getServer() && !GM.getGameOver())
	{
		NR.getServer()->spawnMap();

		// Send message to Client to start game
		std::string msg = "START GAME|";
		const char* msgToSend = msg.c_str(); // 'START GAME'

		LM.writeLog("GameStart::start(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to start game
	}

	// Spawn ManagerPerson
	MPer.newGameSpawn((float) MM.getXBoundMax() + 3, (float) MM.getYBoundMax(), MM.getSpawnLocations());
	LM.writeLog("GameStart::start(): Bounds -> Map XBound: %d | Map YBound: %d | HorizontalBound: %.2f | VerticalBound: %.2f", MM.getXBoundMax(), MM.getYBoundMax(), WM.getBoundary().getHorizontal(), WM.getBoundary().getVertical());

	// Spawn ManagerBomb
	MB;

	// Spawn ManagerPowerUp
	MPow;

	pauseTitleScreenMusic(); // Pause title music when game start

	// When games starts, become inactive
	setActive(false);
}