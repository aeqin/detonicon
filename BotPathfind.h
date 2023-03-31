//
// BotPathfind.h
//

#include "EventKeyboard.h"
#include "Object.h"
#include "Person.h"
#include <stack>

class BotPathfind : public Person {
#define ROW 40 // For 2D array use 20
#define COL 98 // For 2D array use 49

private:
	// Global variables for bot attributes
	int braveVal;
	int smartVal;
	bool checkStandby;
	int standbyCount;
	int qualityCheckVal = 0;
	df::Vector previousPos;

	std::stack<df::Vector> path;						// Array of vector to describe a path
	const int maxPathLength = 50;						// Max length of array

	void botAI();										// Control the behavior of the bot
	void moveDefensive();								// Make a defensive move (find the safest location)
	void moveSelfish();									// Make a move for self (power up)
	void moveOffensive();								// Make an offensive move (find the target to attack)
	//void moveRandom();								// Make a semi-random move
	void step();										// Step function
	bool hasVisited(df::Vector position, std::vector<df::Vector> &array);
	int followPath(df::Vector target, int radius);

	// Struct to hold necessary parameters
	struct cell {
		// Row and Column of the parent
		int parent_i, parent_j;
		// f = g + h
		double f, g, h;
		// WallBreakable Check
		//bool isBreakable;
	};

	struct Pair {
		double f;
		df::Vector loc;
	};

	// A shortcut for pair<int, Vector>> type
	//typedef std::pair<double, df::Vector> Pair;

	bool checkValid(df::Vector pos);
	bool checkAccessible(df::Vector pos);
	bool checkPassable(df::Vector pos);
	bool checkArrival(df::Vector pos, df::Vector dest);
	double calculateHDist(df::Vector pos, df::Vector dest);
	void clearPathStack();
	void generatePath(cell map[][COL], df::Vector dest);
	void aStarSearchWithObstruction(df::Vector src, df::Vector dest);
	void aStarSearchNoObstruction(df::Vector src, df::Vector dest);
	void cleanPath();
	void drawPath();
	std::string serialize(bool all);
	int deserialize(std::string str);

protected:

public:
	BotPathfind(PersonDisplay* display, std::string name, const int maxHP, const int maxBombs, const int x, const int y, df::Sprite* sprite, int braveInput, int smartInput);
	int eventHandler(const df::Event *p_e);
};
