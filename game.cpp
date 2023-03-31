// game.cpp

// System includes 
#include <filesystem>
#include <iostream>
#include <string>
#include <sstream>

// Engine includes
#include "GameManager.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "ResourceManager.h"

// Game includes
#include "Client.h"
#include "GameStart.h"
#include "NetworkRole.h"
#include "Server.h"

// Function prototypes
void loadResources(void);
void populateWorld(void);
void helpMenu(void);

int main(int argc, char *argv[])
{
	bool isServer = false;
	bool isPlayer = false;
	bool isUsingLagCompensation = false; // Using player Client side prediction
	std::string serverName;
	int numClientsForAutoStart = -1; // Number of Clients needed for automatic game start (when specified number of Clients connect)
	int mapID = 7; // ID of map to spawn
	int lagInMS = 0; // Amount of lag simulated from the game
	int braveVal = -1; // Range from 0 to 4
	int smartVal = -1; // Range from 0 to 4

	// Parse command line
	if(argc < 2 || argc > 7) // Too little/much options (command in wrong format)
	{
		helpMenu();
	}
	else
	{
		char* spawnArg = argv[1];
		if(!(strcmp(spawnArg, "-s") == 0 || strcmp(spawnArg, "-p") == 0 || strcmp(spawnArg, "-b") == 0)) helpMenu(); // Not specifying to create a server, a player, or a bot (command in wrong format)

		if(strcmp(spawnArg, "-s") == 0 && argc > 1 && argc < 5) // Need to set up Server
		{
			isServer = true;

			if(argc == 3) // Just map id
			{
				std::string val = std::string(argv[2]);
				std::istringstream ss(val);
				if(!(ss >> mapID).fail()) // Yes integer
				{
					// Integer stored in mapID
				}
				else // Not integer
				{
					helpMenu();
				}
			}
			else if(argc == 4) // Possibly supplied numClientsAutoStart argument
			{
				std::string val = std::string(argv[2]);
				std::istringstream ss(val);
				if(!(ss >> mapID).fail()) // Yes integer
				{
					// Integer stored in mapID

					std::string val2 = std::string(argv[3]);
					std::istringstream ss2(val2);
					if(!(ss2 >> numClientsForAutoStart).fail()) // Yes integer
					{
						// Integer stored in numClientsForAutoStart
					}
					else // Not integer
					{
						numClientsForAutoStart = -1;
					}
				}
				else // Not integer
				{
					helpMenu();
				}
			}
		}
		else
		{
			if(argc > 2)
			{
				try
				{
					char* lagCompArg = argv[2];
					if(!(strcmp(lagCompArg, "-lc") == 0 || strcmp(lagCompArg, "-wolc") == 0)) helpMenu(); // Not specifying to create a server, a player, or a bot (command in wrong format)
					else
					{
						if(strcmp(lagCompArg, "-lc") == 0) isUsingLagCompensation = true;
						else if(strcmp(lagCompArg, "-wolc") == 0) isUsingLagCompensation = false;
					}

					serverName = argv[3];
					lagInMS = std::stoi(argv[4]);
					if(strcmp(spawnArg, "-p") == 0) // Spawn player
					{
						isPlayer = true;
					}
					else if(strcmp(spawnArg, "-b") == 0) // Spawn bot
					{
						isPlayer = false;

						if(argc != 7) helpMenu(); // Command in wrong format

						braveVal = std::stoi(argv[5]);
						smartVal = std::stoi(argv[6]);
					}
				}
				catch(...)
				{
					helpMenu();
				}
			}
			else if(isServer == false)
			{
				helpMenu();
			}
		}
	}

	// Unique log file for server and clients
	if(isServer)
	#if defined(_WIN32) || defined(_WIN64)
		_putenv_s("DRAGONFLY_LOG", "server.log");
	#endif
	else
	#if defined(_WIN32) || defined(_WIN64)
		_putenv_s("DRAGONFLY_LOG", "client.log");
	#endif 

	// Start up GameManager
	if(GM.startUp())
	{
		LM.writeLog("game.cpp::main(): ERROR, GameManager would not start");
		GM.shutDown();
		return 0;
	}

	// Set flush of logfile during development (when done, make false)
	LM.setFlush(true);

	// Load game resources
	loadResources();

	// Set up Server or Client
	if(isServer) // Set up Server
	{
		df::splash();

		Server* server = new Server(mapID, numClientsForAutoStart);
		NR.setServer(server);
		NR.setClient(NULL);
		NM.setServer(true);
		if(!NM.isServer())
		{
			LM.writeLog("game.cpp::main(): ERROR, Server failed to set");
			GM.setGameOver();
			return 1;
		}
	}
	else
	{
		Client* client = new Client(serverName, lagInMS, isUsingLagCompensation, isPlayer, braveVal, smartVal);
		NR.setClient(client);
		NR.setServer(NULL);
		LM.writeLog("game.cpp::main(): Client is set");
	}

	// Seed random
	srand((unsigned int) time(NULL));

	// Run game (this blocks until game loop is over)
	GM.run();

	// Shut down connections
	if(NR.getServer())
		NetworkRole::getInstance().getServer()->sendMessage(df::SET_GAME_OVER);
	else
		NetworkRole::getInstance().getClient()->sendMessage(df::SET_GAME_OVER);

	// Shut everything down
	GM.shutDown();
}

