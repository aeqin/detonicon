// PersonDisplay.cpp

// Engine includes
#include "EventStep.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"

// Game includes
#include "NetworkRole.h"

#include "PersonDisplay.h"

PersonDisplay::PersonDisplay(bool isOnServer, df::Vector pos, Person* person) // Constructor, only creates object if Server
{
	if(isOnServer == true)
	{
		initOnServer(pos, person);
	}
}

void PersonDisplay::initOnServer(df::Vector pos, Person* person)
{
	// Set sprite
	setDisplaySprite("personDisplay");

	// Set object type
	setType("PersonDisplay");

	// Set up variables
	int x = (int) pos.getX();
	int y = (int) pos.getY();
	this->originalPos = df::Vector((float) x + getSprite()->getWidth() / 2, (float) y); // Original position of PersonDisplay before shake
	this->shakeCounterCurr = 0; // Current count until shake again
	this->shakeCounterMax = 10; // Current count until shake again
	this->shakeLeft = true; // Is there more shake left?

	this->personToFollow = person; // Person whose stats PersonDisplay displays
	this->personToFollow->setPersonDisplay(this);

	// Register with events
	registerInterest(df::STEP_EVENT); // To update position during shaking

	// Set position
	setPosition(this->originalPos);

	// Display Person sprite
	int xFudge = 2;

	// Set Altitude
	setAltitude(0);

	if(NR.getServer() && !GM.getGameOver()) // On Server
	{
		this->pds = new PersonDisplaySprite((int) (this->getPosition().getX() - this->getSprite()->getWidth() / 2.5), y + 1, this->personToFollow->getSprite());
		this->pdn = new PersonDisplayName((int) this->getPosition().getX() - xFudge, y - 1, this->personToFollow->getName(), 24); // 24 allows maximum name to be 18 characters
		this->pdh = new PersonDisplayHealth(true, df::Vector(this->getPosition().getX() - xFudge, (float) y), this->personToFollow->getCurrHP(), 24); // 24 allows maximum HP to be 16
		this->pdb = new PersonDisplayBomb(true, df::Vector(this->getPosition().getX() - xFudge, (float) y + 1), this->personToFollow->getCurrBombs(), 24); // 24 allows maximum bombs to be 17
		this->pdp = new PersonDisplayPower(true, df::Vector(this->getPosition().getX() - xFudge, (float) y + 2), 24);
		this->pdd = nullptr;

		std::string msg = "SPAWN DISPLAYSPRITE|";
		msg += std::to_string(this->pds->getId());
		msg += "|";
		msg += std::to_string((int) this->pds->getPosition().getX());
		msg += "|";
		msg += std::to_string((int) this->pds->getPosition().getY());
		msg += "|";
		msg += this->personToFollow->getSpriteName();
		const char* msgToSend = msg.c_str(); // 'SPAWN DISPLAYSPRITE'|idOfObject|xCoordinate|yCoordinate|spriteName message
		LM.writeLog("PersonDisplay::PersonDisplay() : sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn PersonDisplaySprite

		msg = "SPAWN DISPLAYNAME|";
		msg += std::to_string(this->pdn->getId());
		msg += "|";
		msg += std::to_string((int) this->pdn->getPosition().getX());
		msg += "|";
		msg += std::to_string((int) this->pdn->getPosition().getY());
		msg += "|";
		msg += "24";
		msg += "|";
		msg += this->personToFollow->getName();
		msgToSend = msg.c_str(); // 'SPAWN DISPLAYNAME'|idOfObject|xCoordinate|yCoordinate|maxChars|personName message
		LM.writeLog("PersonDisplay::PersonDisplay() : sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn PersonDisplayName

		msg = "SPAWN DISPLAYHEALTH|";
		msg += std::to_string((int) this->pdh->getId());
		msg += "|";
		msg += std::to_string((int) this->personToFollow->getCurrHP());
		msg += "|";
		msg += std::to_string((int) this->pdh->getPosition().getX());
		msg += "|";
		msg += std::to_string((int) this->pdh->getPosition().getY());
		msg += "|";
		msg += this->personToFollow->getSpriteName();
		msgToSend = msg.c_str(); // 'SPAWN DISPLAYHEALTH'|healthID|currHP|xCoordinate|yCoordinate message
		LM.writeLog("PersonDisplay::PersonDisplay() : sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn PersonDisplayHealth

		msg = "SPAWN DISPLAYBOMB|";
		msg += std::to_string((int) this->pdb->getId());
		msg += "|";		
		msg += std::to_string((int) this->personToFollow->getCurrBombs());
		msg += "|";
		msg += std::to_string((int) this->pdb->getPosition().getX());
		msg += "|";
		msg += std::to_string((int) this->pdb->getPosition().getY());
		msgToSend = msg.c_str(); // 'SPAWN DISPLAYBOMB'|bombID|currBombs|xCoordinate|yCoordinate message
		LM.writeLog("PersonDisplay::PersonDisplay() : sending custom message: '%s'", msgToSend);
		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn PersonDisplayBomb
	}

	this->updatePowers(this->personToFollow);
}

PersonDisplay::PersonDisplay(const int x, const int y, Person* person) // Constructor
{
	// Set sprite
	setDisplaySprite("personDisplay");

	// Set up variables
	this->originalPos = df::Vector((float) x + getSprite()->getWidth() / 2, (float) y); // Original position of PersonDisplay before shake
	this->shakeCounterCurr = 0; // Current count until shake again
	this->shakeCounterMax = 10; // Current count until shake again
	this->shakeLeft = true; // Is there more shake left?

	this->personToFollow = person; // Person whose stats PersonDisplay displays
	this->personToFollow->setPersonDisplay(this);

	// Register with events
	registerInterest(df::STEP_EVENT); // To update position during shaking

	// Set position
	setPosition(this->originalPos);

	// Display Person sprite
	int xFudge = 2;

	if(NR.getServer() && !GM.getGameOver()) // On Server
	{
		this->pds = new PersonDisplaySprite((int) (this->getPosition().getX() - this->getSprite()->getWidth() / 2.5), y + 1, this->personToFollow->getSprite());
		this->pdn = new PersonDisplayName((int) this->getPosition().getX() - xFudge, y - 1, this->personToFollow->getName(), 24); // 24 allows maximum name to be 18 characters
		this->pdh = new PersonDisplayHealth(true, df::Vector(this->getPosition().getX() - xFudge, (float) y), this->personToFollow->getCurrHP(), 24); // 24 allows maximum HP to be 16
		this->pdb = new PersonDisplayBomb(true, df::Vector(this->getPosition().getX() - xFudge, (float) y + 1), this->personToFollow->getCurrBombs(), 24); // 24 allows maximum bombs to be 17
		this->pdp = new PersonDisplayPower(true, df::Vector(this->getPosition().getX() - xFudge, (float) y + 2), 24);
		this->pdd = nullptr;

		std::string msg = "SPAWN DISPLAYSPRITE|";
		msg += std::to_string((int) this->pds->getPosition().getX());
		msg += "|";
		msg += std::to_string((int) this->pds->getPosition().getY());
		msg += "|";
		msg += this->personToFollow->getSpriteName();
		const char* msgToSend = msg.c_str(); // 'SPAWN DISPLAY'|xCoordinateSprite|yCoordinateSprite|xCoordinateName|yCoordinateName|maxChar|spriteName personName message
		LM.writeLog("PersonDisplay::PersonDisplay() : sending custom message: '%s'", msgToSend);

		NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn PersonDisplaySprite
	}
	this->updatePowers(this->personToFollow);
}

void PersonDisplay::draw() // Draws PersonDisplay's sprite
{
	setAltitude(0);
	df::Object::draw();
}

int PersonDisplay::eventHandler(const df::Event *p_e) // Handles events
{
	if(p_e->getType() == df::STEP_EVENT)
	{
		if(shakeCounterCurr > 0)
		{
			setDisplaySprite("personDisplayDamaged");
			shakeCounterCurr--;
			if(shakeCounterCurr % 2 == 0)
			{
				if(shakeLeft == true)
				{
					shake(df::Vector(-0.5, 0));
					shakeLeft = false;
				}
				else
				{
					shake(df::Vector(0.5, 0));
					shakeLeft = true;
				}
			}
		}
		else
		{
			setDisplaySprite("personDisplay");
			setPosition(this->originalPos);
		}
		return 1;
	}

	return 0;
}

void PersonDisplay::updateBombs(const int bombs) // Updates PersonDisplayBomb
{
	if(pdb != nullptr) this->pdb->updateBombs(bombs);
}

void PersonDisplay::updateHealth(const int newHealth) // Updates PersonDisplayHealth
{
	if(pdh != nullptr)
	{
		if(newHealth < this->pdh->getCurrHealth()) // Damaged
		{
			shakeCounterCurr = shakeCounterMax; // Shake HUD of damaged Person

			// Send message to Client to shake HUD of damaged Person
			std::string msg = "SHAKE DISPLAY|";
			msg += std::to_string(getId());
			msg += "|";
			msg += std::to_string(this->shakeCounterMax);
			msg += "|";
			msg += std::to_string(this->shakeCounterCurr);
			const char* msgToSend = msg.c_str(); // 'SHAKE DISPLAY'|idOfObject|shakeCounterMax|shakeCounterCurr message
			LM.writeLog("PersonDisplay::updateHealth() : sending custom message: '%s'", msgToSend);
			NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to shake PersonDisplay
		}
		this->pdh->updateHealth(newHealth);
	}
	if(newHealth <= 0)
	{
		this->pdd = new PersonDisplayDead((int) this->getPosition().getX(), (int) this->getPosition().getY());

		if(NR.getServer() && !GM.getGameOver()) // On Server
		{
			std::string msg = "SPAWN DISPLAYDEAD|";
			msg += std::to_string(getId());
			msg += "|";
			msg += std::to_string((int) this->getPosition().getX());
			msg += "|";
			msg += std::to_string((int) this->getPosition().getY());
			const char* msgToSend = msg.c_str(); // 'SPAWN DISPLAYDEAD'|idOfObject|xCoordinate|yCoordinate message
			LM.writeLog(" PersonDisplay::updateHealth() : sending custom message: '%s'", msgToSend);
			NR.getServer()->sendMessage(df::CUSTOM_MESSAGE, (int) strlen(msgToSend) + 1, msgToSend); // Send message to Client to spawn PersonDisplayDead
		}
	}
}

void PersonDisplay::updatePowers(Person* personToFollow) // Updates PersonDisplayPower
{
	if(pdp != nullptr) this->pdp->updatePowers(personToFollow);
}

void PersonDisplay::moveTo(const int x, const int y) // Moves PersonDisplay
{
	df::Vector pos((float) x, (float) y);
	float dx = x - getPosition().getX();
	float dy = y - getPosition().getY();

	setPosition(pos);
	if(this->pds != nullptr) this->pds->setPosition(df::Vector(this->pds->getPosition().getX() + dx, this->pds->getPosition().getY() + dy));
	if(this->pdn != nullptr) this->pdn->setPosition(df::Vector(this->pdn->getPosition().getX() + dx, this->pdn->getPosition().getY() + dy));
	if(this->pdh != nullptr) this->pdh->setPosition(df::Vector(this->pdh->getPosition().getX() + dx, this->pdh->getPosition().getY() + dy));
	if(this->pdb != nullptr) this->pdb->setPosition(df::Vector(this->pdb->getPosition().getX() + dx, this->pdb->getPosition().getY() + dy));
	if(this->pdp != nullptr) this->pdp->setPosition(df::Vector(this->pdp->getPosition().getX() + dx, this->pdp->getPosition().getY() + dy));
	if(this->pdd != nullptr) this->pdd->setPosition(df::Vector(this->pdd->getPosition().getX() + dx, this->pdd->getPosition().getY() + dy));

}

void PersonDisplay::shake(df::Vector dist) // Shakes PersonDisplay
{
	setPosition(this->originalPos + dist);
}

void PersonDisplay::setDisplaySprite(std::string spriteName) // Sets PersonDisplay's sprite
{
	df::Sprite *p_temp_sprite;
	p_temp_sprite = RM.getSprite(spriteName);

	if(!p_temp_sprite)
		LM.writeLog("PersonDisplay::setDisplaySprite(): Warning! Sprite '%s' not found", spriteName.c_str());
	else
	{
		setSprite(p_temp_sprite);
		setTransparency();
	}
}

std::string PersonDisplay::sizeStr(std::string str, std::string posBuffer) // Properly sizes string str based on allowed max character length, may concatnate
{
	if(str.length() > posBuffer.length())
	{
		str = str.substr(0, posBuffer.length());
	}
	else
	{
		str += posBuffer.substr(str.length(), posBuffer.length());
	}

	return str;
}

std::string PersonDisplay::serialize(bool all) // Custom serialize for variables
{
	// Do main serialize from parent
	std::string s = Object::serialize(all);

	// Add PersonDisplay-specific attribute
	s += ("originalPosX:" + std::to_string((int) this->originalPos.getX()) + ",");
	s += ("originalPosY:" + std::to_string((int) this->originalPos.getY()) + ",");

	// Return full serialization
	return s;
}

int PersonDisplay::deserialize(std::string str) // Custom deserialize for variables
{
	// Do main deserialize from parent
	Object::deserialize(str);

	// Look for socket index
	float x;
	float y;
	std::string parseForStr = df::match("", "originalPosX");
	if(!parseForStr.empty())
	{
		x = stof(parseForStr);
	}
	parseForStr = df::match("", "originalPosY");
	if(!parseForStr.empty())
	{
		y = stof(parseForStr);
	}

	this->originalPos = df::Vector(x, y);

	return 0;
}

void PersonDisplay::setShakes(const int max, const int curr) // Sets PersonDisplay's current shake and max shake values
{
	this->shakeCounterMax = max;
	this->shakeCounterCurr = curr;
}
