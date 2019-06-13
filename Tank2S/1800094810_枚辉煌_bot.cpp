#include <bits/stdc++.h>
#include "jsoncpp/json.h"

using namespace std;

template<typename T> inline T operator~ (T a) { return (T)~(int)a; }
template<typename T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<typename T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<typename T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<typename T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<typename T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<typename T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }

#define For(i,a,b) for(int i = a;i <= b; i++)
#define Rep(i,a,b) for(int i = a;i >= b; i--)
#define REP(i, n) for(int i = 0; i < n; i++)
#define FOR(i, f) for(auto i : f)
#define fi first
#define se second
#define pb push_back
#define eb emplace_back
#define mp make_pair
#define BUG(x) (cerr << #x << " = " << x << "\n")
#define sz(s) int(s.size())
#define reset(f, x) memset(f, x, sizeof(f))
#define all(x) x.begin(), x.end()
#define two(x) (1LL << x)
#define bit(x, i) ((x >> (i)) & 1LL)
#define onbit(x, i) (x | (1LL << (i)))
#define offbit(x, i) (x & ~(1 << (i)))

int tx, ty;


enum GameResult
{
    NotFinished = -2, Draw = -1, Blue = 0, Red = 1
};

enum FieldItem
{
    None = 0, Brick = 1, Steel = 2, Base = 4, Blue0 = 8, Blue1 = 16, Red0 = 32, Red1 = 64, Water = 128, Forest = 256
};

enum Action
{
    Invalid = -3, Unknown = -2, Stay = -1, Up, Right, Down, Left, UpShoot, RightShoot, DownShoot, LeftShoot
};

bool firstTurn = true;

const int fieldHeight = 9, fieldWidth = 9, sideCount = 2, tankPerSide = 2;

// 基地的横坐标
const int baseX[sideCount] = { fieldWidth / 2, fieldWidth / 2 };

// 基地的纵坐标
const int baseY[sideCount] = { 0, fieldHeight - 1 };

const int N = 9;
const int INF = 1000;

const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
const FieldItem tankItemTypes[sideCount][tankPerSide] = {
    { Blue0, Blue1 },{ Red0, Red1 }
};

int maxTurn = 100;
Action finalAct[2] = {Stay, Stay};
bool danger[fieldHeight][fieldWidth];
bool tankCanShoot[2][2];

inline bool ActionIsMove(Action x)
{
    return x >= Up && x <= Left;
}

inline bool ActionIsShoot(Action x)
{
    return x >= UpShoot && x <= LeftShoot;
}

inline bool ActionDirectionIsOpposite(Action a, Action b)
{
    return a >= Up && b >= Up && (a + 2) % 4 == b % 4;
}

inline bool CoordValid(int x, int y)
{
    return x >= 0 && x < fieldWidth && y >= 0 && y < fieldHeight;
}

inline int count(FieldItem item)
{
    int cnt = 0;
    while(item)cnt += 1, item =(FieldItem)(((int)item) & (((int)item) - 1));
    return cnt;
}
// 判断 item 是不是叠在一起的多个坦克
inline bool HasMultipleTank(FieldItem item) // changed in tank2s
{
    int cnt = count(item);
    cnt -= !!(item & Forest);//除去forest
    return cnt == 2;
//      return !!(item & (item - 1));
}

inline int GetTankSide(FieldItem item)
{
    return item == Blue0 || item == Blue1 ? Blue : Red;
}

inline int GetTankID(FieldItem item)
{
    return item == Blue0 || item == Red0 ? 0 : 1;
}

// 获得动作的方向
inline int ExtractDirectionFromAction(Action x)
{
    if (x >= Up)
        return x % 4;
    return -1;
}

// 游戏场地上的物件（一个格子上可能有多个坦克）
FieldItem gameField[fieldHeight][fieldWidth] = {};

// 坦克是否存活
bool tankAlive[sideCount][tankPerSide] = { { true, true },{ true, true } };

// 基地是否存活
bool baseAlive[sideCount] = { true, true };

// 坦克横坐标，-1表示坦克已炸
int tankX[sideCount][tankPerSide] = {
    { fieldWidth / 2 - 2, fieldWidth / 2 + 2 },{ fieldWidth / 2 + 2, fieldWidth / 2 - 2 }
};

int lastTurnX[2], lastTurnY[2];

// 坦克纵坐标，-1表示坦克已炸
int tankY[sideCount][tankPerSide] = { { 0, 0 },{ fieldHeight - 1, fieldHeight - 1 } };

// 当前回合编号
int currentTurn = 1;

