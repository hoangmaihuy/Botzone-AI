#ifndef BOARD_H 
#define BOARD_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

typedef vector<vector<int>> grid_t;
typedef pair<int, int> cell;

const int GRID_SIZE = 8;
const string SAVE_FILE = "save_board.txt";
const char BLACK_CHAR = 'B';
const char WHITE_CHAR = 'W';
const char OBSTACLE_CHAR = char(254);
const int dx[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
const int dy[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };

class Board
{
private:
	grid_t grid; // use 2D int array to save grid information

public:
	int turnID;
	Board();
	Board(bool isNewGame); // Board's constructor, isNewGame == false to load game from file
	int GetCell(int x, int y); // return cell information
	grid_t GetGrid(); // return all grid
	vector<cell> pos[2]; // pos[0/1] is white/black chess location 
	bool InGrid(int x, int y); 
	bool OnStraghtLine(int x1, int y1, int x2, int y2);
	bool HaveObstacleBetween(int x1, int y1, int x2, int y2);
	void DrawLine(int length);
	void DrawBoard();
	void SaveBoard(); // save board information to SAVE_FILE 
	bool ValidMove(int x1, int y1, int x2, int y2, int x3, int y3, int color); // check valid move
	void ApplyMove(int x1, int y1, int x2, int y2, int x3, int y3, bool revert); // apply move to board, can revert move
	bool LoseState(int color); // check if color in lose state
};

#endif 




