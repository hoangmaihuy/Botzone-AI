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
template<typename T> inline T operator- (T a, T b) { return (T)((int)a-(int)b); }

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

enum GameResult
{
    NotFinished = -2, Draw = -1, Blue = 0, Red = 1
};

enum FieldItem
{
    None = 0, Brick = 1, Steel = 2, Base = 4, Blue0 = 8, Blue1 = 16, Red0 = 32, Red1 = 64, Water = 128
};

enum Action
{
    Invalid = -2, Stay = -1, Up, Right, Down, Left, UpShoot, RightShoot, DownShoot, LeftShoot
};

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

// 判断 item 是不是叠在一起的多个坦克
inline bool HasMultipleTank(FieldItem item)
{
    // 如果格子上只有一个物件，那么 item 的值是 2 的幂或 0
    // 对于数字 x，x & (x - 1) == 0 当且仅当 x 是 2 的幂或 0
    return !!(item & (item - 1));
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

// 物件消失的记录，用于回退
struct DisappearLog
{
    FieldItem item;

    // 导致其消失的回合的编号
    int turn;

    int x, y;
    bool operator< (const DisappearLog& b) const
    {
        if (x == b.x)
        {
            if (y == b.y)
                return item < b.item;
            return y < b.y;
        }
        return x < b.x;
    }
};

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

// 坦克纵坐标，-1表示坦克已炸
int tankY[sideCount][tankPerSide] = { { 0, 0 },{ fieldHeight - 1, fieldHeight - 1 } };

// 当前回合编号
int currentTurn = 1;

// 我是哪一方
int mySide;

// 用于回退的log
stack<DisappearLog> logs;

// 过往动作（previousActions[x] 表示所有人在第 x 回合的动作，第 0 回合的动作没有意义）
Action previousActions[101][sideCount][tankPerSide] = { { { Stay, Stay },{ Stay, Stay } } };

//!//!//!// 以上变量设计为只读，不推荐进行修改 //!//!//!//

// 本回合双方即将执行的动作，需要手动填入
Action nextAction[sideCount][tankPerSide] = { { Invalid, Invalid },{ Invalid, Invalid } };

// 判断行为是否合法（出界或移动到非空格子算作非法）
// 未考虑坦克是否存活
bool ActionIsValid(int side, int tank, Action act)
{
    if (act == Invalid)
        return false;
    if (act > Left && previousActions[currentTurn - 1][side][tank] > Left) // 连续两回合射击
        return false;
    if (act == Stay || act > Left)
        return true;
    int x = tankX[side][tank] + dx[act],
        y = tankY[side][tank] + dy[act];
    return CoordValid(x, y) && gameField[y][x] == None;// water cannot be stepped on
}

// 判断 nextAction 中的所有行为是否都合法
// 忽略掉未存活的坦克
bool ActionIsValid()
{
    for (int side = 0; side < sideCount; side++)
        for (int tank = 0; tank < tankPerSide; tank++)
            if (tankAlive[side][tank] && !ActionIsValid(side, tank, nextAction[side][tank]))
                return false;
    return true;
}

void _destroyTank(int side, int tank)
{
    tankAlive[side][tank] = false;
    tankX[side][tank] = tankY[side][tank] = -1;
}

void _revertTank(int side, int tank, DisappearLog& log)
{
    int &currX = tankX[side][tank], &currY = tankY[side][tank];
    if (tankAlive[side][tank])
        gameField[currY][currX] &= ~tankItemTypes[side][tank];
    else
        tankAlive[side][tank] = true;
    currX = log.x;
    currY = log.y;
    gameField[currY][currX] |= tankItemTypes[side][tank];
}

// 执行 nextAction 中指定的行为并进入下一回合，返回行为是否合法
bool DoAction()
{
    if (!ActionIsValid())
        return false;

    // 1 移动
    for (int side = 0; side < sideCount; side++)
        for (int tank = 0; tank < tankPerSide; tank++)
        {
            Action act = nextAction[side][tank];

            // 保存动作
            previousActions[currentTurn][side][tank] = act;
            if (tankAlive[side][tank] && ActionIsMove(act))
            {
                int &x = tankX[side][tank], &y = tankY[side][tank];
                FieldItem &items = gameField[y][x];

                // 记录 Log
                DisappearLog log;
                log.x = x;
                log.y = y;
                log.item = tankItemTypes[side][tank];
                log.turn = currentTurn;
                logs.push(log);

                // 变更坐标
                x += dx[act];
                y += dy[act];

                // 更换标记（注意格子可能有多个坦克）
                gameField[y][x] |= log.item;
                items &= ~log.item;
            }
        }

    // 2 射♂击!
    set<DisappearLog> itemsToBeDestroyed;
    for (int side = 0; side < sideCount; side++)
        for (int tank = 0; tank < tankPerSide; tank++)
        {
            Action act = nextAction[side][tank];
            if (tankAlive[side][tank] && ActionIsShoot(act))
            {
                int dir = ExtractDirectionFromAction(act);
                int x = tankX[side][tank], y = tankY[side][tank];
                bool hasMultipleTankWithMe = HasMultipleTank(gameField[y][x]);
                while (true)
                {
                    x += dx[dir];
                    y += dy[dir];
                    if (!CoordValid(x, y))
                        break;
                    FieldItem items = gameField[y][x];
                    //tank will not be on water, and water will not be shot, so it can be handled as None
                    if (items != None && items != Water)
                    {
                        // 对射判断
                        if (items >= Blue0 &&
                            !hasMultipleTankWithMe && !HasMultipleTank(items))
                        {
                            // 自己这里和射到的目标格子都只有一个坦克
                            Action theirAction = nextAction[GetTankSide(items)][GetTankID(items)];
                            if (ActionIsShoot(theirAction) &&
                                ActionDirectionIsOpposite(act, theirAction))
                            {
                                // 而且我方和对方的射击方向是反的
                                // 那么就忽视这次射击
                                break;
                            }
                        }

                        // 标记这些物件要被摧毁了（防止重复摧毁）
                        for (int mask = 1; mask <= Red1; mask <<= 1)
                            if (items & mask)
                            {
                                DisappearLog log;
                                log.x = x;
                                log.y = y;
                                log.item = (FieldItem)mask;
                                log.turn = currentTurn;
                                itemsToBeDestroyed.insert(log);
                            }
                        break;
                    }
                }
            }
        }

    for (auto& log : itemsToBeDestroyed)
    {
        switch (log.item)
        {
        case Base:
        {
            int side = log.x == baseX[Blue] && log.y == baseY[Blue] ? Blue : Red;
            baseAlive[side] = false;
            break;
        }
        case Blue0:
            _destroyTank(Blue, 0);
            break;
        case Blue1:
            _destroyTank(Blue, 1);
            break;
        case Red0:
            _destroyTank(Red, 0);
            break;
        case Red1:
            _destroyTank(Red, 1);
            break;
        case Steel:
            continue;
        default:
            ;
        }
        gameField[log.y][log.x] &= ~log.item;
        logs.push(log);
    }

    for (int side = 0; side < sideCount; side++)
        for (int tank = 0; tank < tankPerSide; tank++)
            nextAction[side][tank] = Invalid;

    currentTurn++;
    return true;
}

// 游戏是否结束？谁赢了？
GameResult GetGameResult()
{
    bool fail[sideCount] = {};
    for (int side = 0; side < sideCount; side++)
        if ((!tankAlive[side][0] && !tankAlive[side][1]) || !baseAlive[side])
            fail[side] = true;
    if (fail[0] == fail[1])
        return fail[0] || currentTurn > maxTurn ? Draw : NotFinished;
    if (fail[Blue])
        return Red;
    return Blue;
}

/* 三个 int 表示场地 01 矩阵（每个 int 用 27 位表示 3 行）
   initialize gameField[][]
   brick>water>steel
*/


void InitField(int hasBrick[3],int hasWater[3],int hasSteel[3], int _mySide)
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
                mask <<= 1;
            }
        }
    }
    for (int side = 0; side < sideCount; side++)
    {
        for (int tank = 0; tank < tankPerSide; tank++)
            gameField[tankY[side][tank]][tankX[side][tank]] = tankItemTypes[side][tank];
        gameField[baseY[side]][baseX[side]] = Base;
    }
}
/* Internals */
Json::Reader reader;
#ifdef _BOTZONE_ONLINE
Json::FastWriter writer;
#else
Json::StyledWriter writer;
#endif