// 我是哪一方
int mySide;

// 过往动作（previousActions[x] 表示所有人在第 x 回合的动作，第 0 回合的动作没有意义）
Action previousActions[101][sideCount][tankPerSide] = { { { Stay, Stay },{ Stay, Stay } } };

/* 三个 int 表示场地 01 矩阵（每个 int 用 27 位表示 3 行）
   initialize gameField[][]
   brick>water>steel
*/

int enemyLastX[101][2], enemyLastY[101][2];
int ourLastX[101][2], ourLastY[101][2];
vector<pair<int, int>> destroyed_blocks;

void InitField(int hasBrick[3],int hasWater[3],int hasSteel[3],int hasForest[3], int _mySide)
{
    mySide = _mySide;
    for (int i = 0; i < 3; i++)
    {
        int mask = 1;
        for (int y = i * 3; y < (i + 1) * 3; y++)
        {
            for (int x = 0; x < fieldWidth; x++)
            {
                if (hasBrick[i] & mask)
                    gameField[y][x] = Brick;
                else if(hasWater[i] & mask)
                    gameField[y][x] = Water;
                else if(hasSteel[i] & mask)
                    gameField[y][x] = Steel;
                else if(hasForest[i] & mask)
                    gameField[y][x] = Forest;
                mask <<= 1;
            }
        }
    }
    for (int side = 0; side < sideCount; side++)
    {
       // for (int tank = 0; tank < tankPerSide; tank++)
       //     gameField[tankY[side][tank]][tankX[side][tank]] = tankItemTypes[side][tank];
        gameField[baseY[side]][baseX[side]] = Base;
    }
    REP(tank, 2)
    {
        enemyLastX[0][tank] = tankX[1-mySide][tank];
        enemyLastY[0][tank] = tankY[1-mySide][tank];
        ourLastX[0][tank] = tankX[mySide][tank];
        ourLastY[0][tank] = tankY[mySide][tank];
    }
}
/* Internals */
Json::Reader reader;
#ifdef _BOTZONE_ONLINE
Json::FastWriter writer;
#else
Json::StyledWriter writer;
#endif

void _processRequestOrResponse(Json::Value& value, bool isOpponent, bool first)
{
    if (!first)
    {
        if (!isOpponent)
        {
            for (int tank = 0; tank < sideCount; tank++)
            if (tankAlive[mySide][tank])
            {
                Action act = (Action)value[tank].asInt();
                previousActions[currentTurn][mySide][tank] = act;
                if (ActionIsMove(act))
                {
                    tankX[mySide][tank] += dx[act];
                    tankY[mySide][tank] += dy[act];
                    ourLastX[currentTurn][tank] = tankX[mySide][tank];
                    ourLastY[currentTurn][tank] = tankY[mySide][tank];
                }
            }
        }
        else
        {
            int n = value["destroyed_blocks"].size();
            destroyed_blocks.clear();
            for (int i = 0; i < n; i += 2)
            {
                int x = value["destroyed_blocks"][i].asInt();
                int y = value["destroyed_blocks"][i+1].asInt();
                destroyed_blocks.push_back(make_pair(x, y));
            }
            n = value["destroyed_tanks"].size();
            for (int i = 0; i < n; i += 2)
            {
                int x = value["destroyed_tanks"][i].asInt();
                int y = value["destroyed_tanks"][i+1].asInt();
                REP(tank, 2)
                if (tankX[mySide][tank] == x && tankY[mySide][tank] == y)
                    tankAlive[mySide][tank] = false;
            }
            for (int tank = 0; tank < tankPerSide; tank++)
            if (tankAlive[1-mySide][tank])
            {
                lastTurnX[tank] = tankX[1-mySide][tank];
                lastTurnY[tank] = tankY[1-mySide][tank];
                previousActions[currentTurn][1 - mySide][tank] = (Action)value["action"][tank].asInt();
                tankX[1-mySide][tank] = enemyLastX[currentTurn][tank] = value["final_enemy_positions"][tank*2].asInt();
                tankY[1-mySide][tank] = enemyLastY[currentTurn][tank] = value["final_enemy_positions"][tank*2+1].asInt();
                if (tankX[1-mySide][tank] == -1) tankAlive[1-mySide][tank] = false;
            }
            currentTurn++;

        }
    }
    else
    {
        // 是第一回合，裁判在介绍场地
        int hasBrick[3],hasWater[3],hasSteel[3],hasForest[3];
        for (int i = 0; i < 3; i++){//Tank2 feature(???????????????)
            hasWater[i] = value["waterfield"][i].asInt();
            hasBrick[i] = value["brickfield"][i].asInt();
            hasSteel[i] = value["steelfield"][i].asInt();
            hasForest[i] = value["forestfield"][i].asInt();
        }
        InitField(hasBrick,hasWater,hasSteel,hasForest,value["mySide"].asInt());
        firstTurn = false;
    }
}

