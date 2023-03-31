// Wall.h

#pragma once

class Wall : public df::Object
{
private:
	df::Vector givenPos; // Position it was meant to be spawned in
	bool atGivenPos; // Is Wall at the position it was meant to be spawned in?
	std::string spriteName; // Name of wall sprite

public:
	Wall(bool isOnServer, std::string spriteName, df::Vector pos, const int id = -1); // Constructor, only creates Object if Server
	void initOnServer(std::string spriteName, df::Vector pos, const int id = -1); // Create Object on Server

	Wall(const int x, const int y); // Constructor, single char Wall
	Wall(const int x, const int y, std::string spriteName); // Constructor, different Wall sprite

	~Wall(); // Destructor
};