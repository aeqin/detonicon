// ManagerPerson.h

#pragma once

// System includes
#include <string>
#include <vector>

// Game includes
#include "Person.h"

#define MPer ManagerPerson::getInstance() // Two-letter acronym for easier access to manager

class ManagerPerson : df::Object
{
	private:
		std::vector<Person*> people; // Vector of all Persons in game
		std::vector<int> peopleObjectIds; // Vector of all Persons in game by ObjectId
		std::vector<df::Vector> spawns; // Vector to hold all spawn points of the current map
		int nextID; // Used to ensure a unique ID for every player
		int freezeCount; // Used to have a "freeze-frame" moment on game end
		int numPersons; // How many Persons are in the game
		std::string peopleStr = ""; // String message used by Server to send Person vector to Client
		bool peopleSet = false; // Used by Client to attempt to set People from Server each frame until properly set

		ManagerPerson(); // Constructor, private for singleton

	public:
		static ManagerPerson &getInstance(); // Get the one and only instance of the ManagerPerson

		// Variable getters
		std::vector<df::Vector> getSpawns(); // Returns vector of spawn points on current map
		std::vector<Person*> getPeople(); // Returns vector of all Persons in game
		int getPersonCount(); // Returns the number of Persons that played in game

		void newGameSpawn(const float xBound, const float yBound, std::vector<df::Vector> spawnLocations); // Spawns all the Persons on game begin
		int eventHandler(const df::Event* p_e); // Handles events
		void step(); // Checks for game end each frame

		Person* getPerson(int id); // Returns Person that matches given ID
		Person* getPerson(std::string name); // Returns Person that matches given name
		Person* getClosestPerson(Person* start); // Returns closest Person to given Person
		Person* getRandomPerson(Person* start); // Returns a random Person that is not given Person
		float getDistance(Person* start, Person* end); // Returns distance between two Persons
		df::Vector getClosestBomb(Person* player); // Returns closest Bomb to given Person
		df::Vector getClosestPowerUp(Person* player); // Returns closest PowerUp to given Person
		bool checkBombDanger(df::Vector playerPos, int smartVal); // Returns whether or not a Person is in danger of explosion

		// Custom Networking
		bool setPeople(std::string peopleMsg); // Used by Client to set People, returns true if all people successfully set
		void removePerson(int personID); // Used by Client to remove Person that has died from people vector
};