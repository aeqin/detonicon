// PersonDisplayPower.cpp

// Engine includes
#include "DisplayManager.h"
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"

// Game includes
#include "PersonDisplay.h"

#include "PersonDisplayPower.h"

PersonDisplayPower::PersonDisplayPower(bool isOnServer, df::Vector pos, const int maxChars) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos, maxChars);
	}
}

void PersonDisplayPower::initOnServer(df::Vector pos, const int maxChars)
{
	// Set up variables
	this->posBuffer = "";
	for(int i = 0; i < maxChars; i++)
	{
		this->posBuffer += " "; // Max power characters
	}

	// Set object type
	setType("PersonDisplayPower");

	std::string str = std::string(POWER_STRING);
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str);
	setColor(df::WHITE);
	setBorder(false);
	setAltitude(0);
	setDrawValue(false);

	// Set position
	setPosition(df::Vector(pos.getX() + 5.0f, pos.getY())); // Adjust x pos depending on maxChars
}

PersonDisplayPower::PersonDisplayPower(const int x, const int y, const int maxChars) // Constructor
{
	// Set up variables
	this->posBuffer = "";
	for(int i = 0; i < maxChars; i++)
	{
		this->posBuffer += " "; // Max power characters
	}

	std::string str = std::string(POWER_STRING);
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str);
	setColor(df::WHITE);
	setBorder(false);
	setAltitude(0);
	setDrawValue(false);

	// Set position
	setPosition(df::Vector((float) x + 5, (float) y)); // Adjust x pos depending on maxChars
}

void PersonDisplayPower::updatePowers(Person* person) // Updates display string to reflect new PowerUps
{
	this->bombPower = person->getBombPower();
	this->numBombs = person->getMaxBombs();
	int moveSlow = person->getMoveSlowdown();

	if(moveSlow == 3) this->moveSpeed = 1;
	else if(moveSlow == 2) this->moveSpeed = 2;
	else if(moveSlow == 1) this->moveSpeed = 3;

	std::string str = std::string(POWER_STRING);
	str += std::string("O:" + std::to_string(this->numBombs));
	str += std::string(" #:" + std::to_string(this->bombPower));
	str += std::string(" SPD:x" + std::to_string(this->moveSpeed));
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str);
}

void PersonDisplayPower::updatePowers(const int bombPwr, const int numBombs, const int moveSpeed) // Updates display string to reflect new PowerUps
{
	this->bombPower = bombPwr;
	this->numBombs = numBombs;
	this->moveSpeed = moveSpeed;

	std::string str = std::string(POWER_STRING);
	str += std::string("O:" + std::to_string(this->numBombs));
	str += std::string(" #:" + std::to_string(this->bombPower));
	str += std::string(" SPD:x" + std::to_string(this->moveSpeed));
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str);
}

std::string PersonDisplayPower::serialize(bool all) // Custom serialize for variables
{
	// Do main serialize from parent
	std::string s = Object::serialize(all);

	// Add Wall-specific attribute
	s += ("posBuffer:" + std::to_string(this->posBuffer.length()) + ",");
	s += ("bombPower:" + std::to_string(this->bombPower) + ",");
	s += ("numBombs:" + std::to_string(this->numBombs) + ",");
	s += ("moveSpeed:" + std::to_string(this->moveSpeed) + ",");

	// Return full serialization
	return s;
}

int PersonDisplayPower::deserialize(std::string str) // Custom deserialize for variables
{
	// Do main deserialize from parent
	Object::deserialize(str);

	// Look for socket index
	std::string parseForStr = df::match("", "posBuffer");
	{
		this->posBuffer = "";
		for(int i = 0; i < stoi(parseForStr); i++)
		{
			this->posBuffer += " "; // Max power characters
		}
	}
	bool isChanged = false; // Check if a power up display needs to be displayed
	parseForStr = df::match("", "bombPower");
	if(!parseForStr.empty())
	{
		this->bombPower = stoi(parseForStr);
		isChanged = true;
	}
	parseForStr = df::match("", "numBombs");
	if(!parseForStr.empty())
	{
		this->numBombs = stoi(parseForStr);
		isChanged = true;
	}
	parseForStr = df::match("", "moveSpeed");
	if(!parseForStr.empty())
	{
		this->moveSpeed = stoi(parseForStr);
		isChanged = true;
	}

	if(isChanged == true)
	{
		setBorder(false);
		setDrawValue(false);
		updatePowers(this->bombPower, this->numBombs, this->moveSpeed);
	}
	return 0;
}

