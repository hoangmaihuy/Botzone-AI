#include "pch.h"
#include "board.h"
#include <iostream>

Board::Board() {}

Board::Board(bool isNewGame)
{
	if (isNewGame)
	{
		turnID = 0;
		for (int i = 0; i < GRID_SIZE; i++)
		{
			vector<int> tmp;
			for (int j = 0; j < GRID_SIZE; j++)
				tmp.emplace_back(0);
			grid.emplace_back(tmp);
		}
		// Init black and white's chess position
		grid[0][2] = grid[2][0] = grid[5][0] = grid[7][2] = 1;
		grid[0][5] = grid[2][7] = grid[5][7] = grid[7][5] = -1;
		pos[0].clear(); pos[1].clear();
		// add chess's position to pos
		for (int x = 0; x < GRID_SIZE; x++)
			for (int y = 0; y < GRID_SIZE; y++)
			{
				if (grid[x][y] == 1) pos[1].emplace_back(x, y);
				else if (grid[x][y] == -1) pos[0].emplace_back(x, y);
			}
	} 
	else
	{
		// Load game data from file
		ifstream fin(SAVE_FILE);
		if (!fin.good())
		{
			grid.push_back(vector<int>(1));
			return;
		}
		fin >> turnID;
		grid.clear();
		for (int i = 0; i < GRID_SIZE; i++)
		{
			vector<int> tmp;
			tmp.clear();
			for (int j = 0; j < GRID_SIZE; j++)
			{
				int x;
				fin >> x;
				tmp.push_back(x);
			}
			grid.push_back(tmp);
		}
		fin.close();
		// add chess's position to pos
		pos[0].clear(); pos[1].clear();
		for (int x = 0; x < GRID_SIZE; x++)
		{
			for (int y = 0; y < GRID_SIZE; y++)
			{
				if (grid[x][y] == 1) pos[1].emplace_back(x, y);
				else if (grid[x][y] == -1) pos[0].emplace_back(x, y);
			}
		}
	}
}

void Board::DrawLine(int len)
{
	cout << "  +";
	for (int i = 0; i < len; i++)
		cout << "---+";
	cout << "\n";
}

void Board::DrawBoard()
{
	system("cls");
	cout << "    "; for (int x = 0; x < GRID_SIZE; x++) cout << x << "   "; cout << "\n";
	DrawLine(GRID_SIZE);
	
	for (int y = 0; y < GRID_SIZE; y++)
	{
		cout << y << " |";
		for (int x = 0; x < GRID_SIZE; x++)
		{
			char ch;
			switch (grid[x][y])
			{
			case 1:
				ch = BLACK_CHAR; break;
			case -1:
				ch = WHITE_CHAR; break;
			case 2:
				ch = OBSTACLE_CHAR; break;
			default:
				ch = ' '; break;
			}
			cout << ' ' << ch << " |";
		}
		cout << "\n";
		DrawLine(GRID_SIZE);
	}
	cout << "____________________________________\n";
}

void Board::SaveBoard()
{
	// Save board data to SAVE_FILE
	ofstream fout(SAVE_FILE);
	fout << turnID << "\n";
	for (int i = 0; i < GRID_SIZE; i++)
	{
		for (int j = 0; j < GRID_SIZE; j++) fout << grid[i][j] << ' ';
		fout << '\n';
	}
	fout.close();
}

bool Board::InGrid(int x, int y)
{
	return (x >= 0 && y >= 0 && x < GRID_SIZE && y < GRID_SIZE);
}

bool Board::OnStraghtLine(int x1, int y1, int x2, int y2)
{
	return (x1 == x2 || y1 == y2 || x1 + y1 == x2 + y2 || x1 - y1 == x2 - y2);
}

bool Board::HaveObstacleBetween(int x1, int y1, int x2, int y2)
{
	if (x1 == x2) // same row
	{
		for (int k = min(y1, y2) + 1; k < max(y1, y2); k++)
		{
			if (grid[x1][k] != 0) return true;
		}
	}
	else if (y1 == y2) // same column
	{
		for (int k = min(x1, x2) + 1; k < max(x1, x2); k++)
		{
			if (grid[k][y1] != 0) return true;
		}
	}
	else if (x1 - y1 == x2 - y2) // check diagonal
	{
		if (x1 > x2)
		{
			swap(x1, x2); swap(y1, y2);
		}
		for (int k = 1; k < x2 - x1; k++)
			if (grid[x1 + k][y1 + k] != 0) return true;
	}
	else if (x1 + y1 == x2 + y2)
	{
		if (x1 > x2)
		{
			swap(x1, x2); swap(y1, y2);
		}
		for (int k = 1; k < x2 - x1; k++)
			if (grid[x1 + k][y1 - k] != 0) return true;
	}
	return false;
}

bool Board::ValidMove(int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
	if (!InGrid(x1, y1) || !InGrid(x2, y2) || !InGrid(x3, y3)) return false; 
	if (grid[x1][y1] != color || grid[x2][y2] != 0) return false; 
	if (grid[x2][y2] != 0 && !(x3 == x1 && y3 == y1)) return false;
	if (!OnStraghtLine(x1, y1, x2, y2) || !OnStraghtLine(x2, y2, x3, y3)) return false;
	ApplyMove(x1, y1, x2, y2, x3, y3, false);
	bool flag = HaveObstacleBetween(x1, y1, x2, y2) || HaveObstacleBetween(x2, y2, x3, y3);
	ApplyMove(x1, y1, x2, y2, x3, y3, true);
	return !flag;
}

void Board::ApplyMove(int x1, int y1, int x2, int y2, int x3, int y3, bool revert)
{
	int t;
	if (!revert)
	{
		t = grid[x1][y1];
		if (t == -1) t = 0;
		swap(grid[x1][y1], grid[x2][y2]);
		grid[x3][y3] = 2;
		for (int i = 0; i < pos[t].size(); i++)
			if (pos[t][i] == cell(x1, y1)) pos[t][i] = cell(x2, y2);
	}
	else
	{
		grid[x3][y3] = 0;
		swap(grid[x1][y1], grid[x2][y2]);
		t = grid[x1][y1];
		if (t == -1) t = 0;
		for (int i = 0; i < pos[t].size(); i++)
			if (pos[t][i] == cell(x2, y2)) pos[t][i] = cell(x1, y1);
	}
}

bool Board::LoseState(int color) // check if (color) has any move left
{
	for (int x1 = 0; x1 < GRID_SIZE; x1++)
	for (int y1 = 0; y1 < GRID_SIZE; y1++)
	if (grid[x1][y1] == color)
	{
		for (int k1 = 0; k1 < 8; k1++)
		for (int d1 = 1; d1 < 8; d1++)
		{
			int x2 = x1 + dx[k1] * d1, y2 = y1 + dy[k1] * d1;
			if (!InGrid(x2, y2)) break;
			if (grid[x2][y2] != 0) break;
			for (int k2 = 0; k2 < 8; k2++)
			for (int d2 = 1; d2 < 8; d2++)
			{
				int x3 = x2 + dx[k2] * d2, y3 = y2 + dy[k2] * d2;
				if (!InGrid(x3, y3)) break;
				if (grid[x3][y3] != 0 && !(x3 == x1 && y3 == y1)) break;
				return false;
			}
		}
	}
	return true;
}

int Board::GetCell(int x, int y)
{
	return grid[x][y];
}

grid_t Board::GetGrid()
{
	return grid;
}