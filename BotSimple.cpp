//
// BotSimple.cpp
//

// Engine includes.
#include "EventStep.h"
#include "EventView.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes.
#include "BotSimple.h"
#include "Bomb.h"
#include "Wall.h"
#include "HPContainer.h"
#include "EventVictor.h"
#include "ManagerPerson.h"
#include "PersonDisplay.h"
#include "GameStart.h"

BotSimple::BotSimple(PersonDisplay* display, std::string name, const int id, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite)
{
	// Set object type.
	setType("Person->BotSimple");

	// Need to update rate control each step.
	registerInterest(df::STEP_EVENT);

	registerInterest(VICTOR_EVENT);

	// Set Variables
	setPersonSharedDefaults(display, name, maxHP, maxBombs, x, y, sprite);

	this->prevMoveX = 0;
	this->prevMoveY = 0;
}

// An intermediate Bot AI that find nearest player and approach it to lay bomb
void BotSimple::botSimpleAI()
{
	Person* target = MPer.getClosestPerson(this);
	// Only place bomb when the bot is near to the other player
	if (MPer.getDistance(this, target) <= 2) {
		if (rand() % 10 == 0) placeBomb();
	}
	// Monitoring movement
	int moveX = 0;
	int moveY = 0;
	int lORr = 0; // 0 - left, 1 - right
	int aORb = 0; // 0 - above, 1 - below
	if (rand() % 10 > 2) // Greater chance to repeat last move, so there's slightly less jittering
	{
		moveX = this->prevMoveX;
		moveY = this->prevMoveY;
	}
	else {
		df::Vector startPos = this->getPosition();
		df::Vector endPos = target->getPosition();
		float x1 = startPos.getX();
		float y1 = startPos.getY();
		float x2 = endPos.getX();
		float y2 = endPos.getY();
		// Check the location of the bot with regards to the nearest player
		if (x1 < x2) {
			aORb = 0;
		}
		else aORb = 1;
		if (y1 < y2) {
			lORr = 0;
		}
		else lORr = 1;

		// Case by case in 4 quadrants
		if (aORb == 0 && lORr == 0) { // above left
			if (rand() % 2 == 0) moveX += 1;
			else moveY += 1;
		}
		if (aORb == 0 && lORr == 1) { // above right
			if (rand() % 2 == 0) moveX += 1;
			else moveY -= 1;
		}
		if (aORb == 1 && lORr == 0) { // below left
			if (rand() % 2 == 0) moveX -= 1;
			else moveY += 1;
		}
		if (aORb == 1 && lORr == 1) { // below right
			if (rand() % 2 == 0) moveX -= 1;
			else moveY -= 1;
		}
	}
	
	if (moveX != 0 || moveY != 0)
	{
		moveXY(moveX, moveY);
	}
}


// Handle event.
// Return 0 if ignored, else 1.
int BotSimple::eventHandler(const df::Event *p_e)
{
	if(GS.getIsGameOver() == true) return 1;
	if(isVictor == true) return 0;

	if (p_e->getType() == df::STEP_EVENT)
	{
		botSimpleAI();
		step();
		return 1;
	}

	if(p_e->getType() == VICTOR_EVENT)
	{
		this->isVictor = true;
		return 1;
	}

	// If get here, have ignored this event.
	return 0;
}

// Decrease rate restriction counters.
void BotSimple::step()
{
	incrementCounters();
}
