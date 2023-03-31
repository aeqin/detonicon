// MiniHP.h

#pragma once

// Engine includes
#include "Object.h"

class MiniHP : public df::Object
{
private:
	df::Object* leader; // What object MiniHP follows
	int leaderObjectID = -1; // ObjectID of what object MiniHP follows
	int xAway; // X margin MiniHP follows leader
	int yAway; // Y margin MiniHP follows leader
	
public:
	MiniHP(bool isOnServer, df::Object* leader, const int xAway, const int yAway); // Constructor, only creates Object if Server
	void initOnServer(df::Object* leader, const int xAway, const int yAway); // Create Object on Server

	MiniHP(df::Object* leader, const int xAway, const int yAway); // Constructor

	// Variable getters
	df::Object* getLeader(); // Gets leader

	int eventHandler(const df::Event *p_e); // Handles events
	void tick(); // Updates position to follow leader every frame

	// Custom networking
	std::string serialize(bool all = false); // Custom serialize for variables
	int deserialize(std::string str); // Custom deserialize for variables

	~MiniHP(); // Destructor
};