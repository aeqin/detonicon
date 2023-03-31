// Player.h

// Engine includes
#include "EventKeyboard.h"

// Game includes
#include "Person.h"

class Player : public Person 
{
 private:
	void kbd(const df::EventKeyboard *p_keyboard_event); // Handles keyboard input
	void step(); // Increments counters each frame

 public:
	Player(PersonDisplay* display, std::string name, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite); // Constructor
	
	int eventHandler(const df::Event *p_e); // Handles events
};
