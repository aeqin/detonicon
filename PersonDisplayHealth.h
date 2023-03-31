// PersonDisplayHealth.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "Event.h"
#include "ViewObject.h"

#define HP_STRING "Health: " // String to be displayed

class PersonDisplayHealth : public df::ViewObject
{
private:
	int currHealth; // Current HP to display
	std::string posBuffer; // String of constant max char length to display correctly regardless of size
	int healthID; // Used to identify corresponding Client PersonDisplayHealth from Server's

public:
	PersonDisplayHealth(bool isOnServer, df::Vector pos, const int HP, const int maxChars); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos, const int bombs, const int maxChars); // Create Object on Server

	PersonDisplayHealth(const int x, const int y, const int HP, const int maxChars); // Constructor
	
	// Variable getters
	int getCurrHealth(); // Returns current HP being displayed
	int getHealthID(); // Returns healthID

	// Variable setters
	void setHealthID(const int healthID); // Sets healthID

	void updateHealth(const int newHealth); // Updates display string to reflect new HP
};