void ReadInput(istream& in, string& outData, string& outGlobalData)
{
    Json::Value input;
    string inputString;
    do
    {
        getline(in, inputString);
    } while (inputString.empty());
#ifndef _BOTZONE_ONLINE
    // 猜测是单行还是多行
    char lastChar = inputString[inputString.size() - 1];
    if (lastChar != '}' && lastChar != ']')
    {
        // 第一行不以}或]结尾，猜测是多行
        string newString;
        do
        {
            getline(in, newString);
            inputString += newString;
        } while (newString != "}" && newString != "]");
    }
#endif
    reader.parse(inputString, input);

    if (firstTurn)
    {
        Json::Value requests = input["requests"];
        int n = requests.size();
        for (int i = 0; i < n; i++)
            _processRequestOrResponse(requests[i], true, firstTurn);
    } else
    {
        _processRequestOrResponse(input, true, false);
    }
}

// 请使用 SubmitAndExit 或者 SubmitAndDontExit
void _submitAction(Action tank0, Action tank1, string debug = "", string data = "", string globalData = "")
{
    Json::Value output(Json::objectValue), response(Json::arrayValue);
    response[0U] = tank0;
    response[1U] = tank1;
    Json::Value a = Json::Value(Json::arrayValue);
    a.append(tankX[mySide][0]);a.append(tankY[mySide][0]);
    a.append(tankX[mySide][1]);a.append(tankY[mySide][1]);
    output["debug"] = a;
    output["response"] = response;
    if (!debug.empty())
        output["debug"] = debug;
    if (!data.empty())
        output["data"] = data;
    if (!globalData.empty())
        output["globalData"] = globalData;
    cout << writer.write(output) << endl;
}

// 提交决策并退出，下回合时会重新运行程序
void SubmitAndExit(Action tank0, Action tank1, string debug = "", string data = "", string globalData = "")
{
    _submitAction(tank0, tank1, debug, data, globalData);
    exit(0);
}

// 提交决策，下回合时程序继续运行（需要在 Botzone 上提交 Bot 时选择“允许长时运行”）
// 如果游戏结束，程序会被系统杀死
void SubmitAndDontExit(Action tank0, Action tank1)
{
    _submitAction(tank0, tank1);
    previousActions[currentTurn][mySide][0] = tank0;
    previousActions[currentTurn][mySide][1] = tank1;
    REP(tank, 2)
    {
        auto act = finalAct[tank];
        if (ActionIsMove(act))
        {
            int dir = ExtractDirectionFromAction(act);
            tankX[mySide][tank] += dx[dir];
            tankY[mySide][tank] += dy[dir];
        }
    }
    cout << ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<" << endl;
    cout << flush;
}

/* Our code start here */

const Action moveOrder[2][2][4] = {
    {{ Down, Left, Right, Up },
    { Down, Right, Left, Up }},
    {{ Up, Right, Left, Down },
    { Up, Left, Right, Down }}
};



inline int Mahattan(int x1, int y1, int x2, int y2)
{
    return abs(x1-x2) + abs(y1-y2);
}

// enemy tank is visible
inline bool tankVisible(int side, int tank)
{
    if (!tankAlive[side][tank]) return false;
    if (side == 1-mySide)
        return enemyLastX[currentTurn-1][tank] >= 0;
    else return gameField[tankY[side][tank]][tankX[side][tank]] != Forest;
}

// check if nothing between (x1, y1) and (x2, y2)
bool NothingBetween(int x1, int y1, int x2, int y2)
{
    if (x1 == x2)
    {
        if (y1 > y2) swap(y1, y2);
        For(yy, y1+1, y2-1)
        if (gameField[yy][x1] != None && gameField[yy][x1] != Water && gameField[yy][x1] != Forest) return false;
    } else // y1 == y2
    {
        if (x1 > x2) swap(x1, x2);
        For(xx, x1+1, x2-1)
        if (gameField[y1][xx] != None && gameField[y1][xx] != Water && gameField[y1][xx] != Forest) return false;
    }
    return true;
}

