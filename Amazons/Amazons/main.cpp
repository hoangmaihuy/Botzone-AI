// main.cpp

#include "pch.h"
#include "game.h"
#include <iostream>

int main()
{
	Game game; // create game
	while (game.isRunning)
	{
		game.ShowMenu();
	}
}