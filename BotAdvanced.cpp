//
// BotAdvanced.cpp
//

// Engine includes.
#include "EventStep.h"
#include "EventView.h"
#include "GameManager.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes.
#include "BotAdvanced.h"
#include "Bomb.h"
#include "Wall.h"
#include "HPContainer.h"
#include "EventVictor.h"
#include "ManagerBomb.h"
#include "ManagerPerson.h"
#include "ManagerPowerUp.h"
#include "NetworkRole.h"

BotAdvanced::BotAdvanced(PersonDisplay* display, std::string name, const int id, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite, int braveValInput, int smartValInput)
{
	// Set Variables
	setPersonSharedDefaults(display, name, maxHP, maxBombs, x, y, sprite);

	// Set object type.
	setType("Person->BotAdvanced");

	// Need to update rate control each step.
	registerInterest(df::STEP_EVENT);

	registerInterest(VICTOR_EVENT);

	// Randomize Brave and Smart values for personalities
	//braveVal = rand() % 5;
	//smartVal = rand() % 5;
	braveVal = braveValInput;
	smartVal = smartValInput;
}

// AI Controller
void BotAdvanced::botAdvancedAI()
{
	// Determine the current stakes for appropriate behaviors
	// We will calculate the respective distance between the current location and other elements, specifically other players, bombs and power-up
	// Interpretation:
	// (1) Closer to bomb (and in danger): Try to find a nearby safe location, set it as destination, and go there.
	// (2) Closer to other players: Try to approach the nearest player, lay 1 or more bombs if possible, then perform (1) immediately
	// (3) Closer to power-up: Simply reach the power-up.

	// Advanced AI will allow the bots to have varying personality based on the fightVal/runVal/exploreVal used)

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

	// Booleans for raising availability flags
	bool canFight = false;
	bool canRun = false;
	bool canExplore = true;

	if (!flagStandby) {
		// Find the best option
		if (distanceToBomb <= distanceToPlayer && distanceToBomb <= distanceToPowerUp) {
			// Bomb is nearer, so flight
			canRun = true;
		}
		else if (distanceToPlayer <= distanceToBomb && distanceToPlayer <= distanceToPowerUp) {
			// Player is nearer, so fight
			canFight = true;
		}
	}
	else { // Make bot stand still for a while
		canExplore = false;
		standbyCount++;
		if (standbyCount >= 20) {
			standbyCount = 0;
			flagStandby = false;
		}
	}

	// Values are between 0 and 4, and are determined only once to maintain personalities
	// 0 - Low in attribute, 4 - High in attribute
	bool actionChosen = false;
	if (canFight) {
		if (rand() % 5 <= braveVal){
			performFight();
			actionChosen = true;
		}
		else {
			performWander();
			actionChosen = true;
		}
	}
	else if (canRun) {
		performRun();
		actionChosen = true;
	}
	if (!actionChosen && canExplore) performExplore();
}

// Procedure for finding and fighting other player
void BotAdvanced::performFight()
{
	// Calculate distance between current location and nearest player
	Person* nearestPlayer = MPer.getClosestPerson(this);
	float targetX = nearestPlayer->getPosition().getX();
	float targetY = nearestPlayer->getPosition().getY();

	// Move is within range, so lay down bomb
	switch (moveToTarget(nearestPlayer->getPosition(), 2)) {
	case 0: // Cannot move
		LM.writeLog("BotAdvanced::performFight() SmartBot cannot approach nearest player anymore!");
		placeBomb(); // An attempt to switch to run mode and to break obstacles that stop movement
		break;
	case 2: // Move is within range
		LM.writeLog("BotAdvanced::performFight() SmartBot is near to other player!");
		placeBomb();
		break;
	default:
		LM.writeLog("BotAdvanced::performFight() SmartBot is approaching nearest player!");
		break;
	}
}

