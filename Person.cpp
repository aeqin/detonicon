// Person.cpp

// System includes
#include <string>

// Engine includes
#include "Config.h"
#include "EventStep.h"
#include "Firework.h"
#include "GameManager.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"

// Game includes
#include "Bomb.h"
#include "EventNuke.h"
#include "EventPersonDeath.h"
#include "GameStart.h"
#include "ManagerMap.h"
#include "NetworkRole.h"
#include "Person.h"
#include "PersonDisplay.h"
#include "PowerUp.h"
#include "Toast.h"

Person::Person() {} // Constructor

void Person::setPersonSharedDefaults(PersonDisplay* display, std::string name, 
									 const int maxHP, const int maxBombs, 
									 const int x, const int y, 
									 df::Sprite* sprite) // Sets defaults of Person
{
	// Set up variables
	this->personDisplay = display; // HUD display for this Person
	this->HPbar = new HPContainer(this, 0, -1, maxHP); // Container that display above Person sprite that displays current HP

	this->name = name; // Name of Person
	this->maxHP = maxHP; // Max HP of Person
	this->currHP = maxHP; // Current HP of Person

	this->invincibleCounter = 0; // How long left Person is invincible
	this->isInvincible = false; // Is Person invincible?

	this->isVictor = false; // True if Person is victor of game

	this->currBombs = maxBombs; // Number of remaining bombs
	this->maxBombs = maxBombs; // Maximum number of held bombs
	this->trueMaxBombs = 10; // Cap for max # of held bombs
	this->bombPower = 5; // Current power of bomb (length of explosion)
	this->baseTimeToReload = 100; // Base time to reload bombs
	this->timeTillReloaded = this->baseTimeToReload; // Time left until reloaded
	
	this->move_slowdown = 3; // Counter to prevent control Person movement speed
	this->move_countdown = this->move_slowdown; // Current count until Person can move again
	this->prevX = x; // X coordinate last place occupied by Person
	this->prevY = y; // Y coordinate last place occupied by Person

	// Set solidness
	setSolidness(df::SOFT);

	// Set sprite
	setPersonSprite(sprite);

	// Set altitude
	setAltitude(3);

	// Set Box
	this->setBox(df::Box());

	// Set position
	WM.moveObject(this, df::Vector((float) x, (float) y));

	// Register with events
	registerInterest(df::STEP_EVENT); // To update per frame
}

std::string Person::getName() // Gets name of Person
{
	return this->name;
}

int Person::getCurrHP() // Gets current HP of Person
{
	return this->currHP;
}

int Person::getCurrBombs() // Gets current # of bombs of Person
{
	return this->currBombs;
}

int Person::getMaxBombs() // Gets current max # of bombs of Person
{
	return this->maxBombs;
}

int Person::getBombPower() // Gets bomb power (length of explosion) of Person
{
	return this->bombPower;
}

int Person::getMoveSlowdown() // Gets movespeed (higher means slower) of Person
{
	return this->move_slowdown;
}

int Person::getPrevX() // Gets previous x coordinate
{
	return this->prevX;
}

int Person::getPrevY() // Gets previous y coordinate
{
	return this->prevY;
}

int Person::getIsInvincible() // Gets whether Person isInvincible
{
	return this->isInvincible;
}

void Person::setCurrHP(const int hp) // Sets currHP of Person
{
	this->currHP = hp;
}

void Person::setMoveSlowdown(const int move_slowdown) // Sets move_slowdown of Person
{
	this->move_slowdown = move_slowdown;
}

void Person::setPersonDisplay(PersonDisplay* personDisplay) // Sets HUD display of Person
{
	this->personDisplay = personDisplay;
}

void Person::setPersonSprite(df::Sprite* sprite) // Sets the current sprite
{
	if(this->mainSprite == nullptr)
		this->mainSprite = sprite;
	
	if(sprite == nullptr)
		sprite = this->mainSprite;

	setSprite(sprite);
	setSpriteSlowdown(3);  // 1/3 speed animation
	setTransparency(); // Transparent sprite
}

void Person::setBlinkSprite(df::Sprite* blinkSprite) // Sets the blank sprite to simulate blinking
{
	if(this->blinkSprite == nullptr)
		this->blinkSprite = blinkSprite;
}

void Person::setIsInvincible(bool invincible)
{
	this->isInvincible = invincible;
	if(this->isInvincible == true) this->invincibleCounter = this->maxInvincibleCounter;
}

void Person::setPrevPosAsCurrPos() // Sets previous x and y coordinates as current position
{
	this->prevX = (int) this->getPosition().getX();
	this->prevY = (int) this->getPosition().getY();
}

