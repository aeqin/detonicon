// ManagerBomb.cpp

// System includes
#include <iostream>

// Engine includes
#include "GameManager.h"
#include "LogManager.h"
#include "WorldManager.h"

// Game includes
#include "EventBomb.h"
#include "EventMiniExplosion.h"
#include "EventVictor.h"
#include "NetworkRole.h"

#include "ManagerBomb.h"

ManagerBomb::ManagerBomb() // Constructor, private for singleton
{
	// Set object type
	setType("ManagerBomb");

	// Register with events
	registerInterest(BOMB_EVENT); // To update Bomb vector
	registerInterest(MINIEXPLOSION_EVENT); // To update MiniExplosion vector
	registerInterest(VICTOR_EVENT); // To clear vectors when game ends
}

static ManagerBomb* p_bombManager = nullptr;
ManagerBomb& ManagerBomb::getInstance() // Get the one and only instance of BombManager
{
	if(p_bombManager == nullptr)
	{
		p_bombManager = new ManagerBomb();
	}

	return *p_bombManager;
}

int ManagerBomb::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == VICTOR_EVENT)
	{
		if(this->bombs.empty() == false)
		{
			this->bombs.clear();
		}
		return 1;
	}
	if(p_e->getType() == BOMB_EVENT)
	{
		LM.writeLog("BombManager::eventHandler - Received BOMB_EVENT, bomb size is %d", this->bombs.size());
		bool isSpawn = ((EventBomb*) p_e)->getIsSpawn();
		Bomb* p_bomb = ((EventBomb*) p_e)->getBomb();

		if(isSpawn == true) // Bomb just spawned
		{
			this->bombs.push_back(p_bomb);
		}
		else // Bomb just exploded
		{
			for(int i = 0; i < this->bombs.size(); i++)
			{
				if(this->bombs.at(i) == p_bomb)
				{
					if(this->bombs.size() > 0)
					{
						this->bombs.erase(this->bombs.begin() + i);
					}
					break;
				}
			}
		}

		return 1;
	}

	if(p_e->getType() == MINIEXPLOSION_EVENT)
	{
		LM.writeLog("BombManager::eventHandler - Received MINIEXPLOSION_EVENT, mini size is %d", this->miniExplosions.size());
		bool isSpawn = ((EventMiniExplosion*) p_e)->getIsSpawn();
		MiniExplosion* p_mini = ((EventMiniExplosion*) p_e)->getMiniExplosion();

		if(isSpawn == true) // MiniExplosion just spawned
		{
			this->miniExplosions.push_back(p_mini);
		}
		else // MiniExplosion just exploded
		{
			for(int i = 0; i < this->miniExplosions.size(); i++)
			{
				if(this->miniExplosions.at(i) == p_mini)
				{
					if(this->miniExplosions.size() > 0)
					{
						this->miniExplosions.erase(this->miniExplosions.begin() + i);
					}
					break;
				}
			}
		}

		return 1;
	}

	return 0;
}

int ManagerBomb::getBombCount() // Gets number of Bombs in game
{
	return (int) this->bombs.size();
}

int ManagerBomb::getMiniExplosionCount() // Gets number of MiniExplosions in game
{
	return (int) this->miniExplosions.size();
}

std::vector<Bomb*> ManagerBomb::getBombs() // Gets vector of all Bombs in game
{
	return this->bombs;
}

std::vector<MiniExplosion*> ManagerBomb::getMiniExplosions() // Gets vector of all MiniExplosions in game
{
	return this->miniExplosions;
}