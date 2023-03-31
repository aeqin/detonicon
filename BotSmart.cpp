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
#include "BotSmart.h"
#include "Bomb.h"
#include "Wall.h"
#include "HPContainer.h"
#include "EventVictor.h"
#include "ManagerPerson.h"
#include "ManagerBomb.h"
#include "ManagerPowerUp.h"
#include "GameStart.h"

BotSmart::BotSmart(PersonDisplay* display, std::string name, const int id, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite)
{
	// Set object type.
	setType("Person->BotSmart");

	// Need to update rate control each step.
	registerInterest(df::STEP_EVENT);

	registerInterest(VICTOR_EVENT);

	// Set Variables
	setPersonSharedDefaults(display, name, maxHP, maxBombs, x, y, sprite);
}

// AI Controller
void BotSmart::botSmartAI(int fightVal, int runVal, int exploreVal)
{
	// Determine the current stakes for appropriate behaviors
	// We will calculate the respective distance between the current location and other elements, specifically other players, bombs and power-up
	// Interpretation:
	// (1) Closer to bomb (and in danger): Try to find a nearby safe location, set it as destination, and go there.
	// (2) Closer to other players: Try to approach the nearest player, lay 1 or more bombs if possible, then perform (1) immediately
	// (3) Closer to power-up: Simply reach the power-up.

	// Calculate distance between current location and nearest player
	Person* nearestPlayer = MPer.getClosestPerson(this);
	float distanceToPlayer = MPer.getDistance(this, nearestPlayer);

	// Calculate distance between current location and nearest bomb
	df::Vector currentPosition = this->getPosition();
	df::Vector closestBomb = MPer.getClosestBomb(this);
	float x1 = currentPosition.getX();
	float y1 = currentPosition.getY();
	float x2 = closestBomb.getX();
	float y2 = closestBomb.getY();
	float distanceToBomb = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
	if (MB.getBombCount() == 0) distanceToBomb = 9998; // In case no bomb is available yet

	// Calculate distance between current location and nearest power up
	df::Vector closestPowerUp = MPer.getClosestPowerUp(this);
	x2 = closestPowerUp.getX();
	y2 = closestPowerUp.getY();
	float distanceToPowerUp = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
	if (MPow.getPowerCount() == 0) distanceToPowerUp = 9999; // In case no power up is available yet

	if (!flagStandby) {
		// Find the best option
		if (distanceToBomb <= distanceToPlayer && distanceToBomb <= distanceToPowerUp) {
			// Bomb is nearer, so flight
			performRun();
		}
		else if (distanceToPlayer <= distanceToBomb && distanceToPlayer <= distanceToPowerUp) {
			// Player is nearer, so fight
			performFight();
		}
		else performExplore();
	}
	else { // Make bot stand still for a while
		standbyCount++;
		if (standbyCount >= 20) {
			standbyCount = 0;
			flagStandby = false;
		}
	}
}

// Procedure for finding and fighting other player
void BotSmart::performFight()
{
	// Calculate distance between current location and nearest player
	Person* nearestPlayer = MPer.getClosestPerson(this);
	if(nearestPlayer == nullptr) return;

	float targetX = nearestPlayer->getPosition().getX();
	float targetY = nearestPlayer->getPosition().getY();

	// Move is within range, so lay down bomb
	switch (moveToTarget(nearestPlayer->getPosition(), 2)) {
	case 0: // Cannot move
		LM.writeLog("BotSmart::performFight() SmartBot cannot approach nearest player anymore!");
		placeBomb(); // An attempt to switch to run mode and to break obstacles that stop movement
		break;
	case 2: // Move is within range
		LM.writeLog("BotSmart::performFight() SmartBot is near to other player!");
		placeBomb();
		break;
	default:
		LM.writeLog("BotSmart::performFight() SmartBot is approaching nearest player!");
		break;
	}
}

// Procedure for getting Power Up
void BotSmart::performExplore()
{
	// Calculate distance between current location and nearest player
	df::Vector nearestPowerUp = MPer.getClosestPowerUp(this);
	float targetX = nearestPowerUp.getX();
	float targetY = nearestPowerUp.getY();

	// Move is within range, so lay down bomb
	switch (moveToTarget(nearestPowerUp, 0)) {
	case 0: // Cannot move
		LM.writeLog("BotSmart::performFight() SmartBot cannot approach nearest power up anymore!");
		placeBomb(); // An attempt to switch to run mode and to break obstacles that stop movement
		break;
	case 2: // Move is within range
		LM.writeLog("BotSmart::performFight() SmartBot has taken the power up!");
		placeBomb();
		break;
	default:
		LM.writeLog("BotSmart::performFight() SmartBot is approaching nearest power up!");
		break;
	}
}

