// PersonDisplayName.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "Event.h"
#include "ViewObject.h"

#define NAME_STRING "Name: " // String to be displayed

class PersonDisplayName : public df::ViewObject
{
private:
	std::string posBuffer; // String of constant max char length to display correctly regardless of size

public:
	PersonDisplayName(const int x, const int y, std::string name, const int maxChars); // Constructor
};