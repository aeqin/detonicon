// MiniExplosion.h

#pragma once

// System includes
#include <string>

// Engine includes
#include "EventCollision.h"

class MiniExplosion : public df::Object
{
private:
	int maxStayTime; // How long MiniExplosion lasts before fading
	int stayTime; // Current time until MiniExplosion fades

public:
	MiniExplosion(bool isOnServer, df::Vector pos, const int stayTime); // Constructor, only creates Object if Server
	void initOnServer(df::Vector pos, const int stayTime); // Create Object on Server

	MiniExplosion(const int x, const int y, const int stayTime); // Constructor
	
	int eventHandler(const df::Event *p_e); // Handles events
	void tick(); // Updates time until MiniExplosion fades every frame
	void collision(const df::EventCollision *p_collision_event); // Handles collisions

	~MiniExplosion(); // Destructor
};