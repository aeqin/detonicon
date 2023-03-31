// PersonDisplay.h

#pragma once

// Engine includes
#include "Event.h"
#include "ViewObject.h"

// Game includes
#include "NetworkRole.h"
#include "Person.h"
#include "PersonDisplayBomb.h"
#include "PersonDisplayDead.h"
#include "PersonDisplayHealth.h"
#include "PersonDisplayName.h"
#include "PersonDisplayPower.h"
#include "PersonDisplaySprite.h"

class PersonDisplay : public df::ViewObject
{
private:
	df::Vector originalPos; // Original position of PersonDisplay before shake
	int shakeCounterCurr = 0; // Current count until shake again
	int shakeCounterMax = 0; // Max count of waiting until shaking
	bool shakeLeft = false; // Is there more shake left?

	Person* personToFollow; // Person whose stats PersonDisplay displays

	PersonDisplayBomb* pdb; // Displays Person's Bombs
	PersonDisplayHealth* pdh; // Displays Person's HP
	PersonDisplayDead* pdd; // Displays X if Person is dead
	PersonDisplayName* pdn; // Displays Person's name
	PersonDisplayPower* pdp; // Displays Person's Bomb power
	PersonDisplaySprite* pds; // Displays Person's sprite

	// Friend classes to make use of "static std::string sizeStr(std::string str, std::string posBuffer);" function
	friend class PersonDisplayBomb;
	friend class PersonDisplayHealth;
	friend class PersonDisplayName;
	friend class PersonDisplayPower;
	friend class PersonDisplaySprite;

public:
	PersonDisplay(bool isOnServer, df::Vector pos, Person* person); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos, Person* person); // Create Object on Server

	PersonDisplay(const int x, const int y, Person* person); // Constructor

	void draw(); // Draws PersonDisplay's sprite
	int eventHandler(const df::Event *p_e); // Handles events
	void updateBombs(const int bombs); // Updates PersonDisplayBomb
	void updateHealth(const int newHealth); // Updates PersonDisplayHealth
	void updatePowers(Person* personToFollow); // Updates PersonDisplayPower
	void moveTo(const int x, const int y); // Moves PersonDisplay
	void shake(df::Vector dist); // Shakes PersonDisplay
	void setDisplaySprite(std::string spriteName); // Sets PersonDisplay's sprite
	static std::string sizeStr(std::string str, std::string posBuffer); // Properly sizes string str based on allowed max character length, may concatnate

	// Custom Networking
	std::string serialize(bool all = false); // Custom serialize for variables
	int deserialize(std::string str); // Custom deserialize for variables
	void setShakes(const int max, const int curr); // Sets PersonDisplay's current shake and max shake values
};