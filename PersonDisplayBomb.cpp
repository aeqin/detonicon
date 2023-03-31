// PersonDisplayBomb.cpp

// Engine includes
#include "DisplayManager.h"
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"

// Game includes
#include "PersonDisplay.h"
#include "Wall.h"

#include "PersonDisplayBomb.h"

PersonDisplayBomb::PersonDisplayBomb(bool isOnServer, df::Vector pos, const int bombs, const int maxChars) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos, bombs, maxChars);
	}
}

void PersonDisplayBomb::initOnServer(df::Vector pos, const int bombs, const int maxChars)
{
	// Set up variables
	this->numBombs = bombs; // Current # bombs to display
	this->posBuffer = "";
	for(int i = 0; i < maxChars; i++)
	{
		this->posBuffer += " "; // Max bomb characters
	}

	// Set object type
	setType("PersonDisplayBomb");

	std::string str = std::string(BOMB_STRING);
	for(int i = 0; i < bombs; i++)
	{
		str += "O";
	}
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str); // "# Bombs: "
	setColor(df::WHITE);
	setBorder(false);
	setAltitude(0);
	setDrawValue(false);

	if(bombs == 0)
	{
		setColor(df::Color::RED);
	}

	// Set position
	setPosition(df::Vector(pos.getX() + 5, pos.getY())); // Adjust x pos depending on maxChars
}

PersonDisplayBomb::PersonDisplayBomb(const int x, const int y, const int bombs, const int maxChars) // Constructor
{
	// Set up variables
	this->numBombs = bombs; // Current # bombs to display
	this->posBuffer = "";
	for(int i = 0; i < maxChars; i++)
	{
		this->posBuffer += " "; // Max bomb characters
	}

	// Set object type
	setType("PersonDisplayBomb");

	std::string str = std::string(BOMB_STRING);
	for(int i = 0; i < bombs; i++)
	{
		str += "O";
	}
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str); // "# Bombs: "
	setColor(df::WHITE);
	setBorder(false);
	setAltitude(0);
	setDrawValue(false);

	if(bombs == 0)
	{
		setColor(df::Color::RED);
	}

	// Set position
	setPosition(df::Vector((float) x + 5, (float) y)); // Adjust x pos depending on maxChars
}

int PersonDisplayBomb::getBombID() // Gets bombID
{
	return this->bombID;
}

void PersonDisplayBomb::setBombID(const int bombID) // Sets bombID
{
	this->bombID = bombID;
}

void PersonDisplayBomb::updateBombs(const int newBombs) // Updates display string to reflect new number of bombs
{
	this->numBombs = newBombs;
	std::string str = std::string(BOMB_STRING);

	if(this->numBombs == 0)
	{
		setColor(df::Color::RED);
		str += "RELOADING";
	}
	else
	{
		setColor(df::Color::WHITE);
	}

	for(int i = 0; i < this->numBombs; i++)
	{
		str += "O";
	}
	str = PersonDisplay::sizeStr(str, this->posBuffer);

	setViewString(str); // "# Bombs: "

	// Send message to every Client to update HUD
	if(NR.getServer())
	{
		std::string msg = "DISPLAYBOMB UPDATE|";
		msg += std::to_string(this->getId());
		msg += "|";
		msg += str;
		const char* msgToSend = msg.c_str(); // 'DISPLAYBOMB UPDATE'|bombId|updatedStr message

		LM.writeLog("PersonDisplayBomb::updateBombs(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);
	}
}