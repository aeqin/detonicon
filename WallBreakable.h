// WallBreakable.h

#pragma once

class WallBreakable : public df::Object
{
private:
	df::Vector spawnPos; // Position WallBreakable was spawned
	
	// Custom networking
	void setPositionDeserialized(df::Vector pos); // Sets position of Wall after deserialization

public:
	WallBreakable(bool isOnServer, df::Vector pos); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos); // Create Object on Server

	WallBreakable(const int x, const int y); // Constructor

	~WallBreakable(); // Destructor, chance to spawn PowerUp
};