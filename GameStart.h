// GameStart.h

#pragma once

// Engine includes
#include "Music.h"
#include "ViewObject.h"

#define GS GameStart::getInstance() // Two-letter acronym for easier access to GameStart

class GameStart : public df::ViewObject
{
private:
	int mapID; // Map id to spawn
	bool isGameOver; // Is the game over?
	bool isGameAutoStart = false; // Should the game be auto started once specified # Clients connected?
	df::Sprite* spriteStart = nullptr; // Sprite of GameStart
	df::Sprite* spriteInstructions = nullptr; // Sprite of GameStart (instructions screen)
	df::Sprite* spriteLoading = nullptr; // Sprite of GameStart (loading screen)
	df::Music* titleScreenMusic = nullptr; // Title screen music

	GameStart(); // Constructor, private for singleton
	void start(); // Sets up game for player

public:
	// Friend class to have Server call start() function
	friend class Server;

	static GameStart &getInstance(); // Get the one and only instance of the GameStart

	// Variable getters
	bool getIsGameOver(); // Returns whether or not game is over

	// Variable setters
	void setGameOver(const bool isGameOver); // Sets whether or not game is over
	void setMap(const int mapID); // Sets map id
	void setGameAutoStart(const bool isGameAutoStart); // Sets whether or not the game is auto start

	void draw(); // Draw GameStart object
	int eventHandler(const df::Event *p_e); // Handles events
	void setTitleSprite(); // Sets GameStart sprite to spriteStart
	void playTitleScreenMusic(); // Plays title music
	void pauseTitleScreenMusic(); // Pause title music
};