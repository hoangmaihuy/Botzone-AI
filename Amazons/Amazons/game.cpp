#include "pch.h"
#include "game.h"
#include <sstream>

Game::Game()
{
	isRunning = true;
	cout << "====================================================\n";
	cout << "           Welcome to Game of the Amazons\n";
	cout << "====================================================\n";
	cout << "1. New game\n";
	cout << "2. Continue last game\n";
	cout << "3. Quit game\n";
	cout << "Please select [1/2/3]: ";
	string line;
	int ops;
	while (true)
	{
		getline(cin, line);
		if (line != "1" && line != "2" && line != "3")
		{
			cout << "Invalid input! Please re-select [1/2/3]: ";
		}
		else
		{
			stringstream st(line);
			st >> ops;
			break;
		}
	}
	switch (ops)
	{
	case 1:
		board = Board(true);
		board.DrawBoard();
		ShowMenu();
		break;
	case 2:
		board = Board(false);
		if (board.GetGrid().size() < GRID_SIZE)
		{
			cout << "There is no saved game available! Please start a new game.";
			isRunning = false;
			break;
		}
		board.DrawBoard();
		ShowMenu();
		break;
	case 3:
		isRunning = false;
		break;
	default:
		break;
	}
}

void Game::HumanMove()
{
	board.turnID++;
	int x1, y1, x2, y2, x3, y3;
	cout << "You are White [W]\n";
	cout << "Enter move [x1 y1 x2 y2 x3 y3]: ";
	string line;
	while (true)
	{
		getline(cin, line);
		bool ok = true;
		int space = 0;
		for (int i = 0; i < line.size(); i++)
		{
			if (line[i] == ' ') space++;
			else if (line[i] < '0' || line[i] > '9') ok = false;
		}
		// valid input only contains digit and has 6 numbers
		if (!ok || space != 5)
		{
			cout << "Invalid input! Please re-enter [x1 y1 x2 y2 x3 y3]: ";
		}
		else
		{
			stringstream st(line);
			st >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;
			// check valid move then apply
			if (board.ValidMove(x1, y1, x2, y2, x3, y3, -BotColor))
			{
				board.ApplyMove(x1, y1, x2, y2, x3, y3, false);
				break;
			}
			else
			{
				cout << "Invalid move! Please re-enter [x1 y1 x2 y2 x3 y3]: ";
			}
		}
	}
}

// Calculate mobility after (depth) moves
int Game::MoveArea(int color, int depth) 
{
	int area = 0;
	memset(dis, 0, sizeof dis);

	int t = color; if (t == -1) t = 0;
	for (int i = 0; i < board.pos[t].size(); i++)
	{
		q.push(board.pos[t][i]);
	}
	// Use BFS to calculate the area
	while (!q.empty())
	{
		int i, j;
		tie(i, j) = q.front(); q.pop();
		for (int k = 0; k < 8; k++)
			for (int d = 1; d < 8; d++)
			{
				int u = i + dx[k] * d, v = j + dy[k] * d;
				if (!board.InGrid(u, v)) break;
				if (board.GetCell(u, v) != 0) break;
				if (dis[u][v] == 0)
				{
					dis[u][v] = dis[i][j] + 1;
					area++;
					if (dis[u][v] < depth)
						q.push(cell(u, v));
				}
			}
	}
	return area;
}

int Game::CalAreaDiff(int color)
{
	if (board.turnID <= 15) // in the first 15 turns, mobility is calculated by queen's move
	{
		return MoveArea(color, 1) - MoveArea(-color, 1);
	}
	else // turn 16-22, calculate the number of cell each color can reach
	{
		return MoveArea(color, INF) - MoveArea(-color, INF);
	}
}


// Use Minimax Algorithm, maximize self's best move and minimize opponent's best move
// Only calculate depth = 1
int Game::GreedySearch(int color) 
{
	int rx1, ry1, rx2, ry2, rx3, ry3, best = -INF;
	int t = color; if (t == -1) t = 0;
	for (int i = 0; i < board.pos[t].size(); i++)
	{
		// iterate all chess's position x1 y1
		int x1 = board.pos[t][i].first, y1 = board.pos[t][i].second;
		for (int k1 = 0; k1 < 8; k1++)
		for (int d1 = 1; d1 < 8; d1++)
		{
			// move to cell x2 y2
			int x2 = x1 + dx[k1] * d1, y2 = y1 + dy[k1] * d1;
			if (!board.InGrid(x2, y2)) break;
			if (board.GetCell(x2, y2) != 0) break;
			for (int k2 = 0; k2 < 8; k2++)
			for (int d2 = 1; d2 < 8; d2++)
			{
				// put obstacle at x3 y3
				int x3 = x2 + dx[k2] * d2, y3 = y2 + dy[k2] * d2;
				if (!board.InGrid(x3, y3)) break;
				if (board.GetCell(x3, y3) != 0 && !(x3 == x1 && y3 == y1)) break;
				// apply move
				board.ApplyMove(x1, y1, x2, y2, x3, y3, false);
				int area;
				if (BotColor == color) // Bot's turn, calculate Player's best move
					area = -GreedySearch(-color);
				else // Player's turn, maximize area difference
					area = CalAreaDiff(color);
				if (area > best)
				{
					best = area;
					if (color == BotColor) tie(rx1, ry1, rx2, ry2, rx3, ry3) = tie(x1, y1, x2, y2, x3, y3);
				}
				// revert move
				board.ApplyMove(x1, y1, x2, y2, x3, y3, true);
			}
		}
	}
	if (best == -INF)
	{
		if (color == BotColor) startX = startY = resultX = resultY = obstacleX = obstacleY = -1;
		return 0;
	}
	else
	{
		if (color == BotColor)
		{
			tie(startX, startY, resultX, resultY, obstacleX, obstacleY) = tie(rx1, ry1, rx2, ry2, rx3, ry3);
		}
		return best;
	}
}


