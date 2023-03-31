// ManagerMap.h

#pragma once

// System includes
#include <vector>

// Game includes
#include "Wall.h"

#define MM ManagerMap::getInstance() // Two-letter acronym for easier access to manager

class ManagerMap : public df::Object
{
private:
	int mapNum; // ID of current map
	int spawnEvery; // Speed to shrink map
	int baseTimeUntilSpawnPowerUp; // Base time until spawning a PowerUp
	int timeUntilSpawnPowerUp; // Time until spawning a PowerUp
	int frameCount; // Number of frames from the start of the game
	bool timeToDeleteWalls; // Is it time to start deleting walls?
	
	// Bounds of map
	const int numRows = 40; // For 2D array use 20
	const int numCols = 98; // For 2D array use 49
	int xBoundMin;
	int xBoundMax;
	int yBoundMin;
	int yBoundMax;

	std::vector<df::Vector> spawnLocations; // Vector of where players spawn
	std::vector<std::vector<df::Vector>> spiralEdgeLocations; // Vector of each "edge" of spiral
	std::vector<int> updateBounds; // -2, -1, 1, 2 | -2 = xBoundMin, -1 = yBoundMin, 1 = yBoundMax, 2 = xBoundMax
	std::vector<std::vector<Wall*>> wallsToBeDeleted; // Vector of Walls to be deleted

	ManagerMap(); // Constructor, private for singleton
public:
	static ManagerMap &getInstance(); // Get the one and only instance of the MapManager

	// Variable getters
	int getMaxRows(); // Returns numRows
	int getMaxCols(); // Returns numCols
	int getXBoundMin(); // Returns min x
	int getXBoundMax(); // Returns max x
	int getYBoundMin(); // Returns min y
	int getYBoundMax(); // Returns max y
	std::vector<df::Vector> getSpawnLocations(); // Returns vector of spawn locations

	// Variable setters
	void setMapNum(const int mapNum); // Sets current map ID 

	int eventHandler(const df::Event *p_e); // Handles events
	bool nearSpawn(const int x, const int y); // Returns whether or not position is near a spawn
	bool isInBounds(df::Vector pos); // Returns whether or not position is out of map bounds
	int drawMap(const int idToStartDrawing = -1); // Draws map, returns object ID of first wall drawn (for client synching purposes)
	void spawnPowerUp(); // Spawns PowerUp
	void getSpiralLocations(const int xMin, const int xMax, const int yMin, const int yMax); // Populates spiralEdgeLocations vector (of enclosing walls)
	void spawnEnclosingWalls(); // Spawns enclosing walls (shrinks map)	
};