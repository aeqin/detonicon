//
// BotSmart.h
//

#include "EventKeyboard.h"
#include "Object.h"
#include "Person.h"

class BotSmart : public Person {

private:
	void botSmartAI(int fightVal, int runVal, int exploreVal);
	void performFight();
	void performRun();
	void performExplore();

	struct Move {
		int x;
		int y;
	};
	
	struct Move prevMove = { 0, 0 };
	bool flagStandby;
	int standbyCount = false;

	int moveToTarget(df::Vector target, int radius);
	bool hasVisited(df::Vector position, df::Vector* array, int size);
	void step();

protected:

public:
	BotSmart(PersonDisplay* display, std::string name, const int id, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite);
	int eventHandler(const df::Event *p_e);
};
