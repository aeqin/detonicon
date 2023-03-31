//
// BotAdvanced.h
//

#include "EventKeyboard.h"
#include "Object.h"
#include "Person.h"

class BotAdvanced : public Person {

private:
	void botAdvancedAI();
	void performFight();
	void performWander();
	void performRun();
	void performExplore();
	int braveVal;
	int smartVal;

	struct Move {
		int x;
		int y;
	};
	
	struct Move prevMove = { 0, 0 };
	bool flagStandby;
	int standbyCount = false;

	int moveToTarget(df::Vector target, int radius);
	bool hasVisited(df::Vector position, std::vector<df::Vector> &array);
	void step();

protected:

public:
	BotAdvanced(PersonDisplay* display, std::string name, const int id, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite, int braveValInput, int smartValInput);
	int eventHandler(const df::Event *p_e);

	// Custom networking
	std::string serialize(bool all = false); // Custom serialize for variables
	int deserialize(std::string str); // Custom deserialize for variables
};