// check (x3, y3) is between (x1, y1) and (x2, y2)
bool InBetween(int x1, int y1, int x2, int y2, int x3, int y3)
{
    if (x1 == x2)
    {
        if (x3 != x1) return false;
        return (y3 > min(y1, y2) && y3 < max(y1, y2));
    } else if (y1 == y2)
    {
        if (y3 != y1) return false;
        return (x3 > min(x1, x2) && x3 < max(x1, x2));
    }
    return false;
}

// check if tank at (fromX, fromY) can destroy object at (desX, desY)
Action CanDestroy(int fromX, int fromY, int desX, int desY, bool first)
{
    if (fromX == desX && fromY == desY)
        return Invalid;
    if (first)
    REP(side, 2) REP(tank, 2)
    if (tankX[side][tank] >= 0 && InBetween(fromX, fromY, tankX[side][tank], tankY[side][tank], desX, desY)) return Invalid;

    if (fromX == desX && NothingBetween(fromX, fromY, desX, desY))
    {
        if (fromY < desY) return DownShoot;
        else return UpShoot;
    }
    if (fromY == desY && NothingBetween(fromX, fromY, desX, desY))
    {
        if (fromX < desX) return RightShoot;
        else return LeftShoot;
    }
    return Invalid;
}

// mark all the cell a tank (side, tank) can shoot with direction dir as dangerous zone
void InitDangerZone(int side, int tank, int dir)
{
    int x = tankX[side][tank], y = tankY[side][tank];
    while (true)
    {
        x += dx[dir]; y += dy[dir];
        if (!CoordValid(x, y)) break;
        FieldItem item = gameField[y][x];
        if (item != None && item != Water && item != Forest) break;
        danger[y][x] = true;
    }
}

// check if tank (side, tank) can avoid shoot
bool CanAvoid(int side, int tank, Action shoot)
{
    if (tankCanShoot[side][tank]) return true; // can shoot to defend
    int x = tankX[side][tank], y = tankY[side][tank];
    if (shoot == UpShoot || shoot == DownShoot) // has to move left or right
    {
        for (int xx = x-1; xx <= x+1; xx += 2)
            if (CoordValid(xx, y) && (gameField[y][xx] == None || gameField[y][xx] == Forest)) return true;
    } else // move up or down
    {
        for (int yy = y-1; yy <= y+1; yy += 2)
            if (CoordValid(x, yy) && (gameField[yy][x] == None || gameField[yy][x] == Forest)) return true;
    }
    return false;
}

// return previous t turn action of a tank
inline Action prevAct(int t, int side, int tank)
{
    return previousActions[currentTurn-t][side][tank];
}

// return true if (x1, y1) < (x2, y2)
bool smaller(int x1, int y1, int x2, int y2)
{
    return tie(x1, y1) < tie(x2, y2);
}

inline bool InAnotherSide(int side, int y)
{
    if (!side && y > 4) return true;
    if (side && y < 4) return true;
    return false;
}

const double fw = 0.7;
const double bw = 0.3;

inline double calWeight(int forest, int brick)
{
    return fw*forest + bw*brick;
}

struct state
{
    // f : total cost, g : number of forest
    int f, g, x, y, timer;
    state() { }
    state(int f, int g, int y, int x, int timer) : f(f), g(g), x(x), y(y), timer(timer) { }
    bool operator < (const state &other) const
    {
        if (f != other.f) return f < other.f;
        if (g != other.g) return g < other.g;
        return timer < other.timer;
    }
};

int dis[fieldHeight][fieldWidth];
int br[fieldHeight][fieldWidth];
set<state> pq;
Action firstAct[fieldHeight][fieldWidth];

// using uniform cost search (Dijkstra algo) find path for a tank to destroy object in (desX, desY) loc
// if two paths have same length then choose the path which has more forest to hide tank
// return path length and the first action

