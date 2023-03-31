// utilities

// Engine includes
#include "Config.h"
#include "DisplayManager.h"

// Game includes
#include "utilities.h"

bool isMouseInWindow() // Is the mouse currently in the window
{
	sf::Vector2i mousePos = sf::Mouse::getPosition(*DM.getWindow());
	if(mousePos.x < df::Config::getInstance().getWindowHorizontalPixels() && mousePos.x > 0 && mousePos.y < df::Config::getInstance().getWindowVerticalPixels() && mousePos.y > 0)
	{
		return true;
	}
	return false;
}
