//
// BotPathfind.cpp
//

// Engine includes.
#include "EventStep.h"
#include "EventView.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"
#include "ManagerMap.h"
#include "DisplayManager.h"


// Game includes.
#include "BotPathfind.h"
#include "Bomb.h"
#include "Wall.h"
#include "HPContainer.h"
#include "EventVictor.h"
#include "ManagerBomb.h"
#include "ManagerPerson.h"
#include "ManagerPowerUp.h"
#include "NetworkRole.h"
#include "GameStart.h"

BotPathfind::BotPathfind(PersonDisplay* display, std::string name, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite, int braveInput, int smartInput)
{
	// Set Variables
	setPersonSharedDefaults(display, name, maxHP, maxBombs, x, y, sprite);

	// Set object type.
	setType("Person->BotPathfind");

	// Need to update rate control each step.
	registerInterest(df::STEP_EVENT);

	registerInterest(VICTOR_EVENT);

	// Register Brave and Smart Values
	braveVal = braveInput;
	smartVal = smartInput;

	// Initialize standby status
	checkStandby = true; // At the start, it should be able to move
	standbyCount = 0; // At the start, count for standby is 0
	clearPathStack();

	// Set previous position to be the starting location on initialization
	previousPos = this->getPosition();
}

// Control the behavior of the bot
void BotPathfind::botAI()
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
	if (this == nearestPlayer) distanceToPlayer = 9999; // Technically, it has no other players and cannot target itself

	// Store variables for current location
	df::Vector currentPosition = this->getPosition();
	float x1 = currentPosition.getX();
	float y1 = currentPosition.getY();

	// Store variables for closest bomb
	df::Vector closestBomb = MPer.getClosestBomb(this);
	float x2 = closestBomb.getX();
	float y2 = closestBomb.getY();

	// Store variables for closest power up
	df::Vector closestPowerUp = MPer.getClosestPowerUp(this);
	float x3 = closestPowerUp.getX();
	float y3 = closestPowerUp.getY();

	// Calulcate distance to nearest bomb
	float distanceToBomb = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
	if (MB.getBombCount() == 0) distanceToBomb = 9999; // In case no bomb is available yet

	// Calculate distance to nearest power up
	float distanceToPowerUp = sqrt(pow((x1 - x3), 2) + pow((y1 - y3), 2));
	if (MPow.getPowerCount() == 0) distanceToPowerUp = 9999; // In case no power up is available yet

	// Booleans for controlling flags for various actions
	bool canFight = false;
	bool canFlight = false;
	bool canPower = false;

	LM.writeLog("BotPathfind::botAI() Distance to nearest player: %f", distanceToPlayer);
	LM.writeLog("BotPathfind::botAI() Distance to nearest bomb: %f", distanceToBomb);
	LM.writeLog("BotPathfind::botAI() Distance to nearest PowerUp: %f", distanceToPowerUp);

	// Determine best action for bot
	// Make sure that the bot can move first
	if (MPer.checkBombDanger(currentPosition, smartVal)) {
		LM.writeLog("BotPathfind::botAI() Bot has to run away since it's in danger!");
		canFlight = true;
		// Reset standby count forcefully
		standbyCount = 0;
		checkStandby = false;
	}
	if (!checkStandby) {
		if (distanceToBomb <= distanceToPlayer && distanceToBomb <= distanceToPowerUp && distanceToBomb != 9999) {
			LM.writeLog("BotPathfind::botAI() Bot has the ability to flight");
			canFlight = true;
		}
		if (distanceToPlayer < distanceToBomb && distanceToPlayer < distanceToPowerUp && distanceToPlayer != 9999) {
			LM.writeLog("BotPathfind::botAI() Bot has the ability to fight");
			canFight = true;
		}
		if (distanceToPowerUp < distanceToPlayer && distanceToPowerUp < distanceToBomb && distanceToPowerUp != 9999) {
			LM.writeLog("BotPathfind::botAI() Bot has the ability to power-up");
			canPower = true;
		}
	}
	else {
		// If we reach here, it means that the bot is in standby
		standbyCount++;
		LM.writeLog("BotPathfind::botAI() Bot is in standby. Standby Count: %d", standbyCount);
		if (standbyCount >= 120) {
			standbyCount = 0;
			checkStandby = false;
		}
	}

	// Check its current location, if it's in an invalid spot then attempt to struggle
	if (!checkValid(currentPosition)) {
		/*if (!checkValid(df::Vector(currentPosition.getX() + 1, currentPosition.getY()))) {
			moveXY(1, 0);
		}
		else if (!checkValid(df::Vector(currentPosition.getX() - 1, currentPosition.getY()))) {
			moveXY(-1, 0);
		}
		else if (!checkValid(df::Vector(currentPosition.getX(), currentPosition.getY() + 1))) {
			moveXY(0, 1);
		}
		else if (!checkValid(df::Vector(currentPosition.getX(), currentPosition.getY() - 1))) {
			moveXY(0, -1);
		}*/
		moveXY(rand() % 3 - 1, rand() % 3 - 1);
	}

	// Values are between 0 and 4, and are determined only once to maintain personalities
	// 0 - Low in attribute, 4 - High in attribute
	bool actionChosen = false;
	if (canFight) {
		if (rand() % 5 <= braveVal) {
			LM.writeLog("BotPathfind::botAI() Action taken: Fight");
			moveOffensive();
		}
	}
	else if (canPower) {
		LM.writeLog("BotPathfind::botAI() Action taken: Get PowerUp");
		moveSelfish();
	}
	else if (canFlight) {
		LM.writeLog("BotPathfind::botAI() Action taken: Run to safety");
		moveDefensive();
	}
}

