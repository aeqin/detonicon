// PlayerPointToHighlight.cpp

// System includes
#include <stdlib.h>	// for rand()

// Engine includes
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "EventPersonDeath.h"
#include "PlayerPointToHighlight.h"

PlayerPointToHighlight::PlayerPointToHighlight(const float x, const float y, df::Object* leader)
{
	// Set up variables
	this->leader = leader;
	this->leaderID = leader->getId();

	// Set object type
	setType("PlayerPointToHighlight");

	// Register with events
	registerInterest(df::STEP_EVENT); // To update position and time until destruction
	registerInterest(PERSONDEATH_EVENT); // On leader death, destroy this

	// Set solidness
	setSolidness(df::SPECTRAL);

	// Set altitude
	setAltitude(df::MAX_ALTITUDE);

	// Set up sprite
	df::Sprite *p_temp_sprite = RM.getSprite("playerPointToHighlight");
	if(!p_temp_sprite)
		LM.writeLog("PlayerPointToHighlight::PlayerPointToHighlight(): Warning! Sprite '%s' not found", "playerPointToHighlight");
	else
	{
		setSprite(p_temp_sprite);
		setTransparency();
		setSpriteSlowdown(2);
	}

	// Set position
	WM.moveObject(this, df::Vector(x, y));
}

int PlayerPointToHighlight::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == PERSONDEATH_EVENT)
	{
		if(((EventPersonDeath*) p_e)->getID() == this->leaderID)
		{
			WM.markForDelete(this);
		}
		return 1;
	}
	if(p_e->getType() == df::STEP_EVENT)
	{
		if(getSpriteIndex() == getSprite()->getFrameCount() - 1)
		{
			this->countTillDeath++;
			if(this->countTillDeath > 4)
			{
				WM.markForDelete(this);
			}
			setSpriteIndex(0);
		}

		if(this->leader != nullptr)
		{
			df::Vector leaderPos(leader->getPosition().getX(), leader->getPosition().getY());
			WM.moveObject(this, leaderPos);
		}
		return 1;
	}
	return 0;
}

