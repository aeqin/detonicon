//
// BotSimple.h
//

#include "EventKeyboard.h"
#include "Object.h"
#include "Person.h"

class BotSimple : public Person {

private:
	void botSimpleAI();
	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;

	int prevMoveX;
	int prevMoveY;

	void kbd(const df::EventKeyboard *p_keyboard_event);
	void step();

protected:

public:
	BotSimple(PersonDisplay* display, std::string name, const int id, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite);
	int eventHandler(const df::Event *p_e);
};