void loadResources(void) // Load resources (sprites, sound effects, music)
{
	// Sprites
	// Game start & over & instructions & loading screens
	RM.loadSprite("sprites/gamestart-spr.txt", "gamestart");
	RM.loadSprite("sprites/gamestartAuto-spr.txt", "gamestartAuto");
	RM.loadSprite("sprites/gameover-spr.txt", "gameover");
	RM.loadSprite("sprites/gameoverTie-spr.txt", "gameoverTie");
	RM.loadSprite("sprites/gameInstructions-spr.txt", "gameInstructions");
	RM.loadSprite("sprites/gameLoading-spr.txt", "gameLoading");

	// Player sprites
	RM.loadSprite("sprites/player0-spr.txt", "player0");
	RM.loadSprite("sprites/player1-spr.txt", "player1");
	RM.loadSprite("sprites/player2-spr.txt", "player2");
	RM.loadSprite("sprites/player3-spr.txt", "player3");
	RM.loadSprite("sprites/player4-spr.txt", "player4");
	RM.loadSprite("sprites/blinkPlayer-spr.txt", "blinkPlayer");
	RM.loadSprite("sprites/playerPointToHighlight-spr.txt", "playerPointToHighlight");

	// HUD stuff
	RM.loadSprite("sprites/personDisplay-spr.txt", "personDisplay");
	RM.loadSprite("sprites/personDisplayDamaged-spr.txt", "personDisplayDamaged");
	RM.loadSprite("sprites/personDisplayDead-spr.txt", "personDisplayDead");
	
	RM.loadSprite("sprites/miniHP-spr.txt", "miniHP");
	RM.loadSprite("sprites/damagedMiniHP-spr.txt", "damagedMiniHP");

	// Powerups
	RM.loadSprite("sprites/powerUpHidden-spr.txt", "powerUpHidden"); // {??}
	RM.loadSprite("sprites/powerUpBlink-spr.txt", "powerUpBlink"); // { }
	RM.loadSprite("sprites/powerUpAmmo-spr.txt", "powerUpAmmo"); // {O+}
	RM.loadSprite("sprites/powerUpHP-spr.txt", "powerUpHP"); // {>+}
	RM.loadSprite("sprites/powerUpBombLength-spr.txt", "powerUpBombLength"); // {#+}
	RM.loadSprite("sprites/powerUpSpeed-spr.txt", "powerUpSpeed"); // {sp}
	RM.loadSprite("sprites/powerUpNuke-spr.txt", "powerUpNuke"); // {NK}
	RM.loadSprite("sprites/powerUpSpawn-spr.txt", "powerUpSpawn"); // Shrinking circle b4 spawning powerUp

	// Walls
	RM.loadSprite("sprites/wall-spr.txt", "wall");
	RM.loadSprite("sprites/wallBreakable-spr.txt", "wallBreakable");
	RM.loadSprite("sprites/wall_98Long-spr.txt", "wall_98Long");
	RM.loadSprite("sprites/wall_38Tall-spr.txt", "wall_38Tall");

	// Bomb/Explosions
	RM.loadSprite("sprites/bomb-spr.txt", "bomb");
	RM.loadSprite("sprites/bombAlmost-spr.txt", "bombAlmost");
	RM.loadSprite("sprites/miniExplosion-spr.txt", "miniExplosion");

	// Sounds
	RM.loadSound("sounds/pop.wav", "pop"); // "closePop july408" by atomwrath freesound.org
	RM.loadSound("sounds/bump.wav", "bump"); // "Hit" by jfreem3 freesound.org
	RM.loadSound("sounds/powerup.wav", "powerup"); // "Spacey 1up/Power up" by GameAudio freesound.org
	RM.loadSound("sounds/takeDmg.wav", "takedmg"); // "Getting Hit Damage Scream" by dersuperanton freesound.org

	// Music 
	RM.loadMusic("sounds/titleScreen.wav", "titleScreen"); // "Moose" by www.bensound.com
}

void populateWorld(void) // Populate world with some objects
{
	// Move base Object id up to get past
	// non-game Objects
	df::Object::max_id = 10;

	GS; // Spawn GameStart
}

void helpMenu(void) // Provides instructions for command line
{
	fprintf(stderr, "Windows Command (Server): \n\tdetonicon.exe -s [mapID] [numClients]\n");
	fprintf(stderr, "Windows Command (Player): \n\tdetonicon.exe -p {-lc|-wolc} [serverName] [lagAmount]\n");
	fprintf(stderr, "Windows Command (Bot):	   \n\tdetonicon.exe -b {-lc|-wolc} [serverName] [lagAmount] [braveValue] [smartValue]\n");
	fprintf(stderr, "\t-s            run as a server\n");
	fprintf(stderr, "\t-p            run client as a player\n");
	fprintf(stderr, "\t-b            run client as a bot\n");
	fprintf(stderr, "\t-lc           run client with lag compensation\n");
	fprintf(stderr, "\t-wolc         run client without lag compensation\n");
	fprintf(stderr, "\t[mapID]       run server with specified map\n");
	fprintf(stderr, "\t[numClients]  run server to AUTOMATICALLY start when specified number of clients connected (OPTIONAL argument)\n");
	fprintf(stderr, "\t[server name] run client attempting to connect to specified server\n");
	fprintf(stderr, "\t[lagAmount]   run client with lag in milliseconds\n");
	fprintf(stderr, "\t[braveValue]  specify brave value if spawning a bot (-b option)\n");
	fprintf(stderr, "\t[smartValue]  specify smart value if spawning a bot (-b option)\n");
	exit(1);
}