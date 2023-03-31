// ManagerMap.cpp

// System includes
#include <fstream>
#include <iostream>
#include <stdlib.h>	// for rand()

// Engine includes
#include "EventOut.h"
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "GameStart.h"
#include "ManagerPerson.h"
#include "ManagerPowerUp.h"
#include "NetworkRole.h"
#include "Person.h"
#include "PowerUp.h"
#include "PowerUpSpawn.h"
#include "WallBreakable.h"

#include "ManagerMap.h"

ManagerMap::ManagerMap()
{
	// Set up variables
	this->mapNum = 1; // ID of current map
	this->spawnEvery = 30; // Speed to shrink map
	this->baseTimeUntilSpawnPowerUp = 80; // Base time until spawning a PowerUp
	this->timeUntilSpawnPowerUp = this->baseTimeUntilSpawnPowerUp; // Time until spawning a PowerUp
	this->frameCount = 0; // Number of frames from the start of the game
	this->timeToDeleteWalls = false; // Is it time to start deleting walls?

	// Bounds of map
	this->xBoundMin = 0;
	this->xBoundMax = this->numCols;
	this->yBoundMin = 0;
	this->yBoundMax = this->numRows;

	// Set object type
	setType("ManagerMap");

	// Register with events
	registerInterest(df::STEP_EVENT); // For spawning PowerUps and spawning enclosing Walls
}

static ManagerMap* p_mapManager = nullptr;
ManagerMap& ManagerMap::getInstance() // Get the one and only instance of the MapManager
{
	if(p_mapManager == nullptr)
	{
		p_mapManager = new ManagerMap();
	}

	return *p_mapManager;
}

int ManagerMap::getMaxRows() // Returns numRows
{
	return this->numRows;
}

int ManagerMap::getMaxCols() // Returns numCols
{
	return this->numCols;
}

int ManagerMap::getXBoundMin() // Gets min x
{
	return this->xBoundMin;
}

int ManagerMap::getXBoundMax() // Gets max x
{
	return this->xBoundMax;
}

int ManagerMap::getYBoundMin() // Gets min y
{
	return this->yBoundMin;
}
int ManagerMap::getYBoundMax() // Gets max y
{
	return this->yBoundMax;
}

std::vector<df::Vector> ManagerMap::getSpawnLocations() // Returns vector of spawn locations
{
	return this->spawnLocations;
}

void ManagerMap::setMapNum(const int mapNum) // Sets current map ID 
{
	// Set variables
	this->timeUntilSpawnPowerUp = this->baseTimeUntilSpawnPowerUp; // Time until spawning a PowerUp
	this->frameCount = 0; // Number of frames from the start of the game
	this->timeToDeleteWalls = false; // Is it time to start deleting walls?
	this->mapNum = mapNum;
	
	// Reset bound variables
	this->xBoundMin = 0;
	this->xBoundMax = this->numCols;
	this->yBoundMin = 0;
	this->yBoundMax = this->numRows;
}

int ManagerMap::eventHandler(const df::Event *p_e) // Handles events
{
	if(GS.getIsGameOver() == true) return 1;

	if(p_e->getType() == df::STEP_EVENT)
	{
		this->frameCount++;
		spawnEnclosingWalls();

		this->timeUntilSpawnPowerUp--;
		spawnPowerUp();
	}

	return 0;
}

bool ManagerMap::nearSpawn(const int x, const int y) // Returns whether or not position is near a spawn
{
	for(auto spawn : this->spawnLocations)
	{
		int spawnX = (int) spawn.getX();
		int spawnY = (int) spawn.getY();

		double dist = sqrt(pow((x - spawnX), 2) + pow((y - spawnY), 2));

		if(dist <= 2)
		{
			return true;
		}
	}
	return false;
}

bool ManagerMap::isInBounds(df::Vector pos) // Returns whether or not position is out of map bounds
{
	float x = pos.getX();
	float y = pos.getY();
	int buffer = 1;

	if(x < getXBoundMin() - buffer * 2 || x > getXBoundMax() + buffer * 2 || y < getYBoundMin() - buffer || y > getYBoundMax() + buffer) return false;
	return true;
}