// Use game theory to calculate all possible moves
bool Game::DeepSearch(int color)
{
	auto it = data.find(make_pair(board.GetGrid(), color));
	// if this state's already calculated, return the value
	if (it != data.end()) return it->second;
	bool flag = false;
	int t = color; if (t == -1) t = 0;
	for (int i = 0; i < board.pos[t].size(); i++)
	{
		// iterate all chess's position x1 y1
		int x1 = board.pos[t][i].first, y1 = board.pos[t][i].second;
		for (int k1 = 0; k1 < 8; k1++)
		for (int d1 = 1; d1 < 8; d1++)
		{
			// move to cell x2 y2
			int x2 = x1 + dx[k1] * d1, y2 = y1 + dy[k1] * d1;
			if (!board.InGrid(x2, y2)) break;
			if (board.GetCell(x2, y2) != 0) break;
			for (int k2 = 0; k2 < 8; k2++)
			for (int d2 = 1; d2 < 8; d2++)
			{
				// put obstacle at x3 y3
				int x3 = x2 + dx[k2] * d2, y3 = y2 + dy[k2] * d2;
				if (!board.InGrid(x3, y3)) break;
				if (board.GetCell(x3, y3) != 0 && !(x3 == x1 && y3 == y1)) break;
				// apply move
				board.ApplyMove(x1, y1, x2, y2, x3, y3, false);
				if (!DeepSearch(-color)) flag = true;
				// revert move
				board.ApplyMove(x1, y1, x2, y2, x3, y3, true);
				// if this move can let (color) player win
				if (flag)
				{
					auto state = make_pair(board.GetGrid(), color);
					step[state].emplace_back(x1, y1);
					step[state].emplace_back(x2, y2);
					step[state].emplace_back(x3, y3);
					break;
				}
			}
			if (flag) break;
		}
		if (flag) break;
	}

	auto state = make_pair(board.GetGrid(), color);
	data[state] = flag;
	return flag;
}


void Game::BotMove()
{
	if (board.LoseState(BotColor)) // Bot in lose state
	{
		cout << "You win!\n";
		isRunning = false; 
		return;
	}

	if (board.turnID <= 23) // Use Minimax algorithm in the first 22 moves
	{
		GreedySearch(BotColor);
	}
	else // Search all possible moves
	{
		data.clear();
		step.clear();
		bool canWin = DeepSearch(BotColor);
		if (canWin)
		{
			auto state = make_pair(board.GetGrid(), BotColor);
			tie(startX, startY) = step[state][0];
			tie(resultX, resultY) = step[state][1];
			tie(obstacleX, obstacleY) = step[state][2];
		}
		else
		{
			startX = startY = resultX = resultY = obstacleX = obstacleY = -1;
		}
	}

	if (startX == -1) // There is no move left for bot
	{
		cout << "You win!\n";
		isRunning = false;
		return;
	}
	else
	{
		board.ApplyMove(startX, startY, resultX, resultY, obstacleX, obstacleY, false);
		board.DrawBoard();
		cout << "Turn: " << board.turnID << "\n";
		cout << "Computer move: " << startX << ' ' << startY << ' ' << resultX << ' '
			<< resultY << ' ' << obstacleX << ' ' << obstacleY << "\n";
	}

	if (board.LoseState(-BotColor)) // Player in lose state
	{
		cout << "Computer win!\n";
		isRunning = false;
		return;
	}
}

void Game::ShowMenu()
{
	cout << "1. Select move\n";
	cout << "2. New game\n";
	cout << "3. Quit game\n";
	cout << "Please select [1/2/3]: ";
	int ops;
	string line;
	while (true)
	{
		getline(cin, line);
		if (line != "1" && line != "2" && line != "3")
		{
			cout << "Invalid input! Please re-select [1/2/3]: ";
		}
		else
		{
			stringstream st(line);
			st >> ops;
			break;
		}
	}
	switch (ops)
	{
	case 1:	// Select move
		HumanMove();
		BotMove();
		break;
	case 2: // New game
		board = new Board(true);
		board.DrawBoard();
		break;
	case 3: // Quit game
		cout << "Do you want to save the game? [y/n]: ";
		char ans;
		cin >> ans;
		if (ans == 'y') board.SaveBoard();
		isRunning = false;
		break;
	default:
		break;
	}
}


