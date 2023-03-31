// HPContainer.h

#pragma once

// System includes
#include <vector>

// Game includes
#include "MiniHP.h"

class HPContainer : public df::Object
{
private:
	df::Object* leader; // Object that HPContainer follows
	std::vector<MiniHP*> children; // Vector of MiniHP that HPContainer controls

	int xAway; // X margin HPContainer follows leader
	int yAway; // Y margin HPContainer follows leader
	int maxHP; // Max HP HPContainer can display
	int currHP; // Current HP being displayed

public:
	HPContainer(df::Object* leader, const int xAway, const int yAway, const int maxHP); // Constructor

	void takeDamage(const int damage); // Updates current HP being displayed
};