void _processRequestOrResponse(Json::Value& value, bool isOpponent)
{
    if (value.isArray())
    {
        if (!isOpponent)
        {
            for (int tank = 0; tank < tankPerSide; tank++)
                nextAction[mySide][tank] = (Action)value[tank].asInt();
        }
        else
        {
            for (int tank = 0; tank < tankPerSide; tank++)
                nextAction[1 - mySide][tank] = (Action)value[tank].asInt();
            DoAction();
        }
    }
    else
    {
        // 是第一回合，裁判在介绍场地
        int hasBrick[3],hasWater[3],hasSteel[3];
        for (int i = 0; i < 3; i++){//Tank2 feature(???????????????)
            hasWater[i] = value["waterfield"][i].asInt();
            hasBrick[i] = value["brickfield"][i].asInt();
            hasSteel[i] = value["steelfield"][i].asInt();
        }
        InitField(hasBrick,hasWater,hasSteel,value["mySide"].asInt());
    }
}

void _submitAction(Action tank0, Action tank1, string debug = "", string data = "", string globalData = "")
{
    Json::Value output(Json::objectValue), response(Json::arrayValue);
    response[0U] = tank0;
    response[1U] = tank1;
    output["response"] = response;
    if (!debug.empty())
        output["debug"] = debug;
    if (!data.empty())
        output["data"] = data;
    if (!globalData.empty())
        output["globalData"] = globalData;
    cout << writer.write(output) << endl;
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

    if (input.isObject())
    {
        Json::Value requests = input["requests"], responses = input["responses"];
        if (!requests.isNull() && requests.isArray())
        {
            int i, n = requests.size();
            for (i = 0; i < n; i++)
            {
                _processRequestOrResponse(requests[i], true);
                if (i < n - 1)
                    _processRequestOrResponse(responses[i], false);
            }
            outData = input["data"].asString();
            outGlobalData = input["globaldata"].asString();
            return;
        }
    }
    _processRequestOrResponse(input, true);
}

