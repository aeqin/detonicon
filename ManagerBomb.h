// ManagerBomb.h

#pragma once

// System includes
#include <string>
#include <vector>

// Game includes
#include "Bomb.h"
#include "MiniExplosion.h"

#define MB ManagerBomb::getInstance() // Two-letter acronym for easier access to manager

class ManagerBomb : df::Object
{
private:
	ManagerBomb(); // Constructor, private for singleton
	std::vector<Bomb*> bombs; // Vector of all Bombs in game
	std::vector<int> bombObjectIds; // Vector of all object ids of Bombs in game
	std::vector<MiniExplosion*> miniExplosions; // Vector of all MiniExplosions in game

public:
	static ManagerBomb &getInstance(); // Get the one and only instance of BombManager

	int eventHandler(const df::Event *p_e); // Handles event

	// Variable getters
	int getBombCount(); // Gets number of Bombs in game
	int getMiniExplosionCount(); // Gets number of MiniExplosions in game
	std::vector<Bomb*> getBombs(); // Gets vector of all Bombs in game
	std::vector<MiniExplosion*> getMiniExplosions(); // Gets vector of all MiniExplosions in game
};