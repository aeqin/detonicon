// NetworkLagSimulator.cpp

// Engine includes
#include "NetworkLagSimulator.h"

// Game includes
#include "NetworkRole.h"

int NetworkLagSimulator::getLagInMS() // Gets amount of lag to simulate in milliseconds
{
	return this->lagInMS;
}

void NetworkLagSimulator::setLagInMS(const int ms) // Sets amount of lag to simulate in milliseconds
{
	this->lagInMS = ms;
}

// Captures sendMessage function call meant for NetworkNode
// Adds captured message to queue to be sent after some time (simulate latency)
int NetworkLagSimulator::sendMessage(df::MessageType msg_type, int sock_index)
{/* Message type 1 */

	if(NR.getClient() && this->lagInMS <= 0) return sendMessagePriority(msg_type, sock_index);

	// Construct message struct
	struct message1 msg;
	msg.msg_type = msg_type;
	msg.sock_index = sock_index;
	msg.timeSentEntry = Clock::now();

	q1.push(msg); // Add message to queue

	sendQueuedMessages(); // See if any messages can be sent from the queue

	return 0; // Return nothing, this function only used just to capture the sendMessage function call
}
int NetworkLagSimulator::sendMessage(df::MessageType msg_type, df::Object *p_obj, bool all_attr, int sock_index)
{/* Message type 2 */

	if(NR.getClient() && this->lagInMS <= 0) return sendMessagePriority(msg_type, p_obj, all_attr, sock_index);

	// Construct message struct
	struct message2 msg;
	msg.msg_type = msg_type;
	msg.p_obj = p_obj;
	msg.all_attr = all_attr;
	msg.sock_index = sock_index;
	msg.timeSentEntry = Clock::now();

	q2.push(msg); // Add message to queue
	
	sendQueuedMessages(); // See if any messages can be sent from the queue

	return 0; // Return nothing, this function only used just to capture the sendMessage function call
}
int NetworkLagSimulator::sendMessage(df::MessageType msg_type, df::EventKeyboardAction action, df::Keyboard::Key key, int sock_index)
{/* Message type 3 */

	if(NR.getClient() && this->lagInMS <= 0) return sendMessagePriority(msg_type, action, key, sock_index);

	// Construct message struct
	struct message3 msg;
	msg.msg_type = msg_type;
	msg.action = action;
	msg.key = key;
	msg.sock_index = sock_index;
	msg.timeSentEntry = Clock::now();

	q3.push(msg); // Add message to queue
	

	sendQueuedMessages(); // See if any messages can be sent from the queue

	return 0; // Return nothing, this function only used just to capture the sendMessage function call
}
int NetworkLagSimulator::sendMessage(df::MessageType msg_type, df::EventMouseAction action, df::Mouse::Button button, df::Vector mouse_position, int sock_index)
{/* Message type 4 */

	if(NR.getClient() && this->lagInMS <= 0) return sendMessagePriority(msg_type, action, button, mouse_position, sock_index);

	// Construct message struct
	struct message4 msg;
	msg.msg_type = msg_type;
	msg.action = action;
	msg.button = button;
	msg.mouse_position = mouse_position;
	msg.sock_index = sock_index;
	msg.timeSentEntry = Clock::now();

	q4.push(msg); // Add message to queue

	sendQueuedMessages(); // See if any messages can be sent from the queue

	return 0; // Return nothing, this function only used just to capture the sendMessage function call
}
int NetworkLagSimulator::sendMessage(df::MessageType msg_type, int num_bytes, const void *bytes, int sock_index)
{/* Message type 5 */

	if(NR.getClient() && this->lagInMS <= 0) return sendMessagePriority(msg_type, num_bytes, bytes, sock_index);

	std::string msgStr((char*) bytes); // Convert char* to a std:string

	// Construct message struct
	struct message5 msg;
	msg.msg_type = msg_type;
	msg.num_bytes = num_bytes;
	msg.bytes = msgStr;
	msg.sock_index = sock_index;
	msg.timeSentEntry = Clock::now();

	q5.push(msg); // Add message to queue

	sendQueuedMessages(); // See if any messages can be sent from the queue

	return 0; // Return nothing, this function only used just to capture the sendMessage function call
}

// Captures sendMessage function call meant for NetworkNode
// Sends message immediately, bypassing simulated lag
int NetworkLagSimulator::sendMessagePriority(df::MessageType msg_type, int sock_index)
{/* Message type 1 */
	return NetworkNode::sendMessage(msg_type, sock_index);
}
int NetworkLagSimulator::sendMessagePriority(df::MessageType msg_type, df::Object *p_obj, bool all_attr, int sock_index)
{/* Message type 2 */
	return NetworkNode::sendMessage(msg_type, p_obj, all_attr, sock_index);
}
int NetworkLagSimulator::sendMessagePriority(df::MessageType msg_type, df::EventKeyboardAction action, df::Keyboard::Key key, int sock_index)
{/* Message type 3 */
	return NetworkNode::sendMessage(msg_type, action, key, sock_index);
}
int NetworkLagSimulator::sendMessagePriority(df::MessageType msg_type, df::EventMouseAction action, df::Mouse::Button button, df::Vector mouse_position, int sock_index)
{/* Message type 4 */
	return NetworkNode::sendMessage(msg_type, action, button, mouse_position, sock_index);
}
int NetworkLagSimulator::sendMessagePriority(df::MessageType msg_type, int num_bytes, const void *bytes, int sock_index)
{/* Message type 5 */
	return NetworkNode::sendMessage(msg_type, num_bytes, bytes, sock_index);
}