int ManagerMap::drawMap(const int idToStartDrawing) // Draws map, returns object ID of first wall drawn (for client synching purposes)
{
	this->spiralEdgeLocations.clear();
	this->wallsToBeDeleted.clear();
	this->updateBounds.clear();
	this->spawnLocations.clear();

	// Assuming unbreakable walls 1 char thick around entire playing field
	// Set spiral variables
	int spiralXMin = 1;
	int spiralXMax = this->numCols - 2;
	int spiralYMin = 1;
	int spiralYMax = this->numRows - 2;
	getSpiralLocations(spiralXMin, spiralXMax, spiralYMin, spiralYMax); // Populates spiralEdgeLocations vector (of enclosing walls)

	// Updates map bounds based on shrinking map
	this->xBoundMin = spiralXMin;
	this->xBoundMax = spiralXMax;
	this->yBoundMin = spiralYMin;
	this->yBoundMax = spiralYMax;
	this->timeToDeleteWalls = false;
	this->frameCount = 0;

	bool flag_firstWallDrawn = false; // While false, record ID of first wall drawn
	int firstWallID = -1; // ID of first wall drawn by Server
	int givenFirstWallID = idToStartDrawing; // ID of first wall to draw, received from Server

	std::ifstream file;
	switch(this->mapNum)
	{
	case 2: 
		file = std::ifstream("maps/map2.txt");
		this->spawnEvery = 60;
		break;
	case 3:
		file = std::ifstream("maps/map3.txt");
		this->spawnEvery = 60;
		break;
	case 4:
		file = std::ifstream("maps/map4.txt");
		this->spawnEvery = 100;
		break;
	case 5:
		file = std::ifstream("maps/map5.txt");
		this->spawnEvery = 60;
		break;
	case 6:
		file = std::ifstream("maps/map6.txt");
		this->spawnEvery = 20;
		break;
	case 7:
		file = std::ifstream("maps/map7.txt");
		this->spawnEvery = 20;
		break;
	default:
		file = std::ifstream("maps/map7.txt");
		this->spawnEvery = 20;
		break;
	}
		
	if(file.is_open())
	{
		float row = 0;
		float col = 0;
		std::string line;
		while(getline(file, line))
		{
			col = 0;
			if(line.at(0) == '#') continue; // Skip comment lines that begin with #
			for(auto c : line) // Otherwise read in line of map
			{
				if(row == 0 || col == 0 || row == this->numRows - 1 || col == this->numCols - 1)
				{
					Wall* wall;
					if(row == this->numRows / 2)
					{
						wall = new Wall(true, "wall_38Tall", df::Vector(col, row));
					}
					if(col == this->numCols / 2)
					{
						wall = new Wall(true, "wall_98Long", df::Vector(col, row), givenFirstWallID);
						givenFirstWallID = -1; // Just drew first wall
						if(flag_firstWallDrawn == false) // Record ID of first wall (used to sync Object ids with Clients)
						{
							firstWallID = wall->getId();
							flag_firstWallDrawn = true;
						}
					}
				}
				else
				{
					c = tolower(c);
					if(c == 'u') // U for Unbreakable
					{
						new Wall(true, "wall", df::Vector(col, row));
					}
					else if(c == 'b') // B for Breakable
					{
						new WallBreakable(true, df::Vector((float) col, (float) row));
					}
					else if(c == 's') // S for spawn point
					{
						this->spawnLocations.push_back(df::Vector((float) col, (float) row));
					}
				}
				col++;
			}
			row++;
		}
		file.close(); // Finished reading in map file

		if(this->spawnLocations.size() < 5)
		{
			LM.writeLog("ManagerMap::drawMap(): ERROR, not enough spawn points in given map num %d, need %d more spawn points", this->mapNum, (5 - this->spawnLocations.size()));
		}
	}
}