pair<int, Action> FindPath(int side, int tank, int desX, int desY, bool watchDanger)
{
    pq.clear();
    REP(i, N) REP(j, N)
    {
        dis[i][j] = INF; br[i][j] = INF;
        firstAct[i][j] = Invalid;
    }
    if (currentTurn > 4)
    REP(side1, 2) REP(tank1, 2)
    {
        int x = tankX[side1][tank1], y = tankY[side1][tank1];
        if (tank != tank1 || side != side1)
        {
            if (gameField[y][x] == None)
            {
                if (side == side1) gameField[y][x] = Steel;
                else gameField[y][x] = Brick;
            }
        }
    }
    int x, y;
    if (side == mySide)
    {
        x = tankX[side][tank], y = tankY[side][tank];
    } else
    {
        x = lastTurnX[tank], y = lastTurnY[tank];
    }
    dis[y][x] = 0;
    pq.insert(state(0, 0, y, x, 0));
    pq.insert(state(1, 0, y, x, 0));
    firstAct[y][x] = Stay;
    int timer = 0;
    int minPath = INF, minBrick = INF;
    Action shouldAct = Stay;
    while (!pq.empty())
    {
        int f = pq.begin()->f;
        int g = pq.begin()->g;
        x = pq.begin()->x; y = pq.begin()->y;
        pq.erase(pq.begin());
        if (f && CanDestroy(x, y, desX, desY, false) != Invalid) // not first move
        {
            if ((f+1 < minPath) || (f+1 == minPath && g < minBrick))
            {
                minPath = f+1; minBrick = g;
                shouldAct = firstAct[y][x];
            }
            continue;
        }
        // if tank in same row or column with enemy base then stay here and shoot
        if (x == desX || y == desY)
        if (!(!f && watchDanger && danger[y][x]))
        {
            int dir; // which direction should I shoot
            int cntBrick = 0; // count how many brick between tank and base
            if (x == desX)
            {
                if (y < desY) dir = (int)Down;
                else dir = (int)Up;
                int y1 = min(y, desY), y2 = max(y, desY);
                For(yy, y1+1, y2-1)
                if (gameField[yy][x] == Brick) cntBrick++;
                else if (gameField[yy][x] == Steel)
                {
                    cntBrick = -1; break;
                }
            }
            if (y == desY)
            {
                if (x < desX) dir = (int)Right;
                else dir = (int)Left;
                int x1 = min(x, desX), x2 = max(x, desX);
                For(xx, x1+1, x2-1)
                if (gameField[y][xx] == Brick) cntBrick++;
                else if (gameField[y][xx] == Steel)
                {
                    cntBrick = -1; break;
                }
            }
            if (cntBrick != -1) // if there is no steel between tank and base
            {
	            int timeNeed = cntBrick*2+1;
	            bool needToStay = false;
	            // if this is first move and last turn is shoot then stay for one turn and begin shooting
	            if (!f && !tankCanShoot[side][tank])
	            {
	                timeNeed++;
	                needToStay = true;
	            }
	            if ((f+timeNeed < minPath) || (f+timeNeed == minPath && g+cntBrick <= minBrick))
	            {
	                Action act;
	                minPath = f+timeNeed; minBrick = g+cntBrick;
	                if (needToStay) act = Stay;
	                else if (!f) act = Action(dir+4); // shoot
	                else act = firstAct[y][x];
	                if (smaller(f+timeNeed, g+cntBrick, minPath, minBrick))
                        shouldAct = act;
                    else if (act != Stay) shouldAct = act;
	            }
			}
        }
        // only consider move action, if moving in brick then shoot to destroy
        REP(i, 4) // 4 directions
        {
            int act = moveOrder[side][tank][i];
            int cost = 1, forest = 0, brick = 0; Action nextAct = (Action)act;
            int u = x + dx[act], v = y + dy[act];
            if (CoordValid(u, v) && (gameField[v][u] == None || gameField[v][u] == Brick || gameField[v][u] == Forest))
            {
                if (!f && watchDanger && danger[v][u]) continue;
                // move into Brick, shoot to destroy
                if (gameField[v][u] == Brick)
                {
                    if (!f && ActionIsShoot(previousActions[currentTurn-1][side][tank])) continue;
                    nextAct = Action(act+4);
                    cost = 2;
                    brick = 1;
                }
                if (gameField[v][u] == Forest) forest = 1;
                // minimize total cost, if total cost equal then maximize number of forest
                if (dis[v][u] > f+cost || (dis[v][u] == f+cost && g+brick < br[v][u]))
                {
                    timer++;
                    dis[v][u] = f+cost;
                    br[v][u] = g+brick;
                    pq.insert(state(f+cost, g+brick, v, u, timer));
                    if (!f) firstAct[v][u] = nextAct;
                    else firstAct[v][u] = firstAct[y][x];
                }
            }
        }
    }
    if (currentTurn > 4)
    REP(side1, 2) REP(tank1, 2)
    {
        int x = tankX[side1][tank1], y = tankY[side1][tank1];
        if (tank != tank1 || side != side1)
        {
            if (gameField[y][x] == Brick || gameField[y][x] == Steel) gameField[y][x] = None;
        }
    }
    return make_pair(minPath, shouldAct);
}

