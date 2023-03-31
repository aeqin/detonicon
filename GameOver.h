// GameOver.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "ViewObject.h"

// Game includes
#include "Person.h"

class GameOver : public df::ViewObject
{
private:
	int time_to_live; // Time until GameOver destroys itself
	int nextFireworkStep; // Time until GameOver spawns another firework graphic
public:
	GameOver(int victorObjectID); // Constructor

	void draw(); // Draws the GameOver sprite
	int eventHandler(const df::Event *p_e); // Handles events
	void step(); // Updates GameOver time to live per frame & time until spawning another firework graphic
	bool canDeleteThisObject(df::Object* obj); // Returns whether or not this Object should be deleted on game over
	void fireworksShow(); // Spawns a firework graphic

	~GameOver(); // Destructor
};