// 提交决策并退出，下回合时会重新运行程序
void SubmitAndExit(Action tank0, Action tank1, string debug = "", string data = "", string globalData = "")
{
    _submitAction(tank0, tank1, debug, data, globalData);
    exit(0);
}

/* Our code start here */

const Action moveOrder[2][2][4] = {
    {{ Down, Left, Right, Up },
    { Down, Right, Left, Up }},
    {{ Up, Right, Left, Down },
    { Up, Left, Right, Down }}
};

Action finalAct[2] = {Stay, Stay};
bool danger[fieldHeight][fieldWidth];
bool tankCanShoot[2][2];

inline int MahattanDistance(int x1, int y1, int x2, int y2)
{
    return abs(x1-x2) + abs(y1-y2);
}

Action CanDestroy(int i, int j, int desX, int desY)
{
    for (Action act = UpShoot; act <= LeftShoot; act = Action(act+1))
    {
        int dir = ExtractDirectionFromAction(act);
        int x = i, y = j;
        while (true)
        {
            x += dx[dir]; y += dy[dir];
            if (!CoordValid(x, y)) break;
            FieldItem item = gameField[y][x];
            if (x == desX && y == desY) return act;
            if (item != None && item != Water)
                break;
        }
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
        if (item != None && item != Water && item != tankItemTypes[mySide][0] && item != tankItemTypes[mySide][1]) break;
        danger[y][x] = true;
    }
}

struct state
{
    // f : total cost, g : number of brick to destroy, (x, y) tank location
    int f, g, x, y;
    int timer;
    state() { }
    state(int f, int g, int y, int x, int timer) : f(f), g(g), x(x), y(y), timer(timer) { }
    bool operator < (const state &other) const
    {
        if (tie(f, g) != tie(other.f, other.g)) return tie(f, g) < tie(other.f, other.g);
        return timer < other.timer;
    }
};

int dis[fieldHeight][fieldWidth];
int br[fieldHeight][fieldWidth];
set<state> pq;
Action firstAct[fieldHeight][fieldWidth];

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

// find path for a tank to destroy object in (desX, desY) loc
// return path length and the first action

