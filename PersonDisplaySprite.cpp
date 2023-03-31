// PersonDisplaySprite.cpp

// System includes
#include <iostream>

// Engine includes
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"

// Game includes
#include "PersonDisplaySprite.h"

PersonDisplaySprite::PersonDisplaySprite(const int x, const int y, df::Sprite* sprite) // Constructor
{
	// Set up variables
	this->originalPos = df::Vector((float) x, (float) y);

	// Set sprite
	setSprite(sprite);

	// Register with events
	registerInterest(df::STEP_EVENT); // To update position in case 
	
	// Set position
	setPosition(this->originalPos);

	// Set Altitude
	setAltitude(0);
}

PersonDisplaySprite::PersonDisplaySprite(const int x, const int y, std::string spriteName) // Constructor
{
	// Set up variables
	this->originalPos = df::Vector((float) x, (float) y);

	// Set sprite
	df::Sprite *p_temp_sprite = RM.getSprite(spriteName);
	if(!p_temp_sprite)
		LM.writeLog("PersonDisplaySprite::PersonDisplaySprite(): Warning! Sprite '%s' not found", spriteName);
	else
	{
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
	}

	// Register with events
	registerInterest(df::STEP_EVENT); // To update position in case

	// Set position
	setPosition(this->originalPos);

	// Set Altitude
	setAltitude(0);
}

void PersonDisplaySprite::draw() // Draws PersonDisplaySprite's sprite
{
	setAltitude(0);
	df::Object::draw();
}