void NetworkLagSimulator::sendQueuedMessages() // Sends any queued messages after some time has passed (to simulate latency)
{
	// timeSendEntry : time when the sendMessage() function was first called in Client or Server
	time_point<Clock> timeSentExit = Clock::now(); // Time right now, possibly when the message should exit the lag simulator and actually be called in the NetworkNode

	std::map<int, ClientInfo*> sockToClient; // Gets mapping of Client sockets to important Client information
	if(NR.getServer()) sockToClient = NR.getServer()->getClientPersons();

	bool isServer = NR.getServer();
	
	while(!q1.empty())
	{
		struct message1 msg = q1.front();
		int lag = this->lagInMS; // Get lag of network node (Client or Server)
		if(isServer && sockToClient.count(msg.sock_index) != 0) // Server should have 0 lag, so lag is the lag of the Client the Server is trying to send to
		{
			lag += sockToClient[msg.sock_index]->getLagInMS(); // Grab the lag of the Client being sent the message
		}
		if(duration_cast<ms>(timeSentExit - msg.timeSentEntry).count() >= lag) // If the difference in time is greater or equal to the set amount of lag, send the message
		{
			NetworkNode::sendMessage(msg.msg_type, msg.sock_index);
			q1.pop(); // Pop off the front of the queue, the message that was just called
		}
		else break; // It is not time to send any message, stop checking this queue
	}
	while(!q2.empty())
	{
		struct message2 msg = q2.front();
		int lag = this->lagInMS;
		if(isServer && sockToClient.count(msg.sock_index) != 0) // Server should have 0 lag, so lag is the lag of the Client the Server is trying to send to
		{
			lag += sockToClient[msg.sock_index]->getLagInMS(); // Grab the lag of the Client being sent the message
		}
		if(duration_cast<ms>(timeSentExit - msg.timeSentEntry).count() >= lag) // If the difference in time is greater or equal to the set amount of lag, send the message
		{
			NetworkNode::sendMessage(msg.msg_type, msg.p_obj, msg.all_attr, msg.sock_index);
			q2.pop(); // Pop off the front of the queue, the message that was just called
		}
		else break; // It is not time to send any message, stop checking this queue
	}
	while(!q3.empty())
	{
		struct message3 msg = q3.front();
		int lag = this->lagInMS;
		if(isServer && sockToClient.count(msg.sock_index) != 0) // Server should have 0 lag, so lag is the lag of the Client the Server is trying to send to
		{
			lag += sockToClient[msg.sock_index]->getLagInMS(); // Grab the lag of the Client being sent the message
		}
		if(duration_cast<ms>(timeSentExit - msg.timeSentEntry).count() >= lag) // If the difference in time is greater or equal to the set amount of lag, send the message
		{
			NetworkNode::sendMessage(msg.msg_type, msg.action, msg.key, msg.sock_index);
			q3.pop(); // Pop off the front of the queue, the message that was just called
		}
		else break; // It is not time to send any message, stop checking this queue
	}
	while(!q4.empty())
	{
		struct message4 msg = q4.front();
		int lag = this->lagInMS;
		if(isServer && sockToClient.count(msg.sock_index) != 0) // Server should have 0 lag, so lag is the lag of the Client the Server is trying to send to
		{
			lag += sockToClient[msg.sock_index]->getLagInMS(); // Grab the lag of the Client being sent the message
		}
		if(duration_cast<ms>(timeSentExit - msg.timeSentEntry).count() >= lag) // If the difference in time is greater or equal to the set amount of lag, send the message
		{
			NetworkNode::sendMessage(msg.msg_type, msg.action, msg.button, msg.mouse_position, msg.sock_index);
			q4.pop(); // Pop off the front of the queue, the message that was just called
		}
		else break; // It is not time to send any message, stop checking this queue
	}
	while(!q5.empty())
	{
		struct message5 msg = q5.front();
		int lag = this->lagInMS;
		if(duration_cast<ms>(timeSentExit - msg.timeSentEntry).count() >= lag) // If the difference in time is greater or equal to the set amount of lag, send the message
		{
			NetworkNode::sendMessage(msg.msg_type, msg.num_bytes, msg.bytes.c_str(), msg.sock_index);
			q5.pop(); // Pop off the front of the queue, the message that was just called
		}
		else break; // It is not time to send any message, stop checking this queue
	}
}

void NetworkLagSimulator::receiveQueuedMessages(df::EventCustomNetwork* msg) // Tries to open any received EventCustomNetworks after some time has passed (to simulate latency)
{
	if(NR.getServer()) return; // Only Client should use the function as Server has no lag

	if(this->lagInMS <= 0 && msg != nullptr) // Has 0 simulated lag, so immediately parse message
	{
		NR.getClient()->parseCustomMsg(msg->getMessage()); // Read message
	}
	else
	{
		time_point<Clock> currTime = Clock::now(); // Time right now, possibly when the Client should read the message

		if(msg != nullptr)
		{
			struct messageR m;
			std::string msgStr((char*) msg->getMessage());
			m.msg = msgStr;
			m.timeReceived = currTime;

			qR.push(m);
		}

		while(!qR.empty())
		{
			struct messageR m = qR.front();

			if(duration_cast<ms>(currTime - m.timeReceived).count() >= this->lagInMS)
			{
				NR.getClient()->parseCustomMsg(m.msg.c_str()); // Read message
				qR.pop();
			}
			else break; // It is not time to read any message, stop checking this queue
		}
	}
}