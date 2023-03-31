// PersonDisplayHealth.cpp

// Engine includes
#include "DisplayManager.h"
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"

// Game includes
#include "PersonDisplay.h"

#include "PersonDisplayHealth.h"

PersonDisplayHealth::PersonDisplayHealth(bool isOnServer, df::Vector pos, const int HP, const int maxChars) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos, HP, maxChars);
	}
}

void PersonDisplayHealth::initOnServer(df::Vector pos, const int HP, const int maxChars)
{
	// Set up variables
	this->currHealth = HP; // Current HP to display
	this->posBuffer = "";
	for(int i = 0; i < maxChars; i++)
	{
		this->posBuffer += " "; // Max HP characters
	}

	// Set object type
	setType("PersonDisplayHealth");

	std::string str = std::string(HP_STRING);
	for(int i = 0; i < HP; i++)
	{
		str += ">";
	}
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str); // "Health: "
	setColor(df::WHITE);
	setBorder(false);
	setAltitude(0);
	setDrawValue(false);

	if(HP <= 1)
	{
		setColor(df::Color::RED);
	}

	// Set position
	setPosition(df::Vector(pos.getX() + 5.0f, pos.getY())); // Adjust x pos depending on maxChars
}

PersonDisplayHealth::PersonDisplayHealth(const int x, const int y, const int HP, const int maxChars) // Constructor
{
	// Set up variables
	this->currHealth = HP; // Current HP to display
	this->posBuffer = "";
	for(int i = 0; i < maxChars; i++)
	{
		this->posBuffer += " "; // Max HP characters
	}

	// Set object type
	setType("PersonDisplayHealth");

	std::string str = std::string(HP_STRING);
	for(int i = 0; i < HP; i++)
	{
		str += ">";
	}
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str); // "Health: "
	setColor(df::WHITE);
	setBorder(false);
	setAltitude(0);
	setDrawValue(false);

	if(HP <= 1)
	{
		setColor(df::Color::RED);
	}

	// Set position
	setPosition(df::Vector((float) x + 5, (float) y)); // Adjust x pos depending on maxChars
}

int PersonDisplayHealth::getCurrHealth() // Returns current HP being displayed
{
	return this->currHealth;
}

int PersonDisplayHealth::getHealthID() // Gets healthID
{
	return this->healthID;
}

void PersonDisplayHealth::setHealthID(const int healthID) // Sets healthID
{
	this->healthID = healthID;
}

void PersonDisplayHealth::updateHealth(const int newHealth) // Updates display string to reflect new HP
{
	this->currHealth = newHealth;
	std::string str = std::string(HP_STRING);
	for(int i = 0; i < newHealth; i++)
	{
		str += ">";
	}
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str); // "Health: "
	if(newHealth <= 1)
	{
		setColor(df::Color::RED);
	}
	else
	{
		setColor(df::Color::WHITE);
	}

	// Send message to every Client to update HUD
	if(NR.getServer())
	{
		std::string msg = "DISPLAYHEALTH UPDATE|";
		msg += std::to_string(this->getId());
		msg += "|";
		msg += str;
		const char* msgToSend = msg.c_str(); // 'DISPLAYHEALTH UPDATE'|healthId|updatedStr message

		LM.writeLog("PersonDisplayHealth::updateHealth(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);
	}
}