bool Person::canPassDiagonally(df::Vector newPos) // Checks if Person can move to pos using horizontal and then vertical movement
{
	df::Vector oldPos(this->getPosition());
	float x = oldPos.getX();
	float y = oldPos.getY();

	if(newPos.getX() == x - 1 && newPos.getY() == y - 1) // Left Top Diagonal
	{
		if(isThereSolidHereExcludingSelf(df::Vector(x - 1, y)) && isThereSolidHereExcludingSelf(df::Vector(x, y - 1)))
		{
			return false;
		}
	}
	else if(newPos.getX() == x + 1 && newPos.getY() == y - 1) // Right Top Diagonal
	{
		if(isThereSolidHereExcludingSelf(df::Vector(x + 1, y)) && isThereSolidHereExcludingSelf(df::Vector(x, y - 1)))
		{
			return false;
		}
	}
	else if(newPos.getX() == x - 1 && newPos.getY() == y + 1) // Left Bot Diagonal
	{
		if(isThereSolidHereExcludingSelf(df::Vector(x - 1, y)) && isThereSolidHereExcludingSelf(df::Vector(x, y + 1)))
		{
			return false;
		}
	}
	else if(newPos.getX() == x + 1 && newPos.getY() == y + 1) // Right Bot Diagonal
	{
		if(isThereSolidHereExcludingSelf(df::Vector(x + 1, y)) && isThereSolidHereExcludingSelf(df::Vector(x, y + 1)))
		{
			return false;
		}
	}
	return true;
}

bool Person::isThereSolidAt(df::Vector pos) // Checks if there is a solid object at position
{
	if(pos.getX() <= 0 || pos.getX() >= MM.getMaxCols() - 1 || pos.getY() <= 0 || pos.getY() >= MM.getMaxRows() - 1) return true; // Consider map bounds as solid

	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for(i.first(); !i.isDone(); i.next())
	{
		df::Object *obj = i.currentObject();
		df::Vector objpos = obj->getPosition();

		if(obj->getSolidness() == df::HARD && objpos == pos || (isThisSolid(obj) && objpos == pos))
		{
			return true;
		}
	}
	return false;
}

bool Person::isThereSolidHereExcludingSelf(df::Vector pos) // Checks if there is a solid object at Person's position, ignoring Person
{
	if(pos.getX() <= 0 || pos.getX() >= MM.getMaxCols() - 1 || pos.getY() <= 0 || pos.getY() >= MM.getMaxRows() - 1) return true; // Consider map bounds as solid

	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for(i.first(); !i.isDone(); i.next())
	{
		df::Object *obj = i.currentObject();
		if(obj == this) continue; // Ignore counting self as solid object

		df::Vector objpos = obj->getPosition();
		if((obj->getSolidness() == df::HARD && objpos == pos) || (isThisSolid(obj) && objpos == pos))
		{
			return true;
		}
	}
	return false;
}

bool Person::isThereSolidHereExcludingPerson(df::Vector pos) // Checks if there is a solid object at Person's position, ignoring Person
{
	if(pos.getX() <= 0 || pos.getX() >= MM.getMaxCols() - 1 || pos.getY() <= 0 || pos.getY() >= MM.getMaxRows() - 1) return true; // Consider map bounds as solid

	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for(i.first(); !i.isDone(); i.next())
	{
		df::Object *obj = i.currentObject();
		if(obj->getType().find("Person") != std::string::npos) continue; // Ignore counting Persons as solid object

		df::Vector objpos = obj->getPosition();
		if((obj->getSolidness() == df::HARD && objpos == pos) || (isThisSolid(obj) && objpos == pos))
		{
			return true;
		}
	}
	return false;
}

bool Person::isThereBreakableAt(df::Vector pos) // Check if there is a breakable object at position
{
	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for (i.first(); !i.isDone(); i.next())
	{
		df::Object *p_o = i.currentObject();
		if (p_o->getType() == "WallBreakable")
		{
			return true;
		}
	}
	return false;
}

bool Person::isNearWall(df::Vector pos) // Check if there is a Wall nearby the position
{
	// Initialize adjacent positions for checking
	df::Vector top, bottom, left, right;
	left.setXY(pos.getX() - 1, pos.getY());
	right.setXY(pos.getX() + 1, pos.getY());
	top.setXY(pos.getX(), pos.getY() - 1);
	bottom.setXY(pos.getX(), pos.getY() + 1);

	// Check if the surrounding is of type "Wall"
	if (isThereWallAt(left) || isThereWallAt(right) || isThereWallAt(top) || isThereWallAt(bottom)) {
		return true;
	}
	return false;
}

bool Person::isThisSolid(df::Object* obj) // Checks a custom list of objects to see if they are solid
{
	if(obj->getSolidness() == df::HARD) return true;
	if(obj->getSolidness() != df::SPECTRAL)
	{
		std::string s = obj->getType();
		if(s.find("Person") != std::string::npos || s == "Wall" || s == "WallBreakable")
		{
			return true;
		}
	}
	return false;
}

