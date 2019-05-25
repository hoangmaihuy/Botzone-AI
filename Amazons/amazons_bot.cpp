#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <tuple>
#include <cstring>
#include <map>
#include <queue>

using namespace std;

typedef pair<int, int> cell;
typedef pair<int, int> II;
typedef vector<vector<int>> grid;

#define For(i,a,b) for(int i = a;i <= b; i++)
#define Rep(i,a,b) for(int i = a;i >= b; i--)
#define REP(i, n) for(int i = 0; i < n; i++)
#define FOR(i, f) for(auto i : f)
#define x first
#define y second
#define pb push_back
#define eb emplace_back
#define mp make_pair
#define BUG(x) (cerr << #x << " = " << x << "\n")
#define ARRAY(f) {cerr << #f << " = "; FOR(i, f) cout << i << ' '; cout << "\n";}
#define sz(s) int(s.size())
#define reset(f, x) memset(f, x, sizeof(f))
#define all(x) x.begin(), x.end()
#define two(x) (1LL << x)
#define bit(x, i) ((x >> (i)) & 1LL)
#define onbit(x, i) (x | (1LL << (i)))
#define offbit(x, i) (x & ~(1 << (i)))
#define black 1
#define white -1
#define obj 2
#define data _data

const int N = 8;
const int INF = 1e9;
const int dx[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
const int dy[8] = {-1, 0, 1, 1, 1, 0, -1, -1};

grid a(N, vector<int>(N, 0));
cell from, to, object;
int turnID, cur;
int dd[N][N];
int d1[N][N], d2[N][N];
vector<cell> pos[2];
int cnt;
int Depth;

inline bool inMap(int x, int y)
{
    return (x >= 0 && y >= 0 && x < N && y < N);
}

inline bool ProcStep(cell from, cell to, cell object, int color, bool check_only)
{
    int x1, y1, x2, y2, x3, y3;
    tie(x1, y1) = from;
    tie(x2, y2) = to;
    tie(x3, y3) = object;
    if (!inMap(x1, y1) || !inMap(x2, y2) || !inMap(x3, y3)) return false;
    if (a[x1][y1] != color || a[x2][y2] != 0) return false;
    if (a[x2][y2] != 0 && !(x3 == x1 && y3 == y1)) return false;
    if (!check_only)
    {
        a[x1][y1] = 0;
        a[x2][y2] = color;
        a[x3][y3] = obj;
    }
}

void dfs(int i, int j)
{
    dd[i][j] = 1;
    REP(k, 8)
    {
        int u = i + dx[k], v = j + dy[k];
        if (inMap(u, v) && a[u][v] == 0 && !dd[u][v]) dfs(u, v);
    }
}

queue<II> q;

inline int bfs(int color, int depth)
{
    int cnt = 0;
    int t;
    if (color == 1) t = 1; else t = 0;
    reset(dd, 0);
    REP(ii, sz(pos[t]))
    {
        int i = pos[t][ii].x, j = pos[t][ii].y;
        q.push(II(i, j));
    }
    while (!q.empty())
    {
        int i, j;
        tie(i, j) = q.front();
        q.pop();
        REP(k, 8) For(d, 1, 7)
        {
            int u = i + dx[k]*d, v = j + dy[k]*d;
            if (inMap(u, v))
            {
                if (a[u][v] != 0) break;
                if (dd[u][v] == 0)
                {
                    dd[u][v] = dd[i][j] + 1;
                    cnt++;
                    if (dd[u][v] < depth)
                    {
                        q.push(II(u, v));

                    }
                }
            } else break;
        }
    }
   // cerr << cnt << "\n";
    return cnt;
}

inline int CalArea(int color)
{
    if (turnID <= 7)
    {
        return bfs(color, 1) - bfs(-color, 1);
    } else
    {
        int ans = 0;
        bfs(color, INF);
        REP(i, N) REP(j, N)
        {
            if (dd[i][j] == 1) ans++;
            d1[i][j] = dd[i][j];
        }
        bfs(-color, INF);
        REP(i, N) REP(j, N)
        {
            if (dd[i][j] == 1) ans--;
            if (d1[i][j] != 0 && dd[i][j] != 0)
            {
                if (d1[i][j] < dd[i][j]) ans += 4;
                else if (d1[i][j] > dd[i][j]) ans -= 4;
            }
        }
        return ans;
    }
}

inline void apply_change(int x1, int y1, int x2, int y2, int x3, int y3, int color, bool revert)
{
    int t = color;
    if (t == -1) t = 0;
    if (!revert)
    {
        swap(a[x1][y1], a[x2][y2]);
        a[x3][y3] = 2;
        REP(ii, sz(pos[t])) if (pos[t][ii] == cell(x1, y1)) pos[t][ii] = cell(x2, y2);
    }
    else
    {
        a[x3][y3] = 0;
        swap(a[x1][y1], a[x2][y2]);
        REP(ii, sz(pos[t])) if (pos[t][ii] == cell(x2, y2)) pos[t][ii] = cell(x1, y1);
    }
}

int solve(int color)
{
    int t = color;
    if (color == -1) t = 0;
    int rx1, ry1, rx2, ry2, rx3, ry3, best = -INF;
    REP(ii, 4)
    {
        int x1 = pos[t][ii].x, y1 = pos[t][ii].y;
        REP(k1, 8) For(d1, 1, 7)
        {
            int x2 = x1 + dx[k1]*d1, y2 = y1 + dy[k1]*d1;
            if (!inMap(x2, y2)) break;
            if (a[x2][y2] != 0) break;
            REP(k2, 8) For(d2, 1, 7)
            {
                int x3 = x2 + dx[k2]*d2, y3 = y2 + dy[k2]*d2;
                if (!inMap(x3, y3)) break;
                if (a[x3][y3] != 0 && !(x3 == x1 && y3 == y1)) break;
                apply_change(x1, y1, x2, y2, x3, y3, color, false);
                int area;
                if (cur == color)
                    area = solve(-color);
                else area = CalArea(color);
                if (area > best)
                {
                    best = area;
                    if (color == cur) tie(rx1, ry1, rx2, ry2, rx3, ry3) = tie(x1, y1, x2, y2, x3, y3);
                }
                apply_change(x1, y1, x2, y2, x3, y3, color, true);
            }
        }
    }
    if (best == -INF)
    {
        if (cur == color) from = to = object = cell(-1, -1);
        return 0;
    } else
    {
        if (cur == color)
        {
            from = II(rx1, ry1);
            to = II(rx2, ry2);
            object = II(rx3, ry3);
           // cerr << best << "\n";
        }
        if (cur == color) return best;
        else return -best;
    }
}

map<pair<grid, int>, bool> data;
map<pair<grid, int>, vector<cell>> step;

bool dp(int color)
{
   // cnt++;
   // cout << cnt << "\n";
    int t = color;
    if (t == -1) t = 0;
    auto it = data.find(make_pair(a, color));
    if (it != data.end()) return it->second;
    bool flag = false;
    REP(ii, 4)
    {
        int x1 = pos[t][ii].x, y1 = pos[t][ii].y;
        REP(k1, 8) For(d1, 1, 7)
        {
            int x2 = x1 + dx[k1]*d1, y2 = y1 + dy[k1]*d1;
            if (!inMap(x2, y2)) break;
            if (a[x2][y2] != 0) break;
            REP(k2, 8) For(d2, 1, 7)
            {
                int x3 = x2 + dx[k2]*d2, y3 = y2 + dy[k2]*d2;
                if (!inMap(x3, y3)) break;
                if (a[x3][y3] != 0 && !(x3 == x1 && y3 == y1)) break;
                apply_change(x1, y1, x2, y2, x3, y3, color, false);
                if (!dp(-color)) flag = true;
                apply_change(x1, y1, x2, y2, x3, y3, color, true);
                if (flag)
                {
                    auto state = make_pair(a, color);
                    step[state].eb(x1, y1);
                    step[state].eb(x2, y2);
                    step[state].eb(x3, y3);
                    break;
                }
            }
            if (flag) break;
        }
        if (flag) break;
    }
    auto state = make_pair(a, color);
    data[state] = flag;
    if (!flag)
    {
        step[state].eb(-1, -1);
        step[state].eb(-1, -1);
        step[state].eb(-1, -1);
    }
    return flag;
}

int main()
{
  //  freopen("in.txt", "r", stdin);
    a[0][2] = a[2][0] = a[5][0] = a[7][2] = black;
    a[0][5] = a[2][7] = a[5][7] = a[7][5] = white;
    cur = white;
   /* REP(i, N)
    {
        REP(j, N) cout << a[j][i] << ' '; cout << "\n";
    }
    */
    cin >> turnID;
    REP(turn, turnID)
    {
        cin >> from.x >> from.y >> to.x >> to.y >> object.x >> object.y;
        if (from.x == -1)
            cur = black;
        else
            ProcStep(from, to, object, -cur, false);

        if (turn < turnID-1)
        {
            cin >> from.x >> from.y >> to.x >> to.y >> object.x >> object.y;
            if (from.x >= 0)
            {
                ProcStep(from, to, object, cur, false);
            }
        }
    }
    /*
    REP(i, N)
    {
        REP(j, N) cerr << a[i][j] << ' ';
        cerr << "\n";
    }
    */
    REP(i, N) REP(j, N)
    {
        if (a[i][j] == 1) pos[1].pb(cell(i, j));
        else if (a[i][j] == -1) pos[0].pb(cell(i, j));
    }
    //cerr << CalArea(1) << "\n";
    if (turnID <= 23)
    {
        solve(cur);
    }
    else
    {
        dp(cur);
        auto state = make_pair(a, cur);
        from = step[state][0];
        to = step[state][1];
        object = step[state][2];
    }

    cout << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << ' ' << object.x << ' ' << object.y << "\n";
}