// Procedure for finding and getting close to other player
void BotAdvanced::performWander()
{
	// Calculate distance between current location and nearest player
	Person* nearestPlayer = MPer.getRandomPerson(this);
	float targetX = nearestPlayer->getPosition().getX();
	float targetY = nearestPlayer->getPosition().getY();

	// Move is within range, so lay down bomb
	switch (moveToTarget(nearestPlayer->getPosition(), 5)) {
	case 0: // Cannot move
		LM.writeLog("BotAdvanced::performFight() SmartBot cannot approach nearest player anymore!");
		placeBomb(); // An attempt to switch to run mode and to break obstacles that stop movement
		break;
	case 2: // Move is within range
		LM.writeLog("BotAdvanced::performFight() SmartBot is near to other player!");
		break;
	default:
		LM.writeLog("BotAdvanced::performFight() SmartBot is approaching nearest player!");
		break;
	}
}

// Procedure for getting Power Up
void BotAdvanced::performExplore()
{
	// Calculate distance between current location and nearest player
	df::Vector nearestPowerUp = MPer.getClosestPowerUp(this);
	float targetX = nearestPowerUp.getX();
	float targetY = nearestPowerUp.getY();

	// Move is within range, so lay down bomb
	switch (moveToTarget(nearestPowerUp, 0)) {
	case 0: // Cannot move
		LM.writeLog("BotAdvanced::performFight() SmartBot cannot approach nearest power up anymore!");
		placeBomb(); // An attempt to switch to run mode and to break obstacles that stop movement
		break;
	case 2: // Move is within range
		LM.writeLog("BotAdvanced::performFight() SmartBot has taken the power up!");
		placeBomb();
		break;
	default:
		LM.writeLog("BotAdvanced::performFight() SmartBot is approaching nearest power up!");
		break;
	}
}

// Procedure for detecting and running away from explosion
void BotAdvanced::performRun()
{
	df::Vector playerPos = this->getPosition();
	df::Vector tempPos;
	std::vector<df::Vector> listOfMove;
	int count = 0;

	// Attempt to pinpoint a location that is safe to move to
	while (MPer.checkBombDanger(playerPos, smartVal) || isThereDangerAt(playerPos) || isNearWall(playerPos)) {
		LM.writeLog("BotAdvanced::performRun() Looking for safe location...");
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
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove)) { // Update new possible position
				LM.writeLog("BotAdvanced::performRun() SmartBot is trying to go left!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curX > awayX) { // Bomb is on the left side
			tempPos = attemptPosition;
			tempPos.setX(attemptPosition.getX() + 1); // Test going to right
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove)) { // Update new possible position
				LM.writeLog("BotAdvanced::performRun() SmartBot is trying to go right!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curY > awayY) { // Bomb is below
			tempPos = attemptPosition;
			tempPos.setY(attemptPosition.getY() - 1); // Test going up
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove)) { // Update new possible position
				LM.writeLog("BotAdvanced::performRun() SmartBot is trying to go up!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curY < awayY) { // Bomb is above
			tempPos = attemptPosition;
			tempPos.setY(attemptPosition.getY() + 1); // Test going down
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove)) { // Update new possible position
				LM.writeLog("BotAdvanced::performRun() SmartBot is trying to go down!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}

		if (moveMade) {
			listOfMove.push_back(attemptPosition);
			playerPos = attemptPosition;
			count++;
		}
		else { // Force a move
			switch (rand() % 4) {
			case 0:
				attemptPosition.setX(attemptPosition.getX() + 1);
				listOfMove.push_back(attemptPosition);
				playerPos = attemptPosition;
				count++;
				break;
			case 1:
				attemptPosition.setX(attemptPosition.getX() - 1);
				listOfMove.push_back(attemptPosition);
				playerPos = attemptPosition;
				count++;
				break;
			case 2:
				attemptPosition.setY(attemptPosition.getY() + 1);
				listOfMove.push_back(attemptPosition);
				playerPos = attemptPosition;
				count++;
				break;
			case 3:
				attemptPosition.setY(attemptPosition.getY() - 1);
				listOfMove.push_back(attemptPosition);
				playerPos = attemptPosition;
				count++;
				break;
			}
		};
	}

	// If we reach here, that means we find a safe location to move to
	switch (moveToTarget(playerPos, 0)) {
	case 0: // Cannot move
		LM.writeLog("BotAdvanced::performRun() SmartBot cannot approach nearest safe location anymore! This should not happen...");
		//placeBomb(); // An attempt to break obstacles that stop movement
		break;
	case 2: // Move is within range
		LM.writeLog("BotAdvanced::performRun() SmartBot is now safe!");
		// Turn on wait flag
		flagStandby = true;
		break;
	default:
		LM.writeLog("BotAdvanced::performFight() SmartBot is approaching nearest safe location!");
		break;
	}

	// Empty array
	//memset(listOfMove, 0, sizeof(listOfMove));
}