bool Person::isThereDangerAt(df::Vector pos) // Checks if there is a dangerous object at position (MiniExplosion, Bomb)
{
	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for(i.first(); !i.isDone(); i.next())
	{
		df::Object *p_o = i.currentObject();
		if(p_o->getType() == "MiniExplosion" || p_o->getType() == "Bomb")
		{
			return true;
		}
	}
	return false;
}

bool Person::isThereWallAt(df::Vector pos) // Check if there is a Wall at position
{
	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for (i.first(); !i.isDone(); i.next())
	{
		df::Object *p_o = i.currentObject();
		if (p_o->getType() == "Wall")
		{
			return true;
		}
	}
	return false;
}

void Person::takeDamage(const int damage) // Damages the Person
{
	if((this->isInvincible == true && damage < 9999) || NR.getClient()) return; // Only take damage if on Server and not invincible

	this->currHP -= damage;
	this->HPbar->takeDamage(damage);
	this->personDisplay->updateHealth(this->currHP);

	// Invincibility frames
	this->isInvincible = true;
	this->invincibleCounter = maxInvincibleCounter;

	// Show toast of taking damage
	new Toast((int) this->getPosition().getX(), (int) this->getPosition().getY() - 1, Toast::ToastStr::OW);

	// Play taking damage sound
	df::Sound *p_sound = RM.getSound("takedmg");
	if(p_sound != nullptr && df::Config::getInstance().getHeadless() == false)
	{
		p_sound->play();
	}

	if(NR.getServer() && !GM.getGameOver()) // Send message to Client to blink
	{
		std::string msg = "BLINK|"; // 'BLINK'|personID
		msg += std::to_string(this->getId());

		const char* msgToSend = msg.c_str();
		LM.writeLog("Person::takeDamage(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Clients to blink Person that took damage
	}
}

void Person::playBump() // Plays bump sound
{
	if(NR.getServer() && !GM.getGameOver()) // Send message to Client to increase # of walls bumped stat
	{
		std::string msg = "INCR BUMP"; // 'INCR BUMP'
		const char* msgToSend = msg.c_str();
		LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend, NR.getServer()->getSocketIDFromPersonObjectID(this->getId())); // Send message to Client to increment # wall bumps
	}

	if(this->bumpCounter == 0)
	{
		if(NR.getClient() && NR.getClient()->getPersonObjectID() != this->getId()) return; // Don't play bumps from other Clients

		// Play bumping (into solid) sound
		df::Sound *p_sound = RM.getSound("bump");
		if(p_sound != nullptr && df::Config::getInstance().getHeadless() == false)
		{
			p_sound->play();
			this->bumpCounter = this->maxBumpCounter;
		}
	}
}

void Person::makeVictor() // Makes Person victor of the game, causing it to ignore future events (to prevent moving on victory screen)
{
	this->isVictor = true;
}

void Person::die() // Person dies
{
	if(!(NR.getServer() && !GM.getGameOver())) return; // Only allow authoritative Server to die

	EventPersonDeath persondeath(this->name, this->getId());
	WM.onEvent(&persondeath);

	//WM.markForDelete(this); // This causes problems with NetworkLagSimulator (as Server attempts to send message with a deleted object pointer due to simulated lag)
	setActive(false);

	// Spawn firework on death
	df::addParticles(df::ParticleType::FIREWORKS, df::Vector(getPosition().getX(), getPosition().getY()), 2.0f, df::Color::CUSTOM);
	df::addParticles(df::ParticleType::SMOKE, df::Vector(getPosition().getX(), getPosition().getY()), 0.75f, df::Color::CUSTOM);
	df::addParticles(df::ParticleType::SPARKS, df::Vector(getPosition().getX(), getPosition().getY()), 1.0f, df::Color::CUSTOM);

	// Send message to Client to have Person die
	std::string msg = "PERSON DEATH|"; // 'PERSON DEATH'|personID message
	msg += std::to_string(this->getId());

	const char* msgToSend = msg.c_str();
	LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
	NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Clients to have Person die
}

int Person::normalizeMoveToDir(const int move) // If positive return 1, if negative return -1, else return 0
{
	if(move < 0) return -1;
	else if(move > 0) return 1;
	else return 0;
}

void Person::moveXY(const int dx, const int dy) // Move Person (Client calls this to check if need to send messages to Server)
{
	if(GS.getIsGameOver() == true) return; // Don't move if game is not in session

	// Internal cooldown keeps hero's movespeed in check
	if(!NR.getServer()) // Only Client needs to keep movespeed in check because only Client is sending the movement commands
	{
		if(this->move_countdown > 0)
		{
			return;
		}
		this->move_countdown = this->move_slowdown;

		if(NR.getClient()->getIsUsingLagCompensation() == false) // Client is not using lag compensation
		{
			sendMoveXYToServer(this->getPosition(), dx, dy); // Tell Server to move Person
			return; // So don't move on Client, only allow Server to move Person
		}
	}

	df::Vector oldPos = this->getPosition(); // Remember position before move

	moveXYTrue(dx, dy); // Do move

	sendMoveXYToServer(oldPos, dx, dy); // Possibly moved, so send message to Server to attempt same move
}