// Make an offensive move (find the target to attack)
void BotPathfind::moveOffensive()
{
	// Calculate distance between current location and nearest player
	Person* nearestPlayer = MPer.getClosestPerson(this);
	if (nearestPlayer != nullptr) {
		float targetX = nearestPlayer->getPosition().getX();
		float targetY = nearestPlayer->getPosition().getY();
		LM.writeLog("BotPathfind::moveOffensive() Nearest Players: %f, %f", targetX, targetY);
	}
	else return; // No nearest player found
	
	// Make path using pathfinding
	if (path.empty()) { 
		// No path is in process, so make new path
		aStarSearchNoObstruction(this->getPosition(), nearestPlayer->getPosition());
		if (path.empty()) { //First pathfinding attempt returns nothing within the timeframe, account for wall
			aStarSearchWithObstruction(this->getPosition(), nearestPlayer->getPosition());
			LM.writeLog("BotPathfind::moveOffensive() Bot is surrounded by breakable wall, proceed to move with obstacle accountable.");
		}
		else {
			LM.writeLog("BotPathfind::moveOffensive() Bot can reach the target freely.");
		}
	}

	// Move is within range, so lay down bomb
	switch (followPath(nearestPlayer->getPosition(), 2)) {
	case 0: // No path can be generated towards that enemy
		LM.writeLog("BotPathfind::moveOffensive() Enemy player cannot be reached!");
		//placeBomb();
		
		break;
	case 2: // Move is within range
		LM.writeLog("BotPathfind::moveOffensive() Enemy player has been targetted!");
		placeBomb();
		checkStandby = true; // It should stand still now
		break;
	default:
		LM.writeLog("BotPathfind::moveOffensive() Finding Enemy in process...");
		break;
	}
}

// Make a move for self (power up)
void BotPathfind::moveSelfish()
{
	// Calculate distance between current location and nearest player
	df::Vector nearestPowerUp = MPer.getClosestPowerUp(this);
	float targetX = nearestPowerUp.getX();
	float targetY = nearestPowerUp.getY();
	LM.writeLog("BotPathfind::moveSelfish() Nearest Powerup: (%f, %f)", targetX, targetY);

	// Make path using pathfinding
	if (path.empty()) {
		aStarSearchNoObstruction(this->getPosition(), nearestPowerUp);
		if (path.empty()) { //First pathfinding attempt returns nothing within the timeframe, account for wall
			aStarSearchWithObstruction(this->getPosition(), nearestPowerUp);
		}
	}

	// Move is within range, so lay down bomb
	switch (followPath(nearestPowerUp, 0)) {
	case 0: // Cannot move
		LM.writeLog("BotPathfind::moveSelfish() Nearest Powerup cannot be reached!");
		//placeBomb();
		break;
	case 2: // Move is within range
		LM.writeLog("BotPathfind::moveSelfish() Powerup has been obtained!");
		checkStandby = true; // It should stand still now
		break;
	default:
		LM.writeLog("BotPathfind::moveSelfish() Finding Powerup in process...");
		break;
	}
}

