// Player.cpp

// Engine includes
#include "EventCollision.h"
#include "EventStep.h"
#include "EventView.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "Bomb.h"
#include "EventVictor.h"
#include "GameStart.h"
#include "HPContainer.h"
#include "NetworkRole.h"
#include "PersonDisplay.h"
#include "Player.h"

Player::Player(PersonDisplay* display, std::string name, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite) // Constructor
{
	// Set up variables
	setPersonSharedDefaults(display, name, maxHP, maxBombs, x, y, sprite);

	// Set object type
	setType("Person->Player");

	// Register with events
	registerInterest(df::KEYBOARD_EVENT); // To accept keyboard input
	registerInterest(df::STEP_EVENT); // To update counters
	registerInterest(VICTOR_EVENT); // To stop accepting keyboard input
}

// Return 0 if ignored, else 1
int Player::eventHandler(const df::Event *p_e) // Handles events
{
	if(GS.getIsGameOver() == true) return 1;
	if(this->isVictor == true) return 1;

	if(p_e->getType() == df::KEYBOARD_EVENT && isMouseInWindow() == true && (!NR.getServer() && this->getId() == NR.getClient()->getPersonObjectID()))
	{
		const df::EventKeyboard *p_keyboard_event = dynamic_cast <const df::EventKeyboard *> (p_e);
		kbd(p_keyboard_event);
		return 1;
	}

	if(p_e->getType() == df::STEP_EVENT)
	{
		step();
		return 1;
	}

	if(p_e->getType() == VICTOR_EVENT)
	{
		this->isVictor = true;
		return 1;
	}

	if(p_e->getType() == df::COLLISION_EVENT)
	{
		const df::EventCollision *p_collision_event = dynamic_cast <df::EventCollision const *> (p_e);

		df::Object* obj1 = p_collision_event->getObject1();
		std::string type1 = obj1->getType();
		df::Object* obj2 = p_collision_event->getObject2();
		std::string type2 = obj2->getType();

		return 1;
	}

	// If get here, have ignored this event.
	return 0;
}

void Player::kbd(const df::EventKeyboard *p_keyboard_event) // Handles keyboard input
{
	switch(p_keyboard_event->getKey())
	{
	case df::Keyboard::W: // UP
	case df::Keyboard::UPARROW:
		if(p_keyboard_event->getKeyboardAction() == df::KEY_DOWN)
			up = true;
		else if(p_keyboard_event->getKeyboardAction() == df::KEY_RELEASED)
			up = false;
		break;
	case df::Keyboard::S: // DOWN
	case df::Keyboard::DOWNARROW:
		if(p_keyboard_event->getKeyboardAction() == df::KEY_DOWN)
			down = true;
		else if(p_keyboard_event->getKeyboardAction() == df::KEY_RELEASED)
			down = false;
		break;
	case df::Keyboard::A: // LEFT
	case df::Keyboard::LEFTARROW:
		if(p_keyboard_event->getKeyboardAction() == df::KEY_DOWN)
			left = true;
		else if(p_keyboard_event->getKeyboardAction() == df::KEY_RELEASED)
			left = false;
		break;
	case df::Keyboard::D: // RIGHT
	case df::Keyboard::RIGHTARROW:
		if(p_keyboard_event->getKeyboardAction() == df::KEY_DOWN)
			right = true;
		else if(p_keyboard_event->getKeyboardAction() == df::KEY_RELEASED)
			right = false;
		break;
	case df::Keyboard::SPACE: // PLACE BOMB
		if(p_keyboard_event->getKeyboardAction() == df::KEY_PRESSED)
			placeBomb();
		break;
	case df::Keyboard::ESCAPE: // QUIT
		if(p_keyboard_event->getKeyboardAction() == df::KEY_PRESSED)
			if(!NR.getServer() && !GM.getGameOver())
			{
				NR.getClient()->writeResults(); // Write results of game to a file

				// Tell Server that it (the Client) is ready to shut down
				std::string msg = "SHUTDOWN";
				const char* msgToSend = msg.c_str(); // 'SHUTDOWN' message
				LM.writeLog("GameOver::~GameOver(): sending custom message: '%s'", msgToSend);
				NR.getClient()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend);

				GM.setGameOver();
			}
		break;
	};

	// Decides how to move player
	int moveX = 0;
	int moveY = 0;
	if(up) moveY = -1;
	if(down) moveY = 1;
	if(left) moveX = -1;
	if(right) moveX = 1;
	if(moveX != 0 || moveY != 0)
	{
		// Only accept move commands when not dead & game is not over
		if(this->currHP > 0 && GS.getIsGameOver() == false)
		{
			moveXY(moveX, moveY);
		}
	}
	
	return;
}

void Player::step() // Increments counters each frame
{
	incrementCounters();
}