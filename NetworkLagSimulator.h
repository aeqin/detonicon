// NetworkLagSimulator.h

#pragma once

// System includes
#include <chrono>
#include <iostream>
#include <queue>

// Engine includes
#include "EventCustomNetwork.h"
#include "NetworkNode.h"

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using ms = std::chrono::milliseconds;

class NetworkLagSimulator : public df::NetworkNode
{
private:
	int numClients;

	// Structs that hold the parameters needed for a particular type of sendMessage call of NetworkNode
	// Also holds the time that the message should be sent to simulate lag
	struct message1
	{
		df::MessageType msg_type;
		int sock_index;
		time_point<Clock> timeSentEntry;
	};
	struct message2
	{
		df::MessageType msg_type;
		df::Object *p_obj;
		bool all_attr;
		int sock_index;
		time_point<Clock> timeSentEntry;
	};
	struct message3
	{
		df::MessageType msg_type;
		df::EventKeyboardAction action;
		df::Keyboard::Key key;
		int sock_index;
		time_point<Clock> timeSentEntry;
	};
	struct message4
	{
		df::MessageType msg_type;
		df::EventMouseAction action;
		df::Mouse::Button button;
		df::Vector mouse_position;
		int sock_index;
		time_point<Clock> timeSentEntry;
	};
	struct message5
	{
		df::MessageType msg_type;
		int num_bytes;
		std::string bytes;
		int sock_index = -1;
		time_point<Clock> timeSentEntry;
	};

	// Separate queues to hold each sendMessage call
	std::queue<struct message1> q1;
	std::queue<struct message2> q2;
	std::queue<struct message3> q3;
	std::queue<struct message4> q4;
	std::queue<struct message5> q5;
	
	// Struct to hold received custom message
	struct messageR
	{
		std::string msg;
		time_point<Clock> timeReceived;
	};

	// Queue to hold custom messages
	std::queue<struct messageR> qR;

protected:
	int lagInMS = 0; // Amount of lag to simulate in milliseconds

public:

	// Variable getters
	int getLagInMS(); // Gets amount of lag to simulate in milliseconds

	// Variable setters
	void setLagInMS(const int ms); // Sets amount of lag to simulate in milliseconds

	// Captures sendMessage function call meant for NetworkNode
	// Adds captured message to queue to be sent after some time (simulate latency)
	/* Message type 1 */ int sendMessage(df::MessageType msg_type, int sock_index = -1);
	/* Message type 2 */ int sendMessage(df::MessageType msg_type, df::Object *p_obj, bool all_attr = false, int sock_index = -1);
	/* Message type 3 */ int sendMessage(df::MessageType msg_type, df::EventKeyboardAction action, df::Keyboard::Key key, int sock_index = -1);
	/* Message type 4 */ int sendMessage(df::MessageType msg_type, df::EventMouseAction action, df::Mouse::Button button, df::Vector mouse_position, int sock_index = -1);
	/* Message type 5 */ int sendMessage(df::MessageType msg_type, int num_bytes, const void *bytes, int sock_index = -1);

	// Captures sendMessage function call meant for NetworkNode
	// Sends message immediately, bypassing simulated lag
	/* Message type 1 */ int sendMessagePriority(df::MessageType msg_type, int sock_index = -1);
	/* Message type 2 */ int sendMessagePriority(df::MessageType msg_type, df::Object *p_obj, bool all_attr = false, int sock_index = -1);
	/* Message type 3 */ int sendMessagePriority(df::MessageType msg_type, df::EventKeyboardAction action, df::Keyboard::Key key, int sock_index = -1);
	/* Message type 4 */ int sendMessagePriority(df::MessageType msg_type, df::EventMouseAction action, df::Mouse::Button button, df::Vector mouse_position, int sock_index = -1);
	/* Message type 5 */ int sendMessagePriority(df::MessageType msg_type, int num_bytes, const void *bytes, int sock_index = -1);

	void sendQueuedMessages(); // Sends any queued messages after some time has passed (to simulate latency)
	void receiveQueuedMessages(df::EventCustomNetwork* msg = nullptr); // Tries to open any received EventCustomNetworks after some time has passed (to simulate latency)
};