// ManagerPowerUp.cpp

// System includes
#include <iostream>

// Engine includes
#include "GameManager.h"
#include "LogManager.h"
#include "WorldManager.h"

// Game includes
#include "EventPowerUp.h"
#include "EventVictor.h"
#include "NetworkRole.h"

#include "ManagerPowerUp.h"

ManagerPowerUp::ManagerPowerUp() // Constructor, private for singleton
{
	// To update power vector
	registerInterest(POWER_EVENT);

	// To clear power vector when game ends
	registerInterest(VICTOR_EVENT);

	setType("PowerUpManager");
}

static ManagerPowerUp* p_powManager = nullptr;
ManagerPowerUp& ManagerPowerUp::getInstance() // Get the one and only instance of the ManagerPerson
{
	if(p_powManager == nullptr)
	{
		p_powManager = new ManagerPowerUp();
	}

	return *p_powManager;
}

int ManagerPowerUp::getPowerCount() // Get number of powers in game
{
	return (int) this->powers.size();
}

std::vector<PowerUp*> ManagerPowerUp::getPowers() // Get vector of all powers in game
{
	return this->powers;
}

int ManagerPowerUp::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == VICTOR_EVENT)
	{
		if(this->powers.empty() == false)
		{
			this->powers.clear();
		}
		return 1;
	}
	if(p_e->getType() == POWER_EVENT)
	{
		bool isSpawn = ((EventPowerUp*) p_e)->getIsSpawn();
		PowerUp* p_pow = ((EventPowerUp*) p_e)->getPower();

		if(isSpawn == true) // Powerup just spawned
		{
			this->powers.push_back(p_pow);
		}
		else // Powerup just died
		{
			for(int i = 0; i < this->powers.size(); i++)
			{
				if(this->powers.at(i) == p_pow)
				{
					if(this->powers.size() > 0)
					{
						this->powers.erase(this->powers.begin() + i);
					}
					break;
				}
			}
		}

		return 1;
	}

	return 0;
}