// Guess enemy tank location and whether they can shoot or not
// 更新：长时运行
// 目的：保留上回合猜测对方的位置
// lastTurnX, lastTurnY是坦克上回合的位置（猜测）
void InitTank()
{
    REP(side, 2) REP(tank, 2)
    {
        tankCanShoot[side][tank] = (tankAlive[side][tank] && !ActionIsShoot(prevAct(1, side, tank)));
    }
    // 最靠谱的猜测
    // if enemy tank from visible state move into forest then guess enemy tank location from last location
    REP(tank, 2)
    if (tankAlive[1-mySide][tank] && tankX[1-mySide][tank] == -2 && enemyLastX[currentTurn-2][tank] >= 0)
    {
        int x = enemyLastX[currentTurn-2][tank], y = enemyLastY[currentTurn-2][tank];
        int cntForest = 0;
        int sX, sY;
        for (int dir = 0; dir < 4; dir++)
        {
            int xx = x+dx[dir], yy = y+dy[dir];
            if (CoordValid(xx, yy) && gameField[yy][xx] == Forest)
            {
                cntForest++;
                if (cntForest == 1) { sX = xx; sY = yy; }
                if (cntForest == 2 && Mahattan(xx, yy, baseX[mySide], baseY[mySide]) < Mahattan(sX, sY, baseX[mySide], baseY[mySide]))
                    { sX = xx; sY = yy; }
            }
        }
        // if cntForest = 1 then tank in this location
        // if cntForest = 2 then choose the block which is closer to our base
        if (cntForest <= 2)
        {
            tankX[1-mySide][tank] = sX;
            tankY[1-mySide][tank] = sY;
        }
    }

    // guess location from shoot
    // check all destroyed blocks in last turn, if a block is destroyed by enemy tank in forest
    // then assume that enemy tank is around this block
    for (int i = 0; i < sz(destroyed_blocks); i++)
    {
        int x = destroyed_blocks[i].first, y = destroyed_blocks[i].second;
        bool flag = false;
        // check all visible tank action in last turn
        REP(side, 2)
        REP(tank, 2)
        if (tankX[side][tank] >= 0)
        {
            Action act = prevAct(1, side, tank);
            if (ActionIsShoot(act))
            {
                int dir = ExtractDirectionFromAction(act);
                int xx = tankX[side][tank], yy = tankY[side][tank];
                while (true)
                {
                    xx += dx[dir]; yy += dy[dir];
                    if (!CoordValid(xx, yy)) break;
                    // if a visible tank destroy this block
                    if (xx == x && yy == y)
                    {
                        flag = true; break;
                    }
                    if (gameField[yy][xx] == Steel || gameField[yy][xx] == Brick) break;
                }
            }
        }
        // !flag means this block was destroyed by enemy tank in forest
        if (!flag)
        {
            REP(tank, 2)
            if (tankAlive[1-mySide][tank] && !tankVisible(1-mySide, tank))
            {
                // 根据地图的对称性判断哪个坦克 shoot
                if (1-mySide == 0 && !tank && x <= 4) tankCanShoot[1-mySide][tank] = false;
                if (1-mySide == 0 && tank && x >= 4) tankCanShoot[1-mySide][tank] = false;
                if (1-mySide == 1 && tank && x <= 4) tankCanShoot[1-mySide][tank] = false;
                if (1-mySide == 1 && !tank && x >= 4) tankCanShoot[1-mySide][tank] = false;
                int cntForrest = 0;
                int sX, sY;
                // 上回猜测的位置是对的
                if (lastTurnX[tank] == x || lastTurnY[tank] == y)
                {
                    tankX[1-mySide][tank] = lastTurnX[tank];
                    tankY[1-mySide][tank] = lastTurnY[tank];
                    break;
                }
                // 如果这block的周围只有一个forest那么假设对方的坦克在这里
                for (int dir = 0; dir < 4; dir++)
                {
                    int xx = x+dx[dir], yy = y+dy[dir];
                    if (CoordValid(xx, yy) && gameField[yy][xx] == Forest)
                    {
                        cntForrest++;
                        sX = xx; sY = yy;
                    }
                }
                if (cntForrest == 1)
                {
                    tankX[1-mySide][tank] = sX;
                    tankY[1-mySide][tank] = sY;
                }
            }
        }
    }

    // 对射判断
    REP(tank, 2)
    {
        auto act = prevAct(1, mySide, tank);
        if (ActionIsShoot(act))
        {
            int dir = ExtractDirectionFromAction(act);
            int x = tankX[mySide][tank], y = tankY[mySide][tank];
            while (true)
            {
                x += dx[dir]; y += dy[dir];
                if (!CoordValid(x, y)) break;
                if (gameField[y][x] == Brick || gameField[y][x] == Steel) break;
            }
            if (!CoordValid(x, y)) continue;
            bool flag = false;
            if (gameField[y][x] == Brick)
            {
                REP(i, sz(destroyed_blocks))
                if (x == destroyed_blocks[i].first && y == destroyed_blocks[i].second)
                {
                    flag = true;
                    break;
                }
                if (!flag)
                    tankCanShoot[1-mySide][1-tank] = false;
            }
        }
    }

    // 假设对方的坦克以最短路径移动
    reset(danger, false);
    REP(tank, 2)
    if (tankX[1-mySide][tank] == -2)
    {
        auto res = FindPath(1-mySide, tank, baseX[mySide], baseY[mySide], true);
        auto act = res.second;
        if (ActionIsMove(act))
        {
            int dir = ExtractDirectionFromAction(act);
            int x = lastTurnX[tank]+dx[dir], y = lastTurnY[tank]+dy[dir];
            // 如果移动到Forest，更新位置
            if (gameField[y][x] == Forest)
            {
                tankX[1-mySide][tank] = x; tankY[1-mySide][tank] = y;
            } else // 如果移动到None, 就假设它留在原位
            {
                tankX[1-mySide][tank] = lastTurnX[tank]; tankY[1-mySide][tank] = lastTurnY[tank];
            }
        } else // 坦克要射击所以位置不变
        {
            tankX[1-mySide][tank] = lastTurnX[tank]; tankY[1-mySide][tank] = lastTurnY[tank];
        }
    }

    for (int i = 0; i < sz(destroyed_blocks); i++)
    {
        int x = destroyed_blocks[i].first, y = destroyed_blocks[i].second;
        gameField[y][x] = None;
    }
    cerr << "Current turn: " << currentTurn << "\n";
    cerr << "Enemy position:\n";
    cerr << "0: " << tankY[1-mySide][0] << ' ' << tankX[1-mySide][0] << ' ' << "can shoot: " << tankCanShoot[1-mySide][0] << "\n";
    cerr << "1: " << tankY[1-mySide][1] << ' ' << tankX[1-mySide][1] << ' ' << "can shoot: " << tankCanShoot[1-mySide][1] << "\n";
    cerr << "Our position:\n";
    cerr << "0: " << tankY[mySide][0] << ' ' << tankX[mySide][0] << ' ' << "can shoot: " << tankCanShoot[mySide][0] << "\n";
    cerr << "1: " << tankY[mySide][1] << ' ' << tankX[mySide][1] << ' ' << "can shoot: " << tankCanShoot[mySide][1] << "\n";
}