void Person::moveXYTrue(const int dx, const int dy) // Move Person no questions asked (Client received correct message from Server, or moving on Server)
{
	if(GS.getIsGameOver() == true) return; // Don't move if game is not in session

	int xMin = MM.getXBoundMin();
	int xMax = MM.getXBoundMax();
	int yMin = MM.getYBoundMin();
	int yMax = MM.getYBoundMax();
	int dxLeft = dx * 2;
	int dyLeft = dy;
	int maxMove = (abs(dxLeft) > abs(dyLeft)) ? abs(dxLeft) : abs(dyLeft);
	df::Vector trueOldPos = getPosition(); // Remember this for diagonal kicks
	for(int i = 0; i < maxMove; i++)
	{
		int x = normalizeMoveToDir(dxLeft);
		int y = normalizeMoveToDir(dyLeft);
		df::Vector oldPos = getPosition();
		df::Vector new_pos(oldPos.getX() + x, oldPos.getY() + y); // New pos to move Person to

		/*if(canPassDiagonally(new_pos) == false) // Trying moving diagonally pass two diagonally adjacent walls
		{
			playBump();

			break;
		}*/
		if(isThereSolidHereExcludingSelf(new_pos)) // Needed because Person is df::SOFT
		{
			df::Vector new_posX(oldPos.getX() + x, oldPos.getY());
			if((new_posX.getX() >= xMin) && (new_posX.getX() <= xMax) && isThereSolidHereExcludingSelf(new_posX) == false && x != 0) // Try moving just x
			{
				if(WM.moveObject(this, new_posX) >= 0)
				{
					int scaledPowerByMoveSpeed = (int) (30 * ((float) 1 / this->move_slowdown));
					if(NR.getServer()) kickBomb(trueOldPos.getX(), trueOldPos.getY(), scaledPowerByMoveSpeed); // Kick any bombs that are at Person's new position, using old position for calculating kick direction

					if(NR.getClient() && this->getId() == NR.getClient()->getPersonObjectID()) NR.getClient()->incrNumSpacesMoved(); // Increment number of spaces moved by Person controlled by this Client
				}
				break;
			}
			else
			{
				df::Vector new_posY(oldPos.getX(), oldPos.getY() + y);
				if((new_posY.getY() >= yMin) && (new_posY.getY() <= yMax) && isThereSolidHereExcludingSelf(new_posY) == false && y != 0) // Try moving just y
				{
					if(WM.moveObject(this, new_posY) >= 0) // If move successful
					{
						int scaledPowerByMoveSpeed = (int) (30 * ((float) 1 / this->move_slowdown));
						if(NR.getServer()) kickBomb(trueOldPos.getX(), trueOldPos.getY(), scaledPowerByMoveSpeed); // Kick any bombs that are at Person's new position, using old position for calculating kick direction

						if(NR.getClient() && this->getId() == NR.getClient()->getPersonObjectID()) NR.getClient()->incrNumSpacesMoved(); // Increment number of spaces moved by Person controlled by this Client
					}
					break;
				}
				else
				{
					playBump();

					break;
				}
			}
		}
		else
		{
			if(WM.moveObject(this, new_pos) >= 0) // If move successful
			{
				int scaledPowerByMoveSpeed = (int) (30 * ((float) 1 / this->move_slowdown));
				if(NR.getServer()) kickBomb(trueOldPos.getX(), trueOldPos.getY(), scaledPowerByMoveSpeed); // Kick any bombs that are at Person's new position, using old position for calculating kick direction

				if(NR.getClient() && this->getId() == NR.getClient()->getPersonObjectID()) NR.getClient()->incrNumSpacesMoved(); // Increment number of spaces moved by Person controlled by this Client
			}
		}

		setPrevPosAsCurrPos();

		if(dxLeft > 0) dxLeft--;
		else if(dxLeft < 0) dxLeft++;
		if(dyLeft > 0) dyLeft--;
		else if(dyLeft < 0) dyLeft++;
	}
}