void ManagerMap::spawnPowerUp() // Spawns PowerUp
{
	if(NR.getClient()) return; // Don't spawn PowerUp on Client, only Server

	if(this->timeUntilSpawnPowerUp <= 0)
	{
		int percentOffBase = (int) ((float) (this->baseTimeUntilSpawnPowerUp / 3));
		int inconsistency = (-percentOffBase + (rand() % static_cast<int>(percentOffBase - (-percentOffBase) + 1)));
		this->timeUntilSpawnPowerUp = this->baseTimeUntilSpawnPowerUp + inconsistency;

		float possX = (float) (getXBoundMin() + (rand() % static_cast<int>(getXBoundMax() - getXBoundMin() + 1))); // Formula between ranges is min + (rand() % static_cast<int>(max - min + 1))
		float possY = (float) (getYBoundMin() + (rand() % static_cast<int>(getYBoundMax() - getYBoundMin() + 1)));

		if(WM.objectsAtPosition(df::Vector(possX, possY)).getCount() == 0 && MPow.getPowerCount() < 30)
		{
			new PowerUpSpawn(possX, possY); // Spawn on Server
		}
	}
}

void ManagerMap::getSpiralLocations(const int xMin, const int xMax, const int yMin, const int yMax) // Populates spiralEdgeLocations vector (of enclosing walls)
{
	int spiralXMin = xMin;
	int spiralXMax = xMax;

	int spiralYMin = yMin;
	int spiralYMax = yMax;

	bool xMatch = false;
	bool yMatch = false;

	while(xMatch == false && yMatch == false)
	{
		if(spiralXMax - spiralXMin <= 1) xMatch = true;
		if(spiralYMax - spiralYMin <= 1) yMatch = true;

		std::vector<df::Vector> edgeOfSpiralLocations;

		// Do up & down spiral edges twice to make up for the greater y dist compared to x
		for(int y = spiralYMin; y <= spiralYMax; y++) // vvv
		{
			for(int x = spiralXMax; ; )
			{
				edgeOfSpiralLocations.insert(edgeOfSpiralLocations.begin(), df::Vector((float) x, (float) y));
				break;
			}
		}
		if(xMatch == false)
		{
			spiralXMax--;
			this->updateBounds.insert(this->updateBounds.begin(), 2); // -2, -1, 1, 2 | -2 = xBoundMin, -1 = yBoundMin, 1 = yBoundMax, 2 = xBoundMax
		}
		this->spiralEdgeLocations.insert(this->spiralEdgeLocations.begin(), edgeOfSpiralLocations);
		edgeOfSpiralLocations.clear();

		for(int y = spiralYMax; y >= spiralYMin; y--) // ^^^
		{
			for(int x = spiralXMin; ; )
			{
				edgeOfSpiralLocations.insert(edgeOfSpiralLocations.begin(), df::Vector((float) x, (float) y));
				break;
			}
		}
		if(xMatch == false)
		{
			spiralXMin++;
			this->updateBounds.insert(this->updateBounds.begin(), -2); // -2, -1, 1, 2 | -2 = xBoundMin, -1 = yBoundMin, 1 = yBoundMax, 2 = xBoundMax
		}
		this->spiralEdgeLocations.insert(this->spiralEdgeLocations.begin(), edgeOfSpiralLocations);
		edgeOfSpiralLocations.clear();

		for(int y = spiralYMin; y <= spiralYMax; y++) // vvv
		{
			for(int x = spiralXMax; ; )
			{
				edgeOfSpiralLocations.insert(edgeOfSpiralLocations.begin(), df::Vector((float) x, (float) y));
				break;
			}
		}
		if(xMatch == false)
		{
			spiralXMax--;
			this->updateBounds.insert(this->updateBounds.begin(), 2); // -2, -1, 1, 2 | -2 = xBoundMin, -1 = yBoundMin, 1 = yBoundMax, 2 = xBoundMax
		}
		this->spiralEdgeLocations.insert(this->spiralEdgeLocations.begin(), edgeOfSpiralLocations);
		edgeOfSpiralLocations.clear();

		for(int y = spiralYMax; y >= spiralYMin; y--) // ^^^
		{
			for(int x = spiralXMin; ; )
			{
				edgeOfSpiralLocations.insert(edgeOfSpiralLocations.begin(), df::Vector((float) x, (float) y));
				break;
			}
		}
		if(xMatch == false)
		{
			spiralXMin++;
			this->updateBounds.insert(this->updateBounds.begin(), -2); // -2, -1, 1, 2 | -2 = xBoundMin, -1 = yBoundMin, 1 = yBoundMax, 2 = xBoundMax
		}
		this->spiralEdgeLocations.insert(this->spiralEdgeLocations.begin(), edgeOfSpiralLocations);
		edgeOfSpiralLocations.clear();

		for(int y = spiralYMin; ; ) // >>>
		{
			for(int x = spiralXMin; x <= spiralXMax; x++)
			{
				edgeOfSpiralLocations.insert(edgeOfSpiralLocations.begin(), df::Vector((float) x, (float) y));
			}
			break;
		}
		if(yMatch == false)
		{
			spiralYMin++;
			this->updateBounds.insert(this->updateBounds.begin(), -1); // -2, -1, 1, 2 | -2 = xBoundMin, -1 = yBoundMin, 1 = yBoundMax, 2 = xBoundMax
		}
		this->spiralEdgeLocations.insert(this->spiralEdgeLocations.begin(), edgeOfSpiralLocations);
		edgeOfSpiralLocations.clear();

		for(int y = spiralYMax; ; ) // <<<
		{
			for(int x = spiralXMax; x >= spiralXMin; x--)
			{
				edgeOfSpiralLocations.insert(edgeOfSpiralLocations.begin(), df::Vector((float) x, (float) y));
			}
			break;
		}
		if(yMatch == false)
		{
			spiralYMax--;
			this->updateBounds.insert(this->updateBounds.begin(), 1); // -2, -1, 1, 2 | -2 = xBoundMin, -1 = yBoundMin, 1 = yBoundMax, 2 = xBoundMax
		}
		this->spiralEdgeLocations.insert(this->spiralEdgeLocations.begin(), edgeOfSpiralLocations);
		edgeOfSpiralLocations.clear();
	}
}

