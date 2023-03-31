// HPContainer.cpp

// System includes
#include <cmath>

// Engine includes
#include "DisplayManager.h"
#include "EventOut.h"
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "HPContainer.h"
#include "NetworkRole.h"
#include "Person.h"

#include "MiniHP.h"

HPContainer::HPContainer(df::Object* leader, const int xAway, const int yAway, const int maxHP) // Constructor
{
	// Set up variables
	this->xAway = xAway; // X margin HPContainer follows leader
	this->yAway = yAway; // Y margin HPContainer follows leader
	this->maxHP = maxHP; // Max HP HPContainer can display
	this->currHP = maxHP; // Current HP being displayed

	df::Vector pos;
	pos.setX(leader->getPosition().getX() - round((float) maxHP / 2));
	pos.setY(leader->getPosition().getY() + yAway);
	this->leader = leader; // Object that HPContainer follows
	for(int i = 0; i < maxHP; i++) // Spawn each individual HP that makes up an "HP bar"
	{
		pos.setX(pos.getX() + 1);
		children.push_back(new MiniHP(true, leader, (int) (pos.getX() - leader->getPosition().getX()), yAway)); // Vector of MiniHP that HPContainer controls
		this->currHP = i;
	}

	// Set solidness
	setSolidness(df::SPECTRAL);
}

void HPContainer::takeDamage(const int damage) // Updates current HP being displayed
{
	if(NR.getClient()) return; // Only update damage on Server

	int damageLeft = damage;
	if(damageLeft < 0)
	{
		while(damageLeft < 0)
		{
			currHP++;
			damageLeft++;
			children.at(currHP)->setVisible(true);
		}
	}
	else
	{
		while(damageLeft > 0 && currHP >= 0)
		{
			children.at(currHP)->setVisible(false);
			currHP--;
			damageLeft--;
		}
		if(currHP < 0)
		{
			for(int i = 0; i < maxHP; i++)
			{
				children.at(i)->setActive(false); // Kill miniHP
			}
			((Person*) leader)->die(); // Kill Person
			WM.markForDelete(this); // Kill Self
		}
	}
}