void Person::placeBomb() // Place a bomb at position of Person
{
	if(GS.getIsGameOver() == true) return; // Don't place Bomb if game is not in session
	if(this->currBombs == 0) return; // Don't place Bomb if not holding any

	if(!NR.getServer() && this->getId() == NR.getClient()->getPersonObjectID()) // Is Client so don't place Bomb, instead send message to Server to place Bomb
	{
		std::string msg = "PLACEBOMB"; // 'PLACEBOMB'
		const char* msgToSend = msg.c_str();

		LM.writeLog("Person::kbd(): sending custom message: '%s'", msgToSend);
		NR.getClient()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Server to place Bomb

		return; // Is Client so don't place bomb
	}

	if(NR.getServer() && !GM.getGameOver()) // Only allow authoritative server to place Bomb
	{
		int fuseTime = 50; // Time until bomb explodes
		int stayTime = 6; // Time explosion hangs around

		Bomb* p_bomb = new Bomb(true, df::Vector(getPosition().getX() + 1, getPosition().getY()), this, fuseTime, stayTime, this->bombPower); // Place bomb on Server

		std::string msg = "PLACE BOMB|"; // 'PLACE BOMB'|bombID|xPosition|yPosition|personID|fuseTime|stayTime|bombPower
		msg += std::to_string(p_bomb->getBombID());
		msg += "|";
		msg += std::to_string((int) getPosition().getX() + 1);
		msg += "|";
		msg += std::to_string((int) getPosition().getY());
		msg += "|";
		msg += std::to_string(this->getId());
		msg += "|";
		msg += std::to_string(fuseTime);
		msg += "|";
		msg += std::to_string(stayTime);
		msg += "|";
		msg += std::to_string(this->bombPower);

		const char* msgToSend = msg.c_str();
		LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Clients to place Bomb

		this->currBombs -= 1;
		this->personDisplay->updateBombs(this->currBombs);

		if(this->currBombs == 0) timeTillReloaded = baseTimeToReload;
	}
}

void Person::kickBomb(const float prevX, const float prevY, const int power) // Person attempts to kick bomb at current pos, using their previous pos
{
	if(!NR.getServer()) return; // Only let authoritive Server check

	df::Vector pos(this->getPosition());
	df::ObjectList object_list = WM.objectsAtPosition(pos);
	df::ObjectListIterator i(&object_list);
	for(i.first(); !i.isDone(); i.next())
	{
		df::Object *p_o = i.currentObject();
		if(p_o->getType() == "Bomb")
		{
			Bomb* b = (Bomb*) p_o;
			if(b->getCenterPos() == pos)
			{
				LM.writeLog("current pos %f, %f | prev pos %f, %f", getPosition().getX(), getPosition().getY(), prevX, prevY);
				b->getKicked(prevX, prevY, power);

				// Send message to Client to also kick Bomb
				std::string msg = "KICK BOMB|"; // 'KICK BOMB'|bombID|prevX|prevY|power
				msg += std::to_string(b->getBombID());
				msg += "|";
				msg += std::to_string((int) prevX);
				msg += "|";
				msg += std::to_string((int) prevY);
				msg += "|";
				msg += std::to_string((int) power);

				const char* msgToSend = msg.c_str();
				LM.writeLog("Person::kickBomb(): sending custom message: '%s'", msgToSend);
				NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Clients to kick Bomb
			}
		}
		else if(p_o->getType().find("PowerUp") != std::string::npos)
		{
			PowerUp* p = (PowerUp*) p_o;
			this->applyPowerUp(p);
			p->setSolidness(df::SPECTRAL); // Prevent PowerUp from activating multiple times before it is deleted
			p->setActive(false);
			WM.markForDelete(p);

			if(NR.getServer() && !GM.getGameOver()) // Send message to Client to increase # of PowerUps picked up
			{
				std::string msg = "INCR PICKUP"; // 'INCR PICKUP'
				const char* msgToSend = msg.c_str();
				LM.writeLog("Client::handleConnect(): sending custom message: '%s'", msgToSend);
				NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend, NR.getServer()->getSocketIDFromPersonObjectID(this->getId())); // Send message to Client to increment # PowerUps picked up
			}
		}
	}
}

