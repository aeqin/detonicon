// Person.h

#pragma once

// System includes
#include <iostream>
#include <string>

// Engine includes
#include "EventKeyboard.h"

// Game includes
#include "HPContainer.h"
#include "PowerUp.h"
#include "utilities.h"

class ManagerPerson;
class PersonDisplay;

class Person : public df::Object
{
private:
	friend class Bomb;

protected:
	PersonDisplay* personDisplay; // HUD display for this Person
	HPContainer* HPbar; // Container that display above Person sprite that displays current HP
	df::Sprite* mainSprite = nullptr; // Sprite of Person
	df::Sprite* blinkSprite = nullptr; // Blank sprite to simulate blinking

	std::string name; // Name of Person
	int maxHP; // Max HP of Person
	int currHP; // Current HP of Person
	
	bool isInvincible; // Is Person invincible? 
	int invincibleCounter; // How long left Person is invincible
	int maxInvincibleCounter = 50; // Maximum length of invincibility
	bool blink; // Blink while invincible
	
	bool isVictor; // True if Person is victor of game
	
	int currBombs; // Number of remaining bombs
	int maxBombs; // Maximum number of held bombs
	int trueMaxBombs = 10; // Cap for max # of held bombs
	int bombPower; // Current power of bomb (length of explosion)
	int baseTimeToReload = 200; // Base time to reload bombs
	int timeTillReloaded; // Time left until reloaded

	int move_slowdown; // Counter to prevent control Person movement speed
	int move_countdown;
	int bumpCounter; // Counter to control the frequency of playing a "bump" sound
	int maxBumpCounter = 8;
	int prevX; // X coordinate last place occupied by Person
	int prevY; // Y coordinate last place occupied by Person

	// Used to determine movement
	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;

	int normalizeMoveToDir(const int move); // If positive return 1, if negative return -1, else return 0
	void incrementCounters(); // Increments various counters, do every step event

	// Custom networking
	int lastValidX = -1; // Last position the Client was at that was a valid position (x part)
	int lastValidY = -1; // Last position the Client was at that was a valid position (y part)
	bool flag_ignoreMoveUntilValid = false; // If true, have Server ignore moveXY commands unless the moveXY originates from the last valid position
	void setSpriteColorDeserialized(const int color); // Sets sprite's color after deserializing

public:
	Person(); // Constructor
	void setPersonSharedDefaults(PersonDisplay* display, std::string name, 
								 const int maxHP, const int maxBombs, 
								 const int x, const int y, 
								 df::Sprite* sprite); // Sets defaults of Person

	// Variable getters
	std::string getName(); // Gets name of Person
	int getCurrHP(); // Gets current HP of Person
	int getCurrBombs(); // Gets current # of bombs of Person
	int getMaxBombs(); // Gets current max # of bombs of Person
	int getBombPower(); // Gets bomb power (length of explosion) of Person
	int getMoveSlowdown(); // Gets movespeed (higher means slower) of Person
	int getPrevX(); // Gets previous x coordinate
	int getPrevY(); // Gets previous y coordinate
	int getIsInvincible(); // Gets whether Person isInvincible

	// Variable setters
	void setCurrHP(const int hp); // Sets currHP of Person
	void setMoveSlowdown(const int mov); // Sets move_slowdown of Person
	void setPersonDisplay(PersonDisplay* personDisplay); // Sets HUD display of Person
	void setPersonSprite(df::Sprite* sprite); // Sets the current sprite
	void setBlinkSprite(df::Sprite* sprite); // Sets the blank sprite to simulate blinking
	void setPrevPosAsCurrPos(); // Sets previous x and y coordinates as current position
	void setIsInvincible(bool invincible); // Sets whether Person isInvincible

	bool canPassDiagonally(df::Vector pos); // Checks if Person can move to pos using horizontal and then vertical movement
	bool isThereSolidAt(df::Vector pos); // Checks if there is a solid object at position
	bool isThereSolidHereExcludingSelf(df::Vector pos); // Checks if there is a solid object at position, ignoring self as solid
	bool isThereSolidHereExcludingPerson(df::Vector pos); // Checks if there is a solid object at position, ignoring Persons
	bool isThereBreakableAt(df::Vector pos); // Check if there is a breakable object at position
	bool isNearWall(df::Vector pos); // Check if there is a wall nearby the position
	bool isThisSolid(df::Object* obj); // Checks a custom list of objects to see if they are solid
	bool isThereDangerAt(df::Vector pos); // Checks if there is a dangerous object at position (MiniExplosion, Bomb)
	bool isThereWallAt(df::Vector pos); // Check if there is a wall object at position

	void takeDamage(const int damage); // Damages the Person
	void playBump(); // Plays bump sound
	void makeVictor(); // Makes Person victor of the game
	void die(); // Person dies
	void kickBomb(const float prevX, const float prevY, const int power); // Person attempts to kick bomb at current pos, using their previous pos
	void applyPowerUp(PowerUp* pow); // Apply powerup to Person
	void moveXY(const int dx, const int dy); // Move Person (Client calls this to check if need to send messages to Server)
	void moveXYTrue(const int dx, const int dy); // Move Person no questions asked (Client received correct message from Server, or moving on Server)
	void placeBomb(); // Place a bomb at position of Person

	// Custom networking
	std::string serialize(bool all = false); // Custom serialize for variables
	int deserialize(std::string str); // Custom deserialize for variables
	int getLastValidX(); // Returns the last valid position of the Client's Person (x part)
	int getLastValidY(); // Returns the last valid position of the Client's Person (y part)
	void setLastValidPos(df::Vector validPos); // Sets the last valid position of the Client's Person 
	bool getFlagIgnoreMove(); // Return the flag that tells the Server to ignore the move command or not
	void setFlagIgnoreMove(bool flag); // Sets the flag that tells the Server to ignore the move command or not
	void sendMoveXYToServer(df::Vector oldPos, const int dx, const int dy); // Sends message to Server to move Client by (dx, dy) after moving Person from old position on Client side

	~Person(); // Destructor, on Person death
};
