// PersonDisplayDead.cpp

// Engine includes
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"

// Game includes
#include "PersonDisplayDead.h"

PersonDisplayDead::PersonDisplayDead(const int x, const int y) // Constructor
{
	// Set sprite
	df::Sprite *p_temp_sprite;
	p_temp_sprite = RM.getSprite("personDisplayDead");

	if(!p_temp_sprite)
		LM.writeLog("PersonDisplayDead::PersonDisplayDead(): Warning! Sprite '%s' not found", "personDisplayDead");
	else
	{
		setSprite(p_temp_sprite);
		setTransparency();
	}

	// Set position
	df::Vector pos((float) x, (float) y);
	setPosition(pos);

	// Set Altitude
	setAltitude(0);
}

void PersonDisplayDead::draw() // Draws PersonDisplayDead
{
	setAltitude(0);
	df::Object::draw();
}