void Person::applyPowerUp(PowerUp* pow) // Apply PowerUp to Person
{
	int type = pow->getPowerType();
	Toast::ToastStr toast = Toast::ToastStr::NONE;

	if(type == TYPE_HP) // Gain HP
	{
		if(this->currHP + 1 <= this->maxHP)
		{
			this->currHP++;
			this->personDisplay->updateHealth(this->currHP);
			this->HPbar->takeDamage(-1);
			toast = Toast::ToastStr::HP_UP;
		}
		else
		{
			toast = Toast::ToastStr::HP_ALREADY_MAX;
		}
	}
	else if(type == TYPE_AMMO) // Gain bomb ammo
	{
		// Reloading automatically implementation
		if(this->maxBombs + 1 <= this->trueMaxBombs)
		{
			this->maxBombs++;
			this->currBombs = this->maxBombs;
			this->personDisplay->updateBombs(this->maxBombs);
			this->personDisplay->updatePowers(this);
			toast = Toast::ToastStr::AMMO_UP;
		}
		else
		{
			toast = Toast::ToastStr::AMMO_MAX;
		}

		/* Not reloading automatically implementation
		if(this->currBombs + 1 <= this->maxBombs)
		{
			this->currBombs++;
			this->personDisplay->updateBombs(this->currBombs);
		}
		*/
	}
	else if(type == TYPE_BOMBLENGTH) // Increase length of bomb explosion from Person
	{
		this->bombPower++;
		this->personDisplay->updatePowers(this);
		toast = Toast::ToastStr::BOMBPWR_UP;
	}
	else if(type == TYPE_SPEED) // Increase movespeed of Person
	{
		this->move_slowdown--;
		if(this->move_slowdown <= 0)
		{
			this->move_slowdown = 1;
			toast = Toast::ToastStr::SPD_ALREADY_MAX;
		}
		else
		{
			toast = Toast::ToastStr::SPD_UP;
		}
		this->personDisplay->updatePowers(this);

		std::string msg = "SPEED UP|";
		msg += std::to_string(this->getId());
		msg += "|";
		msg += std::to_string(this->move_slowdown);

		const char* msgToSend = msg.c_str(); // 'SPEED UP'|personID|move_slowdown message

		LM.writeLog("Person::applyPowerUp() : sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend, NR.getServer()->getSocketIDFromPersonObjectID(this->getId())); // Send message to Client to speedup
	}
	else if(type == TYPE_NUKE) // Instantly explode every bomb on screen
	{
		EventNuke nuke;
		WM.onEvent(&nuke);
		toast = Toast::ToastStr::NUKE;

		std::string msg = "SPAWN NUKE|";

		const char* msgToSend = msg.c_str(); // 'SPAWN NUKE' message

		LM.writeLog("Person::applyPowerUp() : sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn event nuke
	}
	
	if(toast != Toast::ToastStr::NONE) // Show toast of gaining power up
	{
		if(NR.getServer() && !GM.getGameOver())
		{
			Toast* t = new Toast((int) pow->getPosition().getX(), (int) pow->getPosition().getY() - 1, toast); // Create Toast on Server

			std::string msg = "SPAWN TOAST|";
			msg += std::to_string(this->getId());
			msg += "|";
			msg += std::to_string(t->getIntFromToast(toast));
			msg += "|";
			msg += std::to_string((int) this->getPosition().getX());
			msg += "|";
			msg += std::to_string((int) this->getPosition().getY());
	
			const char* msgToSend = msg.c_str(); // 'SPAWN TOAST'|idOfObject|toastAsInt|xCoordinate|yCoordinate message

			LM.writeLog("Person::applyPowerUp() : sending custom message: '%s'", msgToSend);
			NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn Toast

			// Tell Client the PowerUp was picked up
			msg = "DELETE POWER|";
			msg += std::to_string(pow->getPowID());

			msgToSend = msg.c_str(); // 'DELETE POWER'|powID message

			LM.writeLog("Person::applyPowerUp() : sending custom message: '%s'", msgToSend);
			NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to delete taken PowerUp
		}
	}

	// Play gaining powerup sound
	df::Sound *p_sound = RM.getSound("powerup");
	if(p_sound != nullptr  && df::Config::getInstance().getHeadless() == false)
	{
		p_sound->play();
	}
}

void Person::incrementCounters() // Increments various counters, do every step event
{
	// Move countdown
	this->move_countdown--;
	if(this->move_countdown < 0)
	{
		this->move_countdown = 0;
	}

	// Bump counter
	this->bumpCounter--;
	if(this->bumpCounter < 0)
	{
		this->bumpCounter = 0;
	}

	// Invincible countdown
	this->invincibleCounter--;
	if(this->invincibleCounter < 0)
	{
		this->invincibleCounter = 0;
		this->isInvincible = false;
		this->blink = false;

		setVisible(true);
	}
	else
	{
		if(this->blink == true)
		{
			setVisible(false);
			this->blink = false;
		}
		else if(invincibleCounter != 0 && invincibleCounter % 5 == 0)
		{
			setVisible(true);
			this->blink = true;
		}
	}

	if(!NR.getServer()) return; // Only increment on authoritative Server

	if(this != nullptr && MM.isInBounds(this->getPosition()) == false) // Destroy if out of map bounds
	{
		takeDamage(9999);
	}

	// Bomb reload countdown
	this->timeTillReloaded--;
	if(this->timeTillReloaded <= 0)
	{
		this->timeTillReloaded = 0;
		if(this->currBombs == 0)
		{
			this->currBombs = this->maxBombs;
			this->personDisplay->updateBombs(this->currBombs);
		}
	}
}

std::string Person::serialize(bool all) // Custom serialize for variables
{
	// Do main serialize from parent
	std::string s = Object::serialize(all);

	// Add Person-specific attribute
	s += ("sprite_color:" + std::to_string(this->getSprite()->getColor()) + ",");
	//if(isVisible() == true) s += ("isVisible:t,");
	//else s += ("isVisible:f,");

	s += ("m_name:" + this->name + ",");

	s += ("m_maxHP:" + std::to_string(this->maxHP) + ",");
	s += ("m_currHP:" + std::to_string(this->currHP) + ",");
	if(this->isInvincible == true) s += ("m_isInvincible:t,");
	else s += ("m_isInvincible:f,");
	s += ("m_invincibleCounter:" + std::to_string(this->invincibleCounter) + ",");
	if(this->blink == true) s += ("m_blink:t,");
	else s += ("m_blink:f,");

	if(this->isVictor == true) s += ("m_isVictor:t,");
	else s += ("m_isVictor:f,");

	s += ("m_currBombs:" + std::to_string(this->currBombs) + ",");
	s += ("m_maxBombs:" + std::to_string(this->maxBombs) + ",");
	s += ("m_bombPower:" + std::to_string(this->bombPower) + ",");
	s += ("m_timeTillReloaded:" + std::to_string(this->timeTillReloaded) + ",");

	s += ("m_move_slowdown:" + std::to_string(this->move_slowdown) + ",");
	s += ("m_move_countdown:" + std::to_string(this->move_countdown) + ",");

	s += ("m_prevX:" + std::to_string(this->prevX) + ",");
	s += ("m_prevY:" + std::to_string(this->prevY) + ",");

	if(this->up == true) s += ("m_up:t,");
	else s += ("m_up:f,");
	if(this->down == true) s += ("m_down:t,");
	else s += ("m_down:f,");
	if(this->left == true) s += ("m_left:t,");
	else s += ("m_left:f,");
	if(this->right == true) s += ("m_right:t,");
	else s += ("m_right:f,");
	
	// Return full serialization
	return s;
}

int Person::deserialize(std::string str) // Custom deserialize for variables
{
	if(GS.getIsGameOver() == true) // Only serialize at the start, when the game is not in session yet (prevents position bouncebacks)
	{
		Object::deserialize(str); // Do main deserialize from parent
	}

	// Deserialize variables
	std::string parseForStr = df::match("", "sprite_color");
	if(!parseForStr.empty())
	{
		int color = atoi(parseForStr.c_str());
		if(getSprite() != nullptr) setSpriteColorDeserialized(color);
	}
	parseForStr = df::match("", "m_name");
	if(!parseForStr.empty())
	{
		this->name = parseForStr;
	}

	parseForStr = df::match("", "m_maxHP");
	if(!parseForStr.empty())
	{
		this->maxHP = stoi(parseForStr);
	}
	parseForStr = df::match("", "m_currHP");
	if(!parseForStr.empty())
	{
		this->currHP = stoi(parseForStr);
	}
	parseForStr = df::match("", "m_isInvincible");
	if(!parseForStr.empty())
	{
		if(parseForStr.at(0) == 't') this->isInvincible = true;
		else this->isInvincible = false;
	}
	parseForStr = df::match("", "m_invincibleCounter");
	if(!parseForStr.empty())
	{
		this->invincibleCounter = stoi(parseForStr);
	}
	parseForStr = df::match("", "m_blink");
	if(!parseForStr.empty())
	{
		if(parseForStr.at(0) == 't') this->blink = true;
		else this->blink = false;
	}

	parseForStr = df::match("", "m_isVictor");
	if(!parseForStr.empty())
	{
		if(parseForStr.at(0) == 't') this->isVictor = true;
		else this->isVictor = false;
	}

	parseForStr = df::match("", "m_currBombs");
	if(!parseForStr.empty())
	{
		this->currBombs = stoi(parseForStr);
	}
	parseForStr = df::match("", "m_maxBombs");
	if(!parseForStr.empty())
	{
		this->maxBombs = stoi(parseForStr);
	}
	parseForStr = df::match("", "m_bombPower");
	if(!parseForStr.empty())
	{
		this->bombPower = stoi(parseForStr);
	}
	parseForStr = df::match("", "m_timeTillReloaded");
	if(!parseForStr.empty())
	{
		this->timeTillReloaded = stoi(parseForStr);
	}

	parseForStr = df::match("", "m_move_slowdown");
	if(!parseForStr.empty())
	{
		this->move_slowdown = stoi(parseForStr);
	}
	parseForStr = df::match("", "m_move_countdown");
	if(!parseForStr.empty())
	{
		this->move_countdown = stoi(parseForStr);
	}

	parseForStr = df::match("", "m_prevX");
	if(!parseForStr.empty())
	{
		this->prevX = stoi(parseForStr);
	}
	parseForStr = df::match("", "m_prevY");
	if(!parseForStr.empty())
	{
		this->prevY = stoi(parseForStr);
	}

	return 0;
}

int Person::getLastValidX() // Returns the last valid position of the Client's Person (x part)
{
	return this->lastValidX;
}

int Person::getLastValidY() // Returns the last valid position of the Client's Person (y part)
{
	return this->lastValidY;
}

void Person::setLastValidPos(df::Vector validPos) // Sets the last valid position of the Client's Person 
{
	this->lastValidX = (int) validPos.getX();
	this->lastValidY = (int) validPos.getY();
}

bool Person::getFlagIgnoreMove() // Return the flag that tells the Server to ignore the move command or not
{
	return this->flag_ignoreMoveUntilValid;
}

void Person::setFlagIgnoreMove(bool flag) // Sets the flag that tells the Server to ignore the move command or not
{
	this->flag_ignoreMoveUntilValid = flag;
}

void Person::sendMoveXYToServer(df::Vector oldPos, const int dx, const int dy) // Sends message to Server to move Client by (dx, dy) after moving Person from old position on Client side
{
	if(GS.getIsGameOver() || GM.getGameOver()) return;

	if(!NR.getServer() && this->getId() == NR.getClient()->getPersonObjectID()) // Is Client, and is Person controlled by Client so send msg to Server to move
	{
		if(NR.getClient()->getIsUsingLagCompensation() == true) // Client using lag compensation
		{
			if(oldPos == this->getPosition()) return; // Didn't move so don't send message to Server to move

			std::string msg = "MOVEXY(LAGCOMP)|"; // 'MOVEXY(LAGCOMP)'|moveX|moveY|oldX|oldY|destX|destY message
			msg += std::to_string(dx);
			msg += "|";
			msg += std::to_string(dy);
			msg += "|";
			msg += std::to_string((int) oldPos.getX()); // Old pos X
			msg += "|";
			msg += std::to_string((int) oldPos.getY()); // Old pos Y
			msg += "|";
			msg += std::to_string((int) getPosition().getX()); // Destination pos X
			msg += "|";
			msg += std::to_string((int) getPosition().getY()); // Destination pos Y
			const char* msgToSend = msg.c_str();

			LM.writeLog("Person::kbd(): sending custom message: '%s'", msgToSend);
			NR.getClient()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Server to move Person
		}
		else // Client is not using lag compensation
		{
			std::string msg = "MOVEXY(NO_LAGCOMP)|"; // 'MOVEXY(NO_LAGCOMP)'|moveX|moveY|oldX|oldY
			msg += std::to_string(dx);
			msg += "|";
			msg += std::to_string(dy);
			msg += "|";
			msg += std::to_string((int) oldPos.getX()); // Old pos X
			msg += "|";
			msg += std::to_string((int) oldPos.getY()); // Old pos Y
			const char* msgToSend = msg.c_str();

			LM.writeLog("Person::kbd(): sending custom message: '%s'", msgToSend);
			NR.getClient()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Server to move Person
		}

		NR.getClient()->incrNumMoveCmdsSent(); // Increment numMoveCmdsSent statistic
	}
}

void Person::setSpriteColorDeserialized(const int color) // Sets sprite's color after deserializing
{
	switch(color)
	{
	case 0:
		getSprite()->setColor(df::Color::BLACK);
		break;
	case 1:
		getSprite()->setColor(df::Color::RED);
		break;
	case 2:
		getSprite()->setColor(df::Color::GREEN);
		break;
	case 3:
		getSprite()->setColor(df::Color::YELLOW);
		break;
	case 4:
		getSprite()->setColor(df::Color::BLUE);
		break;
	case 5:
		getSprite()->setColor(df::Color::MAGENTA);
		break;
	case 6:
		getSprite()->setColor(df::Color::CYAN);
		break;
	case 7:
		getSprite()->setColor(df::Color::WHITE);
		break;
	default:
		getSprite()->setColor(df::Color::CUSTOM);
		break;
	}
}

Person::~Person() // On Person death
{
	// Spawn firework on death
	df::addParticles(df::ParticleType::FIREWORKS, df::Vector(getPosition().getX(), getPosition().getY()), 2.0f, df::Color::CUSTOM);
	df::addParticles(df::ParticleType::SMOKE, df::Vector(getPosition().getX(), getPosition().getY()), 0.75f, df::Color::CUSTOM);
	df::addParticles(df::ParticleType::SPARKS, df::Vector(getPosition().getX(), getPosition().getY()), 1.0f, df::Color::CUSTOM);

	// Send message to Server (& Clients) to delete this Object
	if(NR.getServer() && !GM.getGameOver()) // Only send delete message to Server
	{
		NR.getServer()->sendMessage(df::DELETE_OBJECT, this);
	}
}