// EventNuke.h
#pragma once

// System includes
#include <string>

// Game includes
#include "Event.h"

const std::string NUKE_EVENT = "NUKE_EVENT";

class EventNuke : public df::Event
{
public:
	EventNuke(); // Constructor
};