void ManagerMap::spawnEnclosingWalls() // Spawns enclosing walls (shrinks map)
{
	if(this->frameCount % this->spawnEvery == 0 && this->spiralEdgeLocations.size() > 0)
	{
		std::vector<Wall*> wallsToDelete;
		if(this->timeToDeleteWalls == true && this->wallsToBeDeleted.size() > 0)
		{
			for(auto wall : this->wallsToBeDeleted.back())
			{
				WM.markForDelete(wall);
			}
			this->wallsToBeDeleted.pop_back();
		}

		for(auto wallLocation : this->spiralEdgeLocations.back())
		{
			// Delete WallBreakable on top of wall
			df::ObjectList object_list = WM.objectsAtPosition(wallLocation);
			df::ObjectListIterator i(&object_list);
			for(i.first(); !i.isDone(); i.next())
			{
				df::Object *p_o = i.currentObject();
				if(p_o->getType().find("Wall") != std::string::npos) // Consume WallBreakable if spawn on top of one
				{
					p_o->setActive(false);
					WM.markForDelete(p_o);
					break;
				}
			}

			Wall* wall = new Wall(true, "wall", wallLocation); // Spawn Wall

			wallsToDelete.insert(wallsToDelete.begin(), wall);
		}
		this->wallsToBeDeleted.insert(this->wallsToBeDeleted.begin(), wallsToDelete);
		this->spiralEdgeLocations.pop_back();

		// Update bounds after placing walls
		if(this->updateBounds.size() > 0)
		{
			int update = this->updateBounds.back();
			switch(update)// -2 = xBoundMin, -1 = yBoundMin, 1 = yBoundMax, 2 = xBoundMax
			{
			case -2:
				this->xBoundMin++;
				break;
			case -1:
				this->yBoundMin++;
				break;
			case 1:
				this->yBoundMax--;
				break;
			case 2:
				this->xBoundMax--;
				break;
			}
			this->updateBounds.pop_back();
		}
	}
	else if(this->spiralEdgeLocations.size() == 0) // Map shrunk to center, delete everyone
	{
		for(auto dedBoi : MPer.getPeople())
		{
			dedBoi->takeDamage(9999);
		}
	}
	if(this->timeToDeleteWalls == false && this->frameCount % (spawnEvery * 6) == 0)
	{
		this->timeToDeleteWalls = true;
	}
}
