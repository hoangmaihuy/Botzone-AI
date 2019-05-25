// game.h

#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <tuple>
#include <queue>
#include <map>
#include <vector>
#include "board.h"

using namespace std;

const int INF = int(1e9);
const int BotColor = 1;

class Game
{
public:
	bool isRunning;
	Game();
	void ShowMenu();
private:
	Board board;
	int startX, startY, resultX, resultY, obstacleX, obstacleY;
	map<pair<grid_t, int>, bool> data; // save checked state in DeepSearch
	map<pair<grid_t, int>, vector<cell>> step; // save move step for each state
	queue<pair<int, int>> q; // queue for bfs
	int dis[GRID_SIZE][GRID_SIZE];
	void HumanMove(); // handle human move
	void BotMove(); // calculate computer move
	int CalAreaDiff(int color); // calculate area difference
	int MoveArea(int color, int depth); // calculate mobility
	int GreedySearch(int color); 
	bool DeepSearch(int color);
};

#endif // !GAME_H