void Greedy()
{

    InitTank();
    finalAct[0] = finalAct[1] = Stay;
    reset(danger, false);
    for (int tank = 0; tank < 2; tank++)
    if (tankCanShoot[1-mySide][tank] && tankX[1-mySide][tank] >= 0)
    {
        // 如果坦克重叠先停一步
        bool hasMultipleTankWithMe = false;
        bool needStay = false;
        REP(tank2, 2)
        if (tankX[mySide][tank2] == tankX[1-mySide][tank] && tankY[mySide][tank2] == tankY[1-mySide][tank])
        {
            hasMultipleTankWithMe = true;
            if (prevAct(1, mySide, tank2) != Stay) needStay = true;
        }
        if (hasMultipleTankWithMe && !needStay) continue;
        REP(tank2, 2)
        {
            auto act = CanDestroy(tankX[1-mySide][tank], tankY[1-mySide][tank], tankX[mySide][tank2], tankY[mySide][tank2], true);
            if (act != Invalid)
            {
                int dir = ExtractDirectionFromAction(act);
                InitDangerZone(1-mySide, tank, dir);
                break;
            }
        }
    }
    if (currentTurn == 8)
	{
		int tmp = 1;
	}
    for (int tank = 0; tank < 2; tank++)
    if (tankAlive[mySide][tank])
    {
        if (tankCanShoot[mySide][tank])
        {
            // can shoot enemy base
            auto act = CanDestroy(tankX[mySide][tank], tankY[mySide][tank], baseX[1-mySide], baseY[1-mySide], true);
            if (act != Invalid)
            {
                finalAct[tank] = act;
                continue;
            }
            // can kill enemy's tank, in case enemy's tank can't avoid the shoot or our tank is in forest
            REP(tank2, 2)
            if (tankX[1-mySide][tank2] >= 0)
            {
                act = CanDestroy(tankX[mySide][tank], tankY[mySide][tank], tankX[1-mySide][tank2], tankY[1-mySide][tank2], true);
                if (act != Invalid && (!CanAvoid(1-mySide, tank2, act) || !tankVisible(mySide, tank)))
                {
                    finalAct[tank] = act;
                    break;
                }
            }
        }
        if (finalAct[tank] != Stay) continue;
        // find shortest path to destroy base
        auto path = FindPath(mySide, tank, baseX[1-mySide], baseY[1-mySide], true);
        int pathLength = path.first;
        auto act = path.second;
		// if between this tank and enemy's tank only have one brick then don't destroy
        // because enemy tank can shoot us in next turn
        if (ActionIsShoot(act))
        {
            int dir = ExtractDirectionFromAction(act);
            int xx = tankX[mySide][tank], yy = tankY[mySide][tank];
            bool flag = true; // enemy tank can kill us in next turn
            int xB = xx+dx[dir], yB = yy+dy[dir];
            if (gameField[yB][xB] == Brick)
            {
                gameField[yB][xB] = None;
                int sTank = -1;
                REP(tank2, 2)
                if (tankX[1-mySide][tank2] >= 0)
                {
                    int x1 = tankX[1-mySide][tank2], y1 = tankY[1-mySide][tank2];
                    if (CanDestroy(x1, y1, xx, yy, true) != Invalid && InBetween(xx, yy, x1, y1, xB, yB))
                    {
                        flag = false; break;
                    }
                    if (prevAct(1, 1-mySide, tank2) != Stay)
                    REP(i, 3)
                    {
                        int dir2 = (int)moveOrder[1-mySide][tank2][i];
                        int x2 = x1+dx[dir2], y2 = y1+dy[dir2];
                        if (CoordValid(x2, y2) && (gameField[y2][x2] == None || gameField[y2][x2] == Forest)
                            && CanDestroy(x2, y2, xx, yy, false) != Invalid && InBetween(xx, yy, x2, y2, xB, yB))
                        {
                            flag = false; break;
                        }
                    }
                }
                gameField[yy+dy[dir]][xx+dx[dir]] = Brick;
                Action actBefore = act;
                if (!flag) act = Stay;
                // if our tank keep staying for 3 turns then try to destroy brick around tank to find another way
                if (act == Stay && prevAct(1, mySide, tank) == Stay && prevAct(2, mySide, tank) == Stay && InAnotherSide(mySide, yy))
                {
                    REP(i, 3)
                    {
                        int dir = moveOrder[mySide][tank][i];
                        Action act2 = Action(dir+4);
                        if (act2 == actBefore) continue;
                        int x1 = xx+dx[dir], y1 = yy+dy[dir];;
                        if (!CoordValid(x1, y1)) continue;
                        if (gameField[y1][x1] == Brick)
                        {
                            act = act2;
                            break;
                        }
                    }
                }
            }
        }
        // if tank action is move then don't need to check because tank won't step in danger zone
        // if tank stay in current location, check if enemy tank can kill this tank and shoot back defend
        if (tankCanShoot[mySide][tank] && tankVisible(mySide, tank) && (act == Stay || ActionIsShoot(act)))
        {
            REP(tank2, 2) // enemy tank
            if (tankX[1-mySide][tank2] >= 0 && tankCanShoot[1-mySide][tank2])
            {
                Action enemyShoot = CanDestroy(tankX[1-mySide][tank2], tankY[1-mySide][tank2], tankX[mySide][tank], tankY[mySide][tank], true);
                if (enemyShoot != Invalid)
                {
                    cerr << "Our tank shoot to defend\n";
                    // shoot opposite direction
                    if (enemyShoot > RightShoot) act = Action((int)enemyShoot-2);
                    else act = Action((int)enemyShoot+2);
                }
            }
        }
        finalAct[tank] = act;
        // 防止我自己坦克移动到被射线
        if (ActionIsShoot(act))
            InitDangerZone(mySide, tank, ExtractDirectionFromAction(act));
        cerr << pathLength << " " << act << "\n";
    }
}

/* Our code end here */

int main()
{
    srand((unsigned)time(nullptr));
#ifndef _BOTZONE_ONLINE
    freopen("data.json", "r", stdin);
    //freopen("reponse.json", "w", stdout);
#endif
    string data, globaldata;
    while (true)
    {
        ReadInput(cin, data, globaldata);
        Greedy();
        SubmitAndDontExit(finalAct[0], finalAct[1]);
    }
}

