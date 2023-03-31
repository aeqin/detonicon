// PersonDisplaySprite.h

#pragma once

// Engine includes
#include "Event.h"
#include "ViewObject.h"

class PersonDisplaySprite : public df::ViewObject
{
private:
	df::Vector originalPos;

public:
	PersonDisplaySprite(const int x, const int y, df::Sprite* sprite); // Constructor
	PersonDisplaySprite(const int x, const int y, std::string spriteName); // Constructor

	void draw(); // Draws PersonDisplaySprite's sprite
};