void BotPathfind::moveDefensive()
{
	df::Vector playerPos = this->getPosition();
	df::Vector tempPos;
	std::vector<df::Vector> listOfMove;
	int count = 0;

	// Attempt to pinpoint a location that is safe to move to
	while (MPer.checkBombDanger(playerPos, smartVal) || isThereDangerAt(playerPos) || isNearWall(playerPos)) {
		LM.writeLog("BotPathfind::moveDefensive() Looking for safe location...");
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
			tempPos.setX(attemptPosition.getX() - 2); // Test going to left
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove)) { // Update new possible position
				LM.writeLog("BotPathfind::moveDefensive() AI is trying to go left!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curX > awayX) { // Bomb is on the left side
			tempPos = attemptPosition;
			tempPos.setX(attemptPosition.getX() + 2); // Test going to right
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove)) { // Update new possible position
				LM.writeLog("BotPathfind::moveDefensive() AI is trying to go right!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curY > awayY) { // Bomb is below
			tempPos = attemptPosition;
			tempPos.setY(attemptPosition.getY() - 1); // Test going up
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove)) { // Update new possible position
				LM.writeLog("BotPathfind::moveDefensive() AI is trying to go up!");
				attemptPosition = tempPos;
				moveMade = true;
			}
		}
		if (!moveMade && curY < awayY) { // Bomb is above
			tempPos = attemptPosition;
			tempPos.setY(attemptPosition.getY() + 1); // Test going down
			if (!isThereSolidAt(tempPos) && !hasVisited(tempPos, listOfMove)) { // Update new possible position
				LM.writeLog("BotPathfind::moveDefensive() AI is trying to go down!");
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
				attemptPosition.setX(attemptPosition.getX() + 2);
				listOfMove.push_back(attemptPosition);
				playerPos = attemptPosition;
				count++;
				break;
			case 1:
				attemptPosition.setX(attemptPosition.getX() - 2);
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

	// Make path using pathfinding
	if (path.empty()) {
		aStarSearchNoObstruction(this->getPosition(), playerPos);
	}

	// If we reach here, that means we find a safe location to move to
	switch (followPath(playerPos, 0)) {
	case 0: // Cannot move
		LM.writeLog("BotPathfind::moveDefensive() Bot cannot locate safe location! This should not happen...");
		//placeBomb(); // An attempt to break obstacles that stop movement
		break;
	case 2: // Move is within range
		LM.writeLog("BotPathfind::moveDefensive() Bot is now safe!");
		checkStandby = true; // It should stand still now
		break;
	default:
		LM.writeLog("BotPathfind::moveDefensive() Bot is approaching nearest safe location!");
		break;
	}
}

// Function to control the bot to move towards a specific target from its location
// This function determine the direction to be made for moveXY()
// to prevent random staggering movement caused by randomized values
// Return:
// 0 - cannot make a move
// 1 - move is successfully made
// 2 - current position is already within the bounding radius
int loopBreakCnt = 0;
int BotPathfind::followPath(df::Vector target, int radius)
{
	// Get current location of the bot
	df::Vector currentPosition = this->getPosition();
	float curX = currentPosition.getX();
	float curY = currentPosition.getY();
	LM.writeLog("BotPathfind::followPath() Current Position: (%f, %f)", curX, curY);

	// Get target location of the bot
	float tarX = target.getX();
	float tarY = target.getY();

	// Calculate the current distance between current location and target location
	float distance = sqrt(pow((curX - tarX), 2) + pow((curY - tarY), 2));

	// If the distance is within the given radius, return 2 as no move is needed to be made
	if (distance <= radius) return 2;
	
	// Initialize direction determinator
	// 0: Left, 1: Right, 2: Up, 3: Down
	int direction = 0; // Default

	// Make sure there is something in the path
	if (path.empty()) {
		LM.writeLog("BotPathfind::followPath() There is no more path to follow!");
		return 0;
	}
	else {
		// Draw current path for debugging
		drawPath();

		// Get the next position from stack
		df::Vector nextPos = path.top();
		path.pop();
		if (nextPos.getX() == curX && nextPos.getY() == curY) {
			LM.writeLog("BotPathfind::followPath() Next position is the same as current position, hence skip.");
			return 1;
		}

		if (nextPos.getX() < curX && nextPos.getY() == curY) {
			direction = 0; // LEFT
		} 
		else if (nextPos.getX() > curX && nextPos.getY() == curY) {
			direction = 1; // RIGHT
		}
		else if (nextPos.getX() == curX && nextPos.getY() < curY) {
			direction = 2; // UP
		}
		else if (nextPos.getX() == curX && nextPos.getY() > curY) {
			direction = 3; // DOWN
		}
		else {
			if (nextPos.getX() > curX && direction == 0) {
				if (nextPos.getY() < curY) 
					direction = 2; // UP
				else direction = 3; // DOWN
			}
			else if (nextPos.getX() < curX && direction == 1) {
				if (nextPos.getY() < curY)
					direction = 2; // UP
				else direction = 3; // DOWN
			}
			else if (nextPos.getY() > curY && direction == 2) {
				if (nextPos.getX() < curX)
					direction = 0; // LEFT
				else direction = 1; // RIGHT
			}
			else if (nextPos.getY() < curY && direction == 3) {
				if (nextPos.getX() < curX)
					direction = 0; // LEFT
				else direction = 1; // RIGHT
			}
		}

		switch (direction) {
		case 0: // LEFT
			LM.writeLog("BotPathfind::followPath() Moving Left...");
			moveXY(-1, 0);
			break;
		case 1: // RIGHT
			LM.writeLog("BotPathfind::followPath() Moving Right...");
			moveXY(1, 0);
			break;
		case 2: // UP
			LM.writeLog("BotPathfind::followPath() Moving Up...");
			moveXY(0, -1);
			break;
		case 3: // DOWN
			LM.writeLog("BotPathfind::followPath() Moving Down...");
			moveXY(0, 1);
			break;
		}
		LM.writeLog("BotPathfind::followPath() Previous Position: (%f, %f)", previousPos.getX(), previousPos.getY());
		LM.writeLog("BotPathfind::followPath() New Position: (%f, %f)", this->getPosition().getX(), this->getPosition().getY());
		// Confirm that a successful move is made by comparing with previous position
		if (previousPos == this->getPosition()) {
			// Move was made but position doesn't change, meaning there is an obstacle
			LM.writeLog("BotPathfind::followPath() No movement is made, probably because of obstacle!");
			path.push(nextPos); // Since no movement is made, node is returnd to stack
			loopBreakCnt++;
			if (loopBreakCnt > 10) {
				LM.writeLog("BotPathfind::followPath() Too many attempt to move pass obstacle, break time!");
				placeBomb();
				loopBreakCnt = 0;
				return 0;
			}
		}
		else {
			previousPos = this->getPosition(); // Update previous position if move is made successfully
			loopBreakCnt = 0;
		}
		return 1;
	}
	
	// Return 0 indicating move fails somehow
	LM.writeLog("BotPathfind::followPath() No movement is made for some odd reason!");
	return 0;
}

// Helper function to check whether a position is in an array of positions
bool BotPathfind::hasVisited(df::Vector position, std::vector<df::Vector> &array)
{
	if (std::find(array.begin(), array.end(), position) != array.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Handle event.
// Return 0 if ignored, else 1.
int BotPathfind::eventHandler(const df::Event *p_e)
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

	// If get here, have ignored this event.
	return 0;
}

// Decrease rate restriction counters.
void BotPathfind::step()
{
	if (!NR.getServer() && this->getId() == NR.getClient()->getPersonObjectID()) // Only think if Client
	{
		if(this->currHP > 0 && GS.getIsGameOver() == false) // Only think if not dead and game is not over
		{
			botAI();
		}
	}

	incrementCounters();
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

// Check validity of a location to make sure it's within bound
bool BotPathfind::checkValid(df::Vector pos) {
	return (pos.getX() >= MM.getXBoundMin()) && (pos.getX() <= MM.getXBoundMax())
		&& (pos.getY() >= MM.getYBoundMin()) && (pos.getY() <= MM.getYBoundMax());
}

// Check passable of a location to make sure players can pass it (either through walking or bombing)
bool BotPathfind::checkPassable(df::Vector pos) {
	if (!checkAccessible(pos)) {
		if (isThereBreakableAt(pos)) return true;
		else return false;
	}
	else return true;
}

// Check accessible of a location to make sure players can walk through it
bool BotPathfind::checkAccessible(df::Vector pos) {
	if (!isThereSolidHereExcludingPerson(pos)) return true;
	else return false;
}

// Check if the players have arrived at the destination
bool BotPathfind::checkArrival(df::Vector pos, df::Vector dest) {
	if (pos == dest) return true;
	else return false;
}

// Helper function to calculate heuristic distance
double BotPathfind::calculateHDist(df::Vector pos, df::Vector dest) {
	return ((double)sqrt((pos.getX() - dest.getX())*(pos.getX() - dest.getX())
					   + (pos.getY() - dest.getY())*(pos.getY() - dest.getY())));
}

// Helper function to clear path stack
void BotPathfind::clearPathStack() {
	// Clear path stack if it's not empty
	if (!path.empty()) {
		while (!path.empty()) path.pop();
	}
}

// Return a path for bot to move along
void BotPathfind::generatePath(cell map[][COL], df::Vector dest) {
	clearPathStack();

	int col = (int) dest.getX();
	int row = (int) dest.getY();
	
	std::stack <df::Vector> foundPath;
	while (!(map[row][col].parent_i == row && map[row][col].parent_j == col)) {
		foundPath.push(df::Vector((float) col, (float) row));
		int temp_row = map[row][col].parent_i;
		int temp_col = map[row][col].parent_j;
		row = temp_row;
		col = temp_col;
	}

	foundPath.push(df::Vector((float) col, (float) row));
	//foundPath.pop(); // Remove the first location
	qualityCheckVal = div((int)foundPath.top().getX(), 2).rem;
	path = foundPath;
}

void BotPathfind::aStarSearchWithObstruction(df::Vector src, df::Vector dest) {
	LM.writeLog("BotPathfind::aStarSearchWithObstruction() Source: %f, %f Destination: %f, %f", src.getX(), src.getY(), dest.getX(), dest.getY());
	// Make sure source is not out of range
	if (!checkValid(src)) {
		LM.writeLog("BotPathfind::aStarSearchWithObstruction() Source is invalid!");
		return;
	}
	// Make sure destination is not out of range
	if (!checkValid(dest)) {
		LM.writeLog("BotPathfind::aStarSearchWithObstruction() Destination is invalid!");
		return;
	}
	// Make sure source and destination are both accessible
	if (!checkAccessible(src) || !checkAccessible(dest)) {
	//if (!checkAccessible(dest)) {
		if (!checkAccessible(src)) LM.writeLog("BotPathfind::aStarSearchWithObstruction() Source is not accessible!");
		if (!checkAccessible(dest)) LM.writeLog("BotPathfind::aStarSearchWithObstruction() Destination is not accessible!");
		return;
	}
	// Make sure source and destination are not at the same spot
	if (checkArrival(src, dest)) {
		LM.writeLog("BotPathfind::aStarSearchWithObstruction() We are already at the destination!");
		return;
	}

	// Initialize closed list, starting with empty list
	bool closedList[ROW][COL];
	memset(closedList, false, sizeof(closedList));

	// Initialize an array of cell details
	cell map[ROW][COL];

	for (int i = 0; i < ROW; i++) {
		for (int j = 0; j < COL; j++) {
			map[i][j].f = FLT_MAX;
			map[i][j].g = FLT_MAX;
			map[i][j].h = FLT_MAX;
			map[i][j].parent_i = -1;
			map[i][j].parent_j = -1;
		}
	}

	// Initialize parameters of the starting node
	int init_j = (int) src.getX();
	int init_i = (int) src.getY();
	map[init_i][init_j].f = 0.0;
	map[init_i][init_j].g = 0.0;
	map[init_i][init_j].h = 0.0;
	map[init_i][init_j].parent_i = init_i;
	map[init_i][init_j].parent_j = init_j;

	/* 
	Create open list having information as
	<f, Vector(i, j)>
	where f = g + h
	and i, j are the row and column index
	*/
	std::vector<Pair> openList;

	// Put the starting cell to the open list
	Pair init;
	init.f = 0.0;
	init.loc = src;
	openList.push_back(init);

	bool foundDest = false;

	// Implement counter to count the number of time loop is executed.
	clock_t begin = clock();
	int loopCount = 0;

	while (!openList.empty()) {
		// If time exceeds specified, break out
		clock_t end = clock();
		float time_spent = (float)((double)(end - begin) / CLOCKS_PER_SEC);
		//LM.writeLog("BotPathfind::aStarSearchWithObstruction() Loop number: %d, Time Spent: %f", loopCount, time_spent);
		if (time_spent > 0.5) break;
		//if (loopCount > 10000) break;

		Pair p = *openList.begin();

		// Remove this vertex from the open list
		openList.erase(openList.begin());

		// Add the vertex to closed list
		init_i = (int) p.loc.getY();
		init_j = (int) p.loc.getX();
		closedList[init_i][init_j] = true;

		/*
		Generating 8 successor of the cell
		   NW  N  NE
		    \  |  /
		     \ | /
		W --- Cell --- E
		     / | \
			/  |  \
		   SW  S  SE

		EDIT: Try with 4 directions instead
		*/

		double fNew, gNew, hNew;
		df::Vector successorNew;

		// NORTH
		// Check validity
		successorNew.setX((float) init_j);
		successorNew.setY((float) init_i - 1);
		// Make sure the cell is valid to proceed
		if (checkValid(successorNew)) {
			
			// Check if arrival
			if (checkArrival(successorNew, dest)) {
				// Set the parent of destination cell
				map[init_i - 1][init_j].parent_i = init_i;
				map[init_i - 1][init_j].parent_j = init_j;
				generatePath(map, dest);
				foundDest = true;
				return;
			}

			// Ignore if the successor is on the closed list
			else if (closedList[init_i - 1][init_j] == false && checkPassable(successorNew)) {
				gNew = map[init_i][init_j].g + 1;
				hNew = calculateHDist(successorNew, dest);
				fNew = gNew + hNew;

				// Check if it is in the open list, OR the path is better
				// to update the open list
				if (map[init_i - 1][init_j].f == FLT_MAX ||
					map[init_i - 1][init_j].f > fNew) {
					Pair tempPair;
					tempPair.f = fNew;
					tempPair.loc = successorNew;
					openList.push_back(tempPair);

					// Update the cell detail on map
					map[init_i - 1][init_j].f = fNew;
					map[init_i - 1][init_j].g = gNew;
					map[init_i - 1][init_j].h = hNew;
					map[init_i - 1][init_j].parent_i = init_i;
					map[init_i - 1][init_j].parent_j = init_j;
				}
			}
		}

		// SOUTH
		// Check validity
		successorNew.setX((float) init_j);
		successorNew.setY((float) init_i + 1);
		// Make sure the cell is valid to proceed
		if (checkValid(successorNew)) {

			// Check if arrival
			if (checkArrival(successorNew, dest)) {
				// Set the parent of destination cell
				map[init_i + 1][init_j].parent_i = init_i;
				map[init_i + 1][init_j].parent_j = init_j;
				generatePath(map, dest);
				foundDest = true;
				return;
			}

			// Ignore if the successor is on the closed list
			else if (closedList[init_i + 1][init_j] == false && checkPassable(successorNew)) {
				gNew = map[init_i][init_j].g + 1;
				hNew = calculateHDist(successorNew, dest);
				fNew = gNew + hNew;

				// Check if it is in the open list, OR the path is better
				// to update the open list
				if (map[init_i + 1][init_j].f == FLT_MAX ||
					map[init_i + 1][init_j].f > fNew) {
					Pair tempPair;
					tempPair.f = fNew;
					tempPair.loc = successorNew;
					openList.push_back(tempPair);

					// Update the cell detail on map
					map[init_i + 1][init_j].f = fNew;
					map[init_i + 1][init_j].g = gNew;
					map[init_i + 1][init_j].h = hNew;
					map[init_i + 1][init_j].parent_i = init_i;
					map[init_i + 1][init_j].parent_j = init_j;
				}
			}
		}

		// EAST
		// Check validity
		successorNew.setX((float) init_j + 1);
		successorNew.setY((float) init_i);
		// Make sure the cell is valid to proceed
		if (checkValid(successorNew)) {

			// Check if arrival
			if (checkArrival(successorNew, dest)) {
				// Set the parent of destination cell
				map[init_i][init_j + 1].parent_i = init_i;
				map[init_i][init_j + 1].parent_j = init_j;
				generatePath(map, dest);
				foundDest = true;
				return;
			}

			// Ignore if the successor is on the closed list
			else if (closedList[init_i][init_j + 1] == false && checkPassable(successorNew)) {
				gNew = map[init_i][init_j].g + 1;
				hNew = calculateHDist(successorNew, dest);
				fNew = gNew + hNew;

				// Check if it is in the open list, OR the path is better
				// to update the open list
				if (map[init_i][init_j + 1].f == FLT_MAX ||
					map[init_i][init_j + 1].f > fNew) {
					Pair tempPair;
					tempPair.f = fNew;
					tempPair.loc = successorNew;
					openList.push_back(tempPair);

					// Update the cell detail on map
					map[init_i][init_j + 1].f = fNew;
					map[init_i][init_j + 1].g = gNew;
					map[init_i][init_j + 1].h = hNew;
					map[init_i][init_j + 1].parent_i = init_i;
					map[init_i][init_j + 1].parent_j = init_j;
				}
			}
		}

		// WEST
		// Check validity
		successorNew.setX((float) init_j - 1);
		successorNew.setY((float) init_i);
		// Make sure the cell is valid to proceed
		if (checkValid(successorNew)) {

			// Check if arrival
			if (checkArrival(successorNew, dest)) {
				// Set the parent of destination cell
				map[init_i][init_j - 1].parent_i = init_i;
				map[init_i][init_j - 1].parent_j = init_j;
				generatePath(map, dest);
				foundDest = true;
				return;
			}

			// Ignore if the successor is on the closed list
			else if (closedList[init_i][init_j - 1] == false && checkPassable(successorNew)) {
				gNew = map[init_i][init_j].g + 1;
				hNew = calculateHDist(successorNew, dest);
				fNew = gNew + hNew;

				// Check if it is in the open list, OR the path is better
				// to update the open list
				if (map[init_i][init_j - 1].f == FLT_MAX ||
					map[init_i][init_j - 1].f > fNew) {
					Pair tempPair;
					tempPair.f = fNew;
					tempPair.loc = successorNew;
					openList.push_back(tempPair);

					// Update the cell detail on map
					map[init_i][init_j - 1].f = fNew;
					map[init_i][init_j - 1].g = gNew;
					map[init_i][init_j - 1].h = hNew;
					map[init_i][init_j - 1].parent_i = init_i;
					map[init_i][init_j - 1].parent_j = init_j;
				}
			}
		}
		loopCount += 1;
	}

	
	// When destination is not found and open list is empty,
	// we conclude that there is no way to reach the destination
	if (foundDest == false) {
		clearPathStack();
		LM.writeLog("BotPathfind::aStarSearchWithObstruction() Cannot reach destination");
	}
	LM.writeLog("BotPathfind::aStarSearchWithObstruction() loopCount: %d", loopCount);
	return;
}

void BotPathfind::aStarSearchNoObstruction(df::Vector src, df::Vector dest) {
	LM.writeLog("BotPathfind::aStarSearchNoObstruction() Source: %f, %f Destination: %f, %f", src.getX(), src.getY(), dest.getX(), dest.getY());
	// Make sure source is not out of range
	if (!checkValid(src)) {
		LM.writeLog("BotPathfind::aStarSearchNoObstruction() Source is invalid!");
		return;
	}
	// Make sure destination is not out of range
	if (!checkValid(dest)) {
		LM.writeLog("BotPathfind::aStarSearchNoObstruction() Destination is invalid!");
		return;
	}
	// Make sure source and destination are both accessible
	if (!checkAccessible(src) || !checkAccessible(dest)) {
	//if (!checkAccessible(dest)) {
		if (!checkAccessible(src)) LM.writeLog("BotPathfind::aStarSearchNoObstruction() Source is not accessible!");
		if (!checkAccessible(dest)) LM.writeLog("BotPathfind::aStarSearchNoObstruction() Destination is not accessible!");
		return;
	}
	// Make sure source and destination are not at the same spot
	if (checkArrival(src, dest)) {
		LM.writeLog("BotPathfind::aStarSearchNoObstruction() We are already at the destination!");
		return;
	}

	// Initialize closed list, starting with empty list
	bool closedList[ROW][COL];
	memset(closedList, false, sizeof(closedList));

	// Initialize an array of cell details
	cell map[ROW][COL];

	for (int i = 0; i < ROW; i++) {
		for (int j = 0; j < COL; j++) {
			map[i][j].f = FLT_MAX;
			map[i][j].g = FLT_MAX;
			map[i][j].h = FLT_MAX;
			map[i][j].parent_i = -1;
			map[i][j].parent_j = -1;
		}
	}

	// Initialize parameters of the starting node
	int init_j = (int) src.getX();
	int init_i = (int) src.getY();
	map[init_i][init_j].f = 0.0;
	map[init_i][init_j].g = 0.0;
	map[init_i][init_j].h = 0.0;
	map[init_i][init_j].parent_i = init_i;
	map[init_i][init_j].parent_j = init_j;

	/*
	Create open list having information as
	<f, Vector(i, j)>
	where f = g + h
	and i, j are the row and column index
	*/
	std::vector<Pair> openList;

	// Put the starting cell to the open list
	Pair init;
	init.f = 0.0;
	init.loc = src;
	openList.push_back(init);

	bool foundDest = false;

	// Implement counter to count the number of time loop is executed.
	clock_t begin = clock();
	int loopCount = 0;

	while (!openList.empty()) {
		// If time exceeds specified, break out
		clock_t end = clock();
		float time_spent = (float)((double)(end - begin) / CLOCKS_PER_SEC);
		//LM.writeLog("BotPathfind::aStarSearchNoObstruction() Loop number: %d, Time Spent: %f", loopCount, time_spent);
		if (time_spent > 0.5) break;
		//if (loopCount > 10000) break;

		Pair p = *openList.begin();

		// Remove this vertex from the open list
		openList.erase(openList.begin());

		// Add the vertex to closed list
		init_i = (int) p.loc.getY();
		init_j = (int) p.loc.getX();
		closedList[init_i][init_j] = true;

		/*
		Generating 8 successor of the cell
		   NW  N  NE
			\  |  /
			 \ | /
		W --- Cell --- E
			 / | \
			/  |  \
		   SW  S  SE

		EDIT: Try with 4 directions instead
		*/

		double fNew, gNew, hNew;
		df::Vector successorNew;

		// NORTH
		// Check validity
		successorNew.setX((float) init_j);
		successorNew.setY((float) init_i - 1);
		// Make sure the cell is valid to proceed
		if (checkValid(successorNew)) {

			// Check if arrival
			if (checkArrival(successorNew, dest)) {
				// Set the parent of destination cell
				map[init_i - 1][init_j].parent_i = init_i;
				map[init_i - 1][init_j].parent_j = init_j;
				generatePath(map, dest);
				foundDest = true;
				return;
			}

			// Ignore if the successor is on the closed list
			else if (closedList[init_i - 1][init_j] == false && checkAccessible(successorNew)) {
				gNew = map[init_i][init_j].g + 1;
				hNew = calculateHDist(successorNew, dest);
				fNew = gNew + hNew;

				// Check if it is in the open list, OR the path is better
				// to update the open list
				if (map[init_i - 1][init_j].f == FLT_MAX ||
					map[init_i - 1][init_j].f > fNew) {
					Pair tempPair;
					tempPair.f = fNew;
					tempPair.loc = successorNew;
					openList.push_back(tempPair);

					// Update the cell detail on map
					map[init_i - 1][init_j].f = fNew;
					map[init_i - 1][init_j].g = gNew;
					map[init_i - 1][init_j].h = hNew;
					map[init_i - 1][init_j].parent_i = init_i;
					map[init_i - 1][init_j].parent_j = init_j;
				}
			}
		}

		// SOUTH
		// Check validity
		successorNew.setX((float) init_j);
		successorNew.setY((float) init_i + 1);
		// Make sure the cell is valid to proceed
		if (checkValid(successorNew)) {

			// Check if arrival
			if (checkArrival(successorNew, dest)) {
				// Set the parent of destination cell
				map[init_i + 1][init_j].parent_i = init_i;
				map[init_i + 1][init_j].parent_j = init_j;
				generatePath(map, dest);
				foundDest = true;
				return;
			}

			// Ignore if the successor is on the closed list
			else if (closedList[init_i + 1][init_j] == false && checkAccessible(successorNew)) {
				gNew = map[init_i][init_j].g + 1;
				hNew = calculateHDist(successorNew, dest);
				fNew = gNew + hNew;

				// Check if it is in the open list, OR the path is better
				// to update the open list
				if (map[init_i + 1][init_j].f == FLT_MAX ||
					map[init_i + 1][init_j].f > fNew) {
					Pair tempPair;
					tempPair.f = fNew;
					tempPair.loc = successorNew;
					openList.push_back(tempPair);

					// Update the cell detail on map
					map[init_i + 1][init_j].f = fNew;
					map[init_i + 1][init_j].g = gNew;
					map[init_i + 1][init_j].h = hNew;
					map[init_i + 1][init_j].parent_i = init_i;
					map[init_i + 1][init_j].parent_j = init_j;
				}
			}
		}

		// EAST
		// Check validity
		successorNew.setX((float) init_j + 1);
		successorNew.setY((float) init_i);
		// Make sure the cell is valid to proceed
		if (checkValid(successorNew)) {

			// Check if arrival
			if (checkArrival(successorNew, dest)) {
				// Set the parent of destination cell
				map[init_i][init_j + 1].parent_i = init_i;
				map[init_i][init_j + 1].parent_j = init_j;
				generatePath(map, dest);
				foundDest = true;
				return;
			}

			// Ignore if the successor is on the closed list
			else if (closedList[init_i][init_j + 1] == false && checkAccessible(successorNew)) {
				gNew = map[init_i][init_j].g + 1;
				hNew = calculateHDist(successorNew, dest);
				fNew = gNew + hNew;

				// Check if it is in the open list, OR the path is better
				// to update the open list
				if (map[init_i][init_j + 1].f == FLT_MAX ||
					map[init_i][init_j + 1].f > fNew) {
					Pair tempPair;
					tempPair.f = fNew;
					tempPair.loc = successorNew;
					openList.push_back(tempPair);

					// Update the cell detail on map
					map[init_i][init_j + 1].f = fNew;
					map[init_i][init_j + 1].g = gNew;
					map[init_i][init_j + 1].h = hNew;
					map[init_i][init_j + 1].parent_i = init_i;
					map[init_i][init_j + 1].parent_j = init_j;
				}
			}
		}

		// WEST
		// Check validity
		successorNew.setX((float) init_j - 1);
		successorNew.setY((float) init_i);
		// Make sure the cell is valid to proceed
		if (checkValid(successorNew)) {

			// Check if arrival
			if (checkArrival(successorNew, dest)) {
				// Set the parent of destination cell
				map[init_i][init_j - 1].parent_i = init_i;
				map[init_i][init_j - 1].parent_j = init_j;
				generatePath(map, dest);
				foundDest = true;
				return;
			}

			// Ignore if the successor is on the closed list
			else if (closedList[init_i][init_j - 1] == false && checkAccessible(successorNew)) {
				gNew = map[init_i][init_j].g + 1;
				hNew = calculateHDist(successorNew, dest);
				fNew = gNew + hNew;

				// Check if it is in the open list, OR the path is better
				// to update the open list
				if (map[init_i][init_j - 1].f == FLT_MAX ||
					map[init_i][init_j - 1].f > fNew) {
					Pair tempPair;
					tempPair.f = fNew;
					tempPair.loc = successorNew;
					openList.push_back(tempPair);

					// Update the cell detail on map
					map[init_i][init_j - 1].f = fNew;
					map[init_i][init_j - 1].g = gNew;
					map[init_i][init_j - 1].h = hNew;
					map[init_i][init_j - 1].parent_i = init_i;
					map[init_i][init_j - 1].parent_j = init_j;
				}
			}
		}
		loopCount += 1;
	}

	// When destination is not found and open list is empty,
	// we conclude that there is no way to reach the destination
	if (foundDest == false) {
		clearPathStack();
		LM.writeLog("BotPathfind::aStarSearchNoObstruction() Cannot reach destination");
	}
	LM.writeLog("BotPathfind::aStarSearchNoObstruction() loopCount: %d", loopCount);
	return;
}

// Function to draw path for debugging purpose
void BotPathfind::drawPath() {
	if (!path.empty()) {
		std::stack<df::Vector> tempStack = path;
		while (!tempStack.empty()) {
			df::Vector pos = tempStack.top();
			LM.writeLog("X: %f, Y: %f", pos.getX(), pos.getY());
			tempStack.pop();
			DM.drawCh(pos, '.', df::RED);
		}
	}
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

std::string BotPathfind::serialize(bool all) // Custom serialize for variables
{
	// Do main serialize from parent
	std::string s = Person::serialize(all);

	// Add BotAdvanced-specific attribute
	s += ("brave_val:" + std::to_string(this->braveVal) + ",");
	s += ("smart_val:" + std::to_string(this->smartVal) + ",");
	s += ("previousPos.x:" + std::to_string(this->previousPos.getX()) + ",");
	s += ("previousPos.y:" + std::to_string(this->previousPos.getY()) + ",");
	if (this->checkStandby == true) s += ("checkStandby:t,");
	else s += ("checkStandby:f,");
	s += ("standbyCount:" + std::to_string(this->standbyCount) + ",");

	// Return full serialization
	return s;
}

int BotPathfind::deserialize(std::string str) // Custom deserialize for variables
{
	// Do main deserialize from parent
	Person::deserialize(str);

	std::string parseForStr = df::match("", "brave_val");
	if (!parseForStr.empty())
	{
		this->braveVal = stoi(parseForStr);
	}
	parseForStr = df::match("", "smart_val");
	if (!parseForStr.empty())
	{
		this->smartVal = stoi(parseForStr);
	}

	// Set prev move
	int x;
	int y;
	parseForStr = df::match("", "previousPos.x");
	if (!parseForStr.empty())
	{
		x = stoi(parseForStr);
	}
	parseForStr = df::match("", "previousPos.y");
	if (!parseForStr.empty())
	{
		y = stoi(parseForStr);
	}
	this->previousPos.setXY( (float) x, (float) y );

	parseForStr = df::match("", "checkStandby");
	if (!parseForStr.empty())
	{
		if (parseForStr.at(0) == 't') this->checkStandby = true;
		else this->checkStandby = false;
	}
	parseForStr = df::match("", "standbyCount");
	if (!parseForStr.empty())
	{
		this->standbyCount = stoi(parseForStr);
	}

	return 0;
}