pair<int, Action> FindPath(int side, int tank, int desX, int desY, bool watchDanger)
{
    pq.clear();
    REP(i, N) REP(j, N)
    {
        dis[i][j] = br[i][j] = INF;
        firstAct[i][j] = Invalid;
    }
    bool saveDanger[2][2];
    REP(side1, 2) REP(tank1, 2)
    {
        int x = tankX[side1][tank1], y = tankY[side1][tank1];
        gameField[y][x] = None;
        if (tank != tank1 || side != side1)
        {
            saveDanger[side1][tank1] = danger[y][x];
            danger[y][x] = true;
        }
    }
    int x = tankX[side][tank], y = tankY[side][tank];
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
        if (f && CanDestroy(x, y, desX, desY) != Invalid) // not first move
        {
            if (smaller(f+1, g, minPath, minBrick))
            {
                minPath = f+1; minBrick = g;
                shouldAct = firstAct[y][x];
            }
            continue;
        }
        // tank in same row with enemy base then stay here and shoot

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
            if (cntBrick != -1) // if there is steel between tank and base
            {
	            int timeNeed = cntBrick*2+1;
	            bool needToStay = false;
	            // if this is first move and last turn is shoot then stay for one turn and begin shooting
	            if (!f && ActionIsShoot(prevAct(1, side, tank)))
	            {
	                timeNeed++;
	                needToStay = true;
	            }
	            if (smaller(f+timeNeed, g+cntBrick, minPath, minBrick))
	            {
	                minPath = f+timeNeed; minBrick = g+cntBrick;
	                if (needToStay) shouldAct = Stay;
	                else if (!f) shouldAct = Action(dir+4); // shoot
	                else shouldAct = firstAct[y][x];
	            }
			}
        }
        // only consider move action, if moving in brick then shoot to destroy
        REP(i, 4)
        {
            int act = moveOrder[side][tank][i];
            int cost = 1, brick = 0; Action nextAct = (Action)act;
            int u = x + dx[act], v = y + dy[act];
            if (CoordValid(u, v) && (gameField[v][u] == None || gameField[v][u] == Brick))
            {
                if (!f && watchDanger && danger[v][u]) continue;
                if (gameField[v][u] == Brick)
                {
                    if (!f && ActionIsShoot(previousActions[currentTurn-1][side][tank])) continue;
                    nextAct = Action(act+4);
                    cost = 2;
                    brick = 1;
                }
                // minimize total cost, if total cost equal then minimize number of brick that have to destroy
                if (dis[v][u] > f+cost || (dis[v][u] == f+cost && br[v][u] > g+brick))
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
    REP(side1, 2) REP(tank1, 2)
    {
        int x = tankX[side1][tank1], y = tankY[side1][tank1];
        gameField[y][x] = None;
        if (tank != tank1 || side != side1)
        {
            danger[y][x] = saveDanger[side1][tank1];
        }
    }
    // can't find a path
    return make_pair(minPath, shouldAct);
}

bool NothingBetween(int x1, int y1, int x2, int y2)
{
    if (x1 == x2)
    {
        if (y1 > y2) swap(y1, y2);
        For(yy, y1+1, y2-1)
        if (gameField[yy][x1] != None && gameField[yy][x1] != Water) return false;
    } else // y1 == y2
    {
        if (x1 > x2) swap(x1, x2);
        For(xx, x1+1, x2-1)
        if (gameField[y1][xx] != None && gameField[y1][xx] != Water) return false;
    }
    return true;
}

// check if tank (side, tank) can avoid shoot
bool CanAvoid(int side, int tank, Action shoot)
{
    if (tankCanShoot[side][tank]) return true; // can shoot to defend
    int x = tankX[side][tank], y = tankY[side][tank];
    if (shoot == UpShoot || shoot == DownShoot) // has to move left or right
    {
        for (int xx = x-1; xx <= x+1; xx += 2)
            if (CoordValid(xx, y) && gameField[y][xx] == None) return true;
    } else // move up or down
    {
        for (int yy = y-1; yy <= y+1; yy += 2)
            if (CoordValid(x, yy) && gameField[yy][x] == None) return true;
    }
    return false;
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

void Greedy()
{
    REP(side, 2) REP(tank, 2)
        tankCanShoot[side][tank] = (tankAlive[side][tank] && !ActionIsShoot(prevAct(1, side, tank)));
    memset(danger, false, sizeof danger);
    for (int tank = 0; tank < 2; tank++)
    if (tankCanShoot[1-mySide][tank])
    {
        // if enemy tank stay for 2 continuos turns then assume this turn also stay
        //if (currentTurn >= 4 && prevAct(1, 1-mySide, tank) == Stay
        //    && prevAct(2, 1-mySide, tank)) continue;
        int x = tankX[1-mySide][tank], y = tankY[1-mySide][tank];
        // if enemy tank and our tank in a same cell then ignore enemy tank shoot
        if (HasMultipleTank(gameField[y][x]))
        {
            FieldItem otherTank = gameField[y][x] - tankItemTypes[1-mySide][tank];
            if (GetTankSide(otherTank) == mySide) continue;
        }
        for (int dir = 0; dir < 4; dir++)
            InitDangerZone(1-mySide, tank, dir);
    }

    for (int tank = 0; tank < 2; tank++)
    if (tankAlive[mySide][tank])
    {
    	REP(side, 2) REP(tank, 2)
	    {
	        gameField[tankY[side][tank]][tankX[side][tank]] = tankItemTypes[side][tank];
	    }
        if (tankCanShoot[mySide][tank])
        {
            // can shoot base
            auto act = CanDestroy(tankX[mySide][tank], tankY[mySide][tank], baseX[1-mySide], baseY[1-mySide]);
            if (act != Invalid)
            {
                finalAct[tank] = act;
                continue;
            }
            // can kill enemy's tank, in case enemy's tank can't avoid the shoot
            REP(tank2, 2)
            {
                act = CanDestroy(tankX[mySide][tank], tankY[mySide][tank], tankX[1-mySide][tank2], tankY[1-mySide][tank2]);
                if (act != Invalid && !CanAvoid(1-mySide, tank2, act))
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
        if (act == UpShoot || act == DownShoot)
        {
            int dir = ExtractDirectionFromAction(act);
            int xx = tankX[mySide][tank], yy = tankY[mySide][tank];
            bool flag = true;
            int xB = xx+dx[dir], yB = yy+dy[dir];
            if (gameField[yB][xB] == Brick)
            {
                gameField[yB][xB] = None;
                REP(tank2, 2)
                if (tankAlive[1-mySide][tank2])
                {
                    int x1 = tankX[1-mySide][tank2], y1 = tankY[1-mySide][tank2];
                    if (CanDestroy(x1, y1, xx, yy) != Invalid && InBetween(xx, yy, x1, y1, xB, yB))
                    {
                        flag = false; break;
                    }
                    REP(dir2, 4)
                    {
                        int x2 = x1+dx[dir2], y2 = y1+dy[dir2];
                        if (CoordValid(x2, y2) && gameField[y2][x2] == None
                            && CanDestroy(x2, y2, xx, yy) != Invalid && InBetween(xx, yy, x2, y2, xB, yB))
                        {
                            flag = false; break;
                        }
                    }
                }
                gameField[yy+dy[dir]][xx+dx[dir]] = Brick;
                Action actBefore = act;
                if (!flag) act = Stay;
                if (act == Stay && prevAct(1, mySide, tank) == Stay && prevAct(2, mySide, tank) == Stay)
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
        // if tank action is move then don't need to check
        // if tank stay in current location, check if enemy tank can kill this tankand defend
        if (tankCanShoot[mySide][tank] && (act == Stay || ActionIsShoot(act)))
        {
            REP(tank2, 2)
            if (tankCanShoot[1-mySide][tank2])
            {
                Action enemyShoot = CanDestroy(tankX[1-mySide][tank2], tankY[1-mySide][tank2], tankX[mySide][tank], tankY[mySide][tank]);
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
    ReadInput(cin, data, globaldata);
   // cerr << tankY[mySide][0] << ' ' << tankX[mySide][0] << ' ' << tankY[mySide][1] << ' ' << tankX[mySide][1] << "\n";
    Greedy();
    SubmitAndExit(finalAct[0], finalAct[1]);
}