// Helper function to check whether a position is in an array of positions
bool BotAdvanced::hasVisited(df::Vector position, std::vector<df::Vector> &array) 
{
	if(std::find(array.begin(), array.end(), position) != array.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Function to control the bot to move towards a specific target from its location
// This function determine the direction to be made for moveXY()
// to prevent random staggering movement caused by randomized values
// Return:
// 0 - cannot make a move
// 1 - move is successfully made
// 2 - current position is already within the bounding radius
int BotAdvanced::moveToTarget(df::Vector target, int radius)
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

	// Make path using pathfinding in Dragonfly
	this->pathTo(target);

	// Obtain path
	std::vector<df::Vector> currentPath = this->getPath().getPath();
	
	// Get next move from path
	if (!currentPath.empty()) {
		tarX = currentPath.at(1).getX();
		tarY = currentPath.at(1).getY();
	}

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
int BotAdvanced::eventHandler(const df::Event *p_e)
{
	if(isVictor == true) return 0;

	if (p_e->getType() == df::STEP_EVENT)
	{
		step();
		return 1;
	}

	if(p_e->getType() == VICTOR_EVENT)
	{
		this->isVictor = true;
		return 1;
	}

	if (p_e->getType() == df::PATH_EVENT) {
		//LM.writeLog("BotAdvanced::PathFinding is ongoing!");
		return 1;
	}

	// If get here, have ignored this event.
	return 0;
}

// Decrease rate restriction counters.
void BotAdvanced::step()
{
	if(!NR.getServer() && this->getId() == NR.getClient()->getPersonObjectID()) // Only think if Client
	{
		botAdvancedAI();
	}

	incrementCounters();
}

std::string BotAdvanced::serialize(bool all) // Custom serialize for variables
{
	// Do main serialize from parent
	std::string s = Person::serialize(all);

	// Add BotAdvanced-specific attribute
	s += ("brave_val:" + std::to_string(this->braveVal) + ",");
	s += ("smart_val:" + std::to_string(this->smartVal) + ",");
	s += ("prev_move_x:" + std::to_string(this->prevMove.x) + ",");
	s += ("prev_move_y:" + std::to_string(this->prevMove.y) + ",");
	if(this->flagStandby == true) s += ("flagStandby:t,");
	else s += ("flagStandby:f,");
	s += ("standbyCount:" + std::to_string(this->standbyCount) + ",");

	// Return full serialization
	return s;
}

int BotAdvanced::deserialize(std::string str) // Custom deserialize for variables
{
	// Do main deserialize from parent
	Person::deserialize(str);

	std::string parseForStr = df::match("", "brave_val");
	if(!parseForStr.empty())
	{
		this->braveVal = stoi(parseForStr);
	}
	parseForStr = df::match("", "smart_val");
	if(!parseForStr.empty())
	{
		this->smartVal = stoi(parseForStr);
	}

	// Set prev move
	int x;
	int y;
	parseForStr = df::match("", "prev_move_x");
	if(!parseForStr.empty())
	{
		x = stoi(parseForStr);
	}
	parseForStr = df::match("", "prev_move_y");
	if(!parseForStr.empty())
	{
		y = stoi(parseForStr);
	}
	this->prevMove = {x, y};

	parseForStr = df::match("", "flagStandby");
	if(!parseForStr.empty())
	{
		if(parseForStr.at(0) == 't') this->flagStandby = true;
		else this->flagStandby = false;
	}
	parseForStr = df::match("", "standbyCount");
	if(!parseForStr.empty())
	{
		this->standbyCount = stoi(parseForStr);
	}

	return 0;
}
