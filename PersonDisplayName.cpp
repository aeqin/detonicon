// PersonDisplayName.cpp

// Engine includes
#include "DisplayManager.h"
#include "EventStep.h"
#include "LogManager.h"
#include "ResourceManager.h"

// Game includes
#include "PersonDisplay.h"

#include "PersonDisplayName.h"

PersonDisplayName::PersonDisplayName(const int x, const int y, std::string name, const int maxChars) // Constructor
{
	// Set up variables
	this->posBuffer = "";
	for(int i = 0; i < maxChars; i++)
	{
		this->posBuffer += " "; // Max DisplayName characters
	}

	std::string str = std::string(NAME_STRING) + name;
	str = PersonDisplay::sizeStr(str, posBuffer);

	setViewString(str); // "Name: "
	setColor(df::WHITE);
	setBorder(false);
	setAltitude(0);
	setDrawValue(false);

	// Set position
	setPosition(df::Vector((float) x + 5, (float) y)); // Adjust x pos depending on maxChars
}

