// PlayerPointToHighlight.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "Object.h"

class PlayerPointToHighlight : public df::Object
{
private:
	int countTillDeath = 0; // Used to determine when to destroy PlayerPointToHighlight
	df::Object* leader; // Object to follow
	int leaderID; // ID of Person to follow

public:
	PlayerPointToHighlight(const float x, const float y, df::Object* leader); // Constructor

	int eventHandler(const df::Event *p_e); // Handles events
};