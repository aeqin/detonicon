// PersonDisplayDead.h

#pragma once

// Engine includes
#include "Event.h"
#include "ViewObject.h"

class PersonDisplayDead : public df::ViewObject
{
public:
	PersonDisplayDead(const int x, const int y); // Constructor

	void draw(); // Draws PersonDisplayDead
};