// Procedure for detecting and running away from explosion
void BotSmart::performRun()
{
	df::Vector playerPos = this->getPosition();
	df::Vector tempPos;
	df::Vector listOfMove[100];
	int count = 0;

	// Attempt to pinpoint a location that is safe to move to
	while (MPer.checkBombDanger(playerPos, 4)) {
		LM.writeLog("BotSmart::performRun() Looking for safe location...");
		// Register position coordinates
		df::Vector awayPosition = MPer.getClosestBomb(this);
		df::Vector attemptPosition = playerPos;
		float curX = attemptPosition.getX();
		float curY = attemptPosition.getY();
		float awayX = awayPosition.getX();
		float awayY = awayPosition.getY();
		bool moveMade = false;

		if (!moveMade && curX < awayX) { // Bomb is on the right side
			tempPos = attemptPosition;
			tempPos.setX(attemptPosition.getX() - 1); // Test going to left
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove, count)) { // Update new possible position
				LM.writeLog("BotSmart::performRun() SmartBot is trying to go left!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curX > awayX) { // Bomb is on the left side
			tempPos = attemptPosition;
			tempPos.setX(attemptPosition.getX() + 1); // Test going to right
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove, count)) { // Update new possible position
				LM.writeLog("BotSmart::performRun() SmartBot is trying to go right!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curY > awayY) { // Bomb is below
			tempPos = attemptPosition;
			tempPos.setY(attemptPosition.getY() - 1); // Test going up
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove, count)) { // Update new possible position
				LM.writeLog("BotSmart::performRun() SmartBot is trying to go up!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curY < awayY) { // Bomb is above
			tempPos = attemptPosition;
			tempPos.setY(attemptPosition.getY() + 1); // Test going down
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove, count)) { // Update new possible position
				LM.writeLog("BotSmart::performRun() SmartBot is trying to go down!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}

		if (moveMade) {
			listOfMove[count] = attemptPosition;
			playerPos = attemptPosition;
			count++;
		}
		else { // Force a move
			switch (rand() % 4) {
			case 0:
				attemptPosition.setX(attemptPosition.getX() + 1);
				listOfMove[count] = attemptPosition;
				playerPos = attemptPosition;
				count++;
				break;
			case 1:
				attemptPosition.setX(attemptPosition.getX() - 1);
				listOfMove[count] = attemptPosition;
				playerPos = attemptPosition;
				count++;
				break;
			case 2:
				attemptPosition.setY(attemptPosition.getY() + 1);
				listOfMove[count] = attemptPosition;
				playerPos = attemptPosition;
				count++;
				break;
			case 3:
				attemptPosition.setY(attemptPosition.getY() - 1);
				listOfMove[count] = attemptPosition;
				playerPos = attemptPosition;
				count++;
				break;
			}
		};
	}

	// If we reach here, that means we find a safe location to move to
	switch (moveToTarget(playerPos, 0)) {
	case 0: // Cannot move
		LM.writeLog("BotSmart::performRun() SmartBot cannot approach nearest safe location anymore! This should not happen...");
		//placeBomb(); // An attempt to break obstacles that stop movement
		break;
	case 2: // Move is within range
		LM.writeLog("BotSmart::performRun() SmartBot is now safe!");
		// Turn on wait flag
		flagStandby = true;
		break;
	default:
		LM.writeLog("BotSmart::performFight() SmartBot is approaching nearest safe location!");
		break;
	}

	// Empty array
	memset(listOfMove, 0, sizeof(listOfMove));
}

// Helper function to check whether a position is in an array of positions
bool BotSmart::hasVisited(df::Vector position, df::Vector* array, int size) {
	for (int i = 0; i < size; i++) {
		if (array[i] == position) return true;
	}
	return false;
}

// Function to control the bot to move towards a specific target from its location
// This function determine the direction to be made for moveXY()
// to prevent random staggering movement caused by randomized values
// Return:
// 0 - cannot make a move
// 1 - move is successfully made
// 2 - current position is already within the bounding radius
int BotSmart::moveToTarget(df::Vector target, int radius)
{
	// Get current location of the bot
	df::Vector currentPosition = this->getPosition();
	float curX = currentPosition.getX();
	float curY = currentPosition.getY();

	// Get target location of the bot
	float tarX = target.getX();
	float tarY = target.getY();

	// Calculate the current distance between current location and target location
	float distance = sqrt(pow((curX - tarX), 2) + pow((curY - tarY), 2));

	// If the distance is within the given radius, return 2 as no move is needed to be made
	if (distance <= radius) return 2;

	Move newMove = { 0, 0 };
	df::Vector testMove;

	// Try to move left
	if (curX < tarX) { 
		testMove = currentPosition;
		testMove.setX(curX + 1);
		// Make sure the path is clear
		if (!isThereSolidAt(testMove)) {
			newMove = { 1, 0 };
		}
	}
	// Try to move right
	if (curX > tarX) {
		testMove = currentPosition;
		testMove.setX(curX - 1);
		// Make sure the path is clear
		if (!isThereSolidAt(testMove)) {
			newMove = { -1, 0 };
		}
	}
	// Try to move up
	if (curY > tarY) {
		testMove = currentPosition;
		testMove.setY(curY - 1);
		// Make sure the path is clear
		if (!isThereSolidAt(testMove)) {
			newMove = { 0, -1 };
		}
	}
	// Try to move down
	if (curY < tarY) {
		testMove = currentPosition;
		testMove.setY(curY + 1);
		// Make sure the path is clear
		if (!isThereSolidAt(testMove)) {
			newMove = { 0, 1 };
		}
	}

	// Make sure we can make a new move, else return 0 indicating move fails
	if (newMove.x == 0 && newMove.y == 0) return 0;
	else { // Move can be made, so make it
		moveXY(newMove.x, newMove.y);
		prevMove = newMove;
	}
	return 1;
}

// Handle event.
// Return 0 if ignored, else 1.
int BotSmart::eventHandler(const df::Event *p_e)
{
	if(GS.getIsGameOver() == true) return 1;
	if(isVictor == true) return 0;

	if (p_e->getType() == df::STEP_EVENT)
	{
		botSmartAI(0, 0, 0);
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
void BotSmart::step()
{
	incrementCounters();
}
