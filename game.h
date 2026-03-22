// game.h
#ifndef GAME_GAME_H
#define GAME_GAME_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>
#include <algorithm>
#include <random>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <array>
#include <functional>
#include <queue>
#include <stdexcept>
#include <cmath>
#include <sstream>


using namespace std;

//---------------------------------游戏逻辑部分---------------------------------//

class Queue {
public:
    Queue() {}

    void push(const std::vector<int>& item) {
        list.push_back(item);
    }

    std::vector<int> pop() {
        if (isEmpty()) {
            throw std::runtime_error("Queue is empty");
        }
        std::vector<int> item = list.front();
        list.erase(list.begin());
        return item;
    }

    bool isEmpty() const {
        return list.empty();
    }

private:
    std::vector<std::vector<int>> list; // 存储 std::vector<int> 类型的元素
};

// 哈希函数
std::hash<std::string> hasher;  // 哈希值

int hashPos(const std::vector<int>& pos) {
    std::string hpos = std::to_string(pos[0]) + "," + std::to_string(pos[1]) + "," + std::to_string(pos[2]);  // 将坐标连成字符串
    return static_cast<int>(hasher(hpos));   // 转换为 int
}

//---------------------------------------------------------------------------------------------------

class Game {
private:
    int P1;                      // 玩家一，执黑棋，先手
    int P2;                      // 玩家二，执白棋，后手
    vector<vector<int>> NEAR;    // 周围一圈的坐标偏移量，从上方开始顺时针
    vector<int> STARTPOSITION;   // 默认的起始位置，棋子在这个坐标时表示尚未下该子
    int numOfPlayers;            // 人类玩家数量。取值为1或2
    vector<vector<vector<int>>> map; // 棋盘状态，顺序是1蜂后，3工蚁，2蜘蛛，3蟋蟀，2甲虫
    vector<int> action;          // 记录最后一步棋子的位置，游戏中高亮显示
    int round;                   // 记录当前回合数
    int currentPlayer;           // 当前下子的玩家
    int first;                   // 记录首个玩家 0 移动的棋子编号
    int drawFlag;                // 平局标志
    unordered_map<int, vector<pair<int, int>>> find; // 位置查找表，键为位置哈希值，值为玩家编号和棋子编号
    vector<vector<int>> angleWeight;     // 角度权重
    vector<int> axleWeight;      // 轴向权重
    vector<std::vector<int>> kindWeight; // 类型权重

    // 角度权重计算
    vector<vector<int>> getAngleWeight() {
        vector<vector<int>> weight(60, vector<int>(60, 0));
        for (int i = 0; i < 30; ++i) {
            int x = 0;
            int y = i;
            int value = i * 6;
            weight[x + 30][y + 30] = value;
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < i; ++k) {
                    x += NEAR[j + 2][0];
                    y += NEAR[j + 2][1];
                    value -= 1;
                    weight[x + 30][y + 30] = value;
                    weight[-x + 30][y - x + 30] = value;
                }
            }
        }
        return weight;
    }

    // 轴向权重计算
    vector<int> getAxleWeight() {
        vector<int> weight(60, 0);
        for (int i = 0; i < 60; ++i) {
            weight[i] = i - 30;
        }
        return weight;
    }

public:
    Game() {
        P1 = 0;                      // 玩家一，执黑棋，先手
        P2 = 1;                      // 玩家二，执白棋，后手
        NEAR = { {0, 1}, {1, 1}, {1, 0}, {0, -1}, {-1, -1}, {-1, 0} }; // 周围一圈的坐标偏移量，从上方开始顺时针
        STARTPOSITION = { 0, 0, -1 };  // 默认的起始位置，棋子在这个坐标时表示尚未下该子
        numOfPlayers = 2;            // 人类玩家数量。取值为1或2
        map = { {}, {} };                 // 棋盘状态 。顺序是1蜂后，3工蚁，2蜘蛛，3蟋蟀，2甲虫
        action = { 0, 0, -1 };          // 记录最后一步棋子的位置，游戏中高亮显示
        round = 1;                   // 记录当前回合数
        currentPlayer = P1;        // 当前下子的玩家
        first = -1;
        drawFlag = 0;

        for (int i = 0; i < 11; ++i) { // 初始化所有棋子
            map[0].push_back(STARTPOSITION);
            map[1].push_back(STARTPOSITION);
        }
        angleWeight = getAngleWeight();
        axleWeight = getAxleWeight();
        kindWeight = { {20, 15, 15, 15, 10, 10, 5, 5, 5, 1, 1}, {45, 40, 40, 40, 35, 35, 30, 30, 30, 25, 25} };
    }

    void restart() {
        *this = Game();
    }

    void changePlayer() {
        currentPlayer = 1 - currentPlayer;
    }

    int getCurrentPlayer() const {
        return currentPlayer;
    }

    int getRound() const {
        return round;
    }

    void setNumOfPlayers(int num) {
        numOfPlayers = num;
    }

    int getNumOfPlayers() const {
        return numOfPlayers;
    }

    pair<vector<vector<vector<int>>>, vector<int>> getCurrentState() const {
        return { map, action };
    }

    //----------------------------------------------------------------------------
    bool moveChess(int id, const std::vector<int>& goal) {
        if ((round == 1) && (currentPlayer == P1)) {
            first = id; // 如果当前是第一轮并且当前玩家是玩家 P1，则记录第一个移动的棋子编号 id
        }

        if (map[currentPlayer][id] != STARTPOSITION) {
            if (map[currentPlayer][id][2] > 1) {        //如果这颗棋子高度大于1
                int pos = hashPos(map[currentPlayer][id]);
                auto& positions = find[pos];            //查找这颗棋子所在位置的哈希值是否在find查找表中
                if (!positions.empty()) {
                    positions.pop_back();               //移除最后一个元素，即棋子所在二维坐标的最上面一层
                    if (positions.empty()) {
                        find.erase(pos);
                    }
                }
            }
            else {
                find.erase(hashPos(map[currentPlayer][id]));   //如果该位子上只有该棋子，则移除所有位置信息（该位置哈希值）
            }
        }

        map[currentPlayer][id] = goal;

        // 向find中添加条目，可以说pos代表了位置，
        int pos = hashPos(goal);
        auto it = find.find(pos);
        if (it == find.end()) {
            find[pos] = { {currentPlayer, id} };     // 如果没有找到pos条目，则创建一个新条目
        }
        else {                           //如果 find 查找表中已经存在键为 pos 的条目，那么 it 指向的就是这个条目。
            it->second.emplace_back(make_pair(currentPlayer, id));      // it->second 访问的是这个条目的值部分，即一个存储了该位置上所有棋子信息的向量。
            //将当前玩家编号和棋子编号作为一个新的元素追加到这个向量的末尾
        }

        action = goal;

        if (currentPlayer == P2) { // 玩家二下完子后，回合数加一
            round++;
        }

        changePlayer(); // 玩家交替
        drawFlag = 0;

        return true;
    }

    //--------------------------------------------------------------------------------

    void skipRound() {
        changePlayer();
        drawFlag += 1;
    }

    int getFirst() const {
        return first;
    }

    bool isLose(int player) { // 判断该玩家是否失败
        if (map[player][0] == STARTPOSITION) { // 若蜂后还未下出，则不会失败
            return false;
        }
        // 检查蜂后周围是否有空位
        return getBlank(player, 0).empty(); // 若蜂后周围没有空位，则失败
    }

    bool isWin(int player) { // 判断该玩家是否胜利
        return isLose(1 - player); // 若对手失败，则玩家胜利
    }

    bool isDraw() const {
        return drawFlag >= 2;
    }

    bool isEnd() {
        return isWin(P1) || isWin(P2) || isDraw();
    }

    //--------------------------------------------------------------------------------------

    vector<vector<int>> getBlank(int player, int id) {
        vector<vector<int>> blank;
        if (map[player][id] == STARTPOSITION) {   // 如果当前棋子还没出，则没有blank
            return blank;
        }
        for (auto& near : NEAR) {                    // 遍历周围棋子
            vector<int> checkPos = {                    // vector中存放当前遍历到的棋子坐标
                    map[player][id][0] + near[0],
                    map[player][id][1] + near[1],
                    1
            };
            int posKey = hashPos(checkPos);
            if (find.find(posKey) == find.end()) {      // 如果checkPos的哈希值不在find中，则将它加入blank
                blank.push_back(checkPos);
            }
        }
        return blank;
    }

    //------------------------------------------------------------------------------

    vector<vector<int>> getSuccessor(int id) {    //返回特定id的后继位置
        vector<vector<int>> next;                // 函数返回值next存放所有可能的后继坐标（二维）
        if (round == 1) {                        // 第一轮玩家一放在0，0，1
            if (currentPlayer == P1) {
                next.push_back({ 0, 0, 1 });
                return next;                       //ok
            }
            else {
                for (auto& near : NEAR) {
                    next.push_back({ near[0], near[1], 1 });
                }
                return next;                      //ok
            }
        }
        if (map[currentPlayer][0] == STARTPOSITION) {
            if ((round == 4) && (id != 0)) {
                return next;                                                   //ok
            }
            else if (map[currentPlayer][id] != STARTPOSITION) {              //ok
                return next;
            }
        }
        if (map[currentPlayer][id] == STARTPOSITION) {  //查看不在场上棋子的后继
            for (int i = 0; i < 11; ++i) {
                if (map[currentPlayer][i][2] == 1) {    //遍历当前玩家在场上的棋
                    auto blanks = getBlank(currentPlayer, i);
                    for (auto& blank : blanks) {       // 遍历棋子i的空白位置
                        bool found = false;
                        for (auto& n : next) {         //检查空白位置是否在后继列表中，在说明找到了后继
                            if (n == blank) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {                   //不在则将这个空位加到后继列表中（避免场上棋子周围的空位重复）
                            next.push_back(blank);
                        }
                    }
                }
            }
            return next;                                //ok
        }
        if (isCovered(currentPlayer, id)) {
            return next;
        }
        if (id == 0) {
            next = getBeeSuccessor(currentPlayer);
            return next;
        }
        if (id >= 1 && id <= 3) {
            next = getAntSuccessor(currentPlayer, id);
            return next;
        }
        if (id >= 4 && id <= 5) {
            next = getSpiderSuccessor(currentPlayer, id);
            return next;
        }
        if (id >= 6 && id <= 8) {
            next = getCricketSuccessor(currentPlayer, id);
            return next;
        }
        if (id >= 9 && id <= 10) {
            next = getBeetleSuccessor(currentPlayer, id);
            return next;
        }
        return next;
    }

    //---------------------------------------------------------------------------
    bool isCovered(int player, int id) const {
        std::vector<int> checkPos = map[player][id];
        int posHash = hashPos(checkPos);
        if (find.find(posHash) == find.end()) {
            return false;
        }
        return checkPos[2] != static_cast<int>(find.at(posHash).size());
    }

    //------------------------------------------------------------------------------------------------
    vector<vector<int>> getBeeSuccessor(int player) {  // 蜂后只能在桌面上，一次移动一步
        return getLegalOneSteps(player, 0);
    }

    //---------------------------------------------------------------------------------------------------

    vector<vector<int>> getAntSuccessor(int player, int id) {
        vector<vector<int>> next;
        Queue queue;
        vector<int> begin = map[player][id];
        queue.push(begin);
        next.push_back(begin);  // 为了方便，先将起始点添加进队列中，最后再删去

        // 使用 BFS，不在过程中修改游戏状态
        unordered_map<int, bool> visited;
        visited[hashPos(begin)] = true;

        while (!queue.isEmpty()) {
            vector<int> node = queue.pop();
            // 获取当前节点的合法移动
            vector<vector<int>> oneSteps = getLegalOneSteps(player, id);
            for (const auto& step : oneSteps) {
                int stepHash = hashPos(step);
                if (!visited[stepHash]) {
                    next.push_back(step);
                    queue.push(step);
                    visited[stepHash] = true;
                }
            }
        }

        // 移除起始点
        if (!next.empty()) {
            next.erase(next.begin());
        }

        return next;
    }

    //------------------------------------------------------------------------------
    void dfs(int depth, const std::vector<std::vector<int>>& path, int player, int id, std::vector<std::vector<int>>& next) {
        if ((depth == 3) && (std::find(next.begin(), next.end(), path[3]) == next.end())) {
            next.push_back(path[3]);
        }
        else if (depth < 3) {
            const auto& current = path[depth];

            vector<vector<int>> oneSteps = getLegalOneSteps(player, id);
            for (const auto& step : oneSteps) {
                if (std::find(path.begin(), path.end(), step) == path.end()) {
                    // 继续递归
                    std::vector<std::vector<int>> newPath = path;
                    newPath.push_back(step);
                    dfs(depth + 1, newPath, player, id, next);
                }
            }
        }
    }

    //-----------------------------------------------------------------------------------------------------

    vector<vector<int>> getSpiderSuccessor(int player, int id) {        // 蜘蛛
        vector<vector<int>> next;
        dfs(0, { map[player][id] }, player, id, next);
        return next;
    }

    //-----------------------------------------------------------------------------------------------------
    vector<vector<int>> getCricketSuccessor(int player, int id) {
        vector<vector<int>> next;
        const auto& begin = map[player][id];

        // 检查蟋蟀离开是否会导致不连通
        vector<int> tempPos = STARTPOSITION;
        map[player][id] = tempPos;
        bool connected = isConnectivity();
        map[player][id] = begin;

        if (!connected) {
            return next;
        }

        for (int i = 0; i < 6; ++i) {
            vector<int> dir = NEAR[i];
            vector<int> pos = begin;
            bool hasNeighbor = false;
            while (true) {
                pos[0] += dir[0];
                pos[1] += dir[1];
                int posHash = hashPos(pos);
                if (find.find(posHash) != find.end()) {
                    hasNeighbor = true;
                    continue;
                }
                else {
                    if (hasNeighbor) {
                        next.push_back({ pos[0], pos[1], 1 });
                    }
                    break;
                }
            }
        }
        return next;
    }

    //-----------------------------------------------------------------------------------------------
    vector<vector<int>> getBeetleSuccessor(int player, int id) {            // 甲虫
        vector<vector<int>> next;
        vector<vector<int>> around;
        const auto& begin = map[player][id];

        for (const auto& near : NEAR) {
            vector<int> checkPos = { begin[0] + near[0], begin[1] + near[1], 0 };
            int posHash = hashPos({ checkPos[0], checkPos[1], 1 });
            if (find.find(posHash) != find.end()) {
                checkPos[2] = static_cast<int>(find[posHash].size());
            }
            else {
                checkPos[2] = 0;
            }
            around.push_back(checkPos);
        }

        for (int goal = 0; goal < 6; ++goal) {
            bool flag = true;
            int left = (goal - 1 + 6) % 6;
            int right = (goal + 1) % 6;
            const auto& checkPos = around[goal];
            const auto& leftPos = around[left];
            const auto& rightPos = around[right];

            if (begin[2] > 1) {
                if (std::min(leftPos[2], rightPos[2]) > std::max(checkPos[2], begin[2] - 1)) {
                    flag = false;
                }
            }
            else {
                if (!((leftPos[2] == 0 && rightPos[2] == 0) || (leftPos[2] > 0 && rightPos[2] > 0))) {
                    flag = false;
                }
            }

            if (flag) {
                next.push_back({ checkPos[0], checkPos[1], checkPos[2] + 1 });
            }
        }

        return next;
    }

    //------------------------------------------------------------------------------------------------
    bool isConnectivity() {
        vector<vector<bool>> domain(2, vector<bool>(11, true)); // 查重数组

        for (auto& temp : find) {
            for (auto& chess : temp.second) {
                domain[chess.first][chess.second] = false;
            }
        }

        int startPlayer = -1, startID = -1;
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 11; ++j) {
                if (!domain[i][j]) {
                    startPlayer = i;
                    startID = j;
                    break;
                }
            }
            if (startPlayer != -1) break;
        }

        if (startID == -1) return false; // 没找到起始点

        domain[startPlayer][startID] = true;
        Queue queue;
        queue.push(map[startPlayer][startID]);

        while (!queue.isEmpty()) {
            auto node = queue.pop();
            for (const auto& near : NEAR) {
                int newRow = node[0] + near[0];
                int newCol = node[1] + near[1];

                auto it = find.find(hashPos({ newRow, newCol, 1 }));
                if (it != find.end()) {
                    for (const auto& chess : it->second) {
                        if (!domain[chess.first][chess.second]) {
                            domain[chess.first][chess.second] = true;
                            queue.push(map[chess.first][chess.second]);
                        }
                    }
                }
            }
        }

        int count = 0;
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 11; ++j) {
                if (!domain[i][j]) count++;
            }
        }
        return count == 0; // 若查重数组全为true，则表示全部连通了
    }

    //---------------------------------------------------------------------------------------------------------
    vector<vector<int>> getLegalOneSteps(int player, int id) { // 只接受两个参数
        vector<vector<int>> next;
        vector<int> pos = map[player][id];

        // 删除 find 中的记录
        int posHash = hashPos(pos);
        auto it = find.find(posHash);
        if (it != find.end()) {
            // 移除当前棋子的记录
            it->second.erase(remove_if(it->second.begin(), it->second.end(),
                [&](const pair<int, int>& p) { return p.first == player && p.second == id; }),
                it->second.end());
            if (it->second.empty()) {
                find.erase(it);
            }
        }

        // 检查连通性
        bool flag = isConnectivity();

        // 恢复 find 中的记录
        if (map[player][id] != STARTPOSITION) {
            find[posHash].emplace_back(make_pair(player, id));
        }

        if (!flag) {
            return next;
        }

        // 获取周围空白位置
        vector<vector<int>> blankPos;
        for (int i = 0; i < 6; ++i) {
            vector<int> checkPos = { pos[0] + NEAR[i][0], pos[1] + NEAR[i][1], 1 };
            int hash = hashPos(checkPos);
            if (find.find(hash) == find.end()) {
                blankPos.push_back(checkPos);
            }
        }

        // 检查左右位置的占用情况
        for (int goal = 0; goal < 6; ++goal) {
            bool flagMove = true;
            int left = (goal - 1 + 6) % 6;
            int right = (goal + 1) % 6;

            vector<int> checkPos = { pos[0] + NEAR[goal][0], pos[1] + NEAR[goal][1], 1 };
            vector<int> leftPos = { pos[0] + NEAR[left][0], pos[1] + NEAR[left][1], 1 };
            vector<int> rightPos = { pos[0] + NEAR[right][0], pos[1] + NEAR[right][1], 1 };

            bool leftOccupied = (find.find(hashPos(leftPos)) != find.end());
            bool rightOccupied = (find.find(hashPos(rightPos)) != find.end());
            bool checkOccupied = (find.find(hashPos(checkPos)) == find.end());

            if (checkOccupied && (leftOccupied || rightOccupied)) {
                next.push_back(checkPos);
            }
        }

        return next;
    }

    //-----------------------------------------------------------------------------------------------------
    // 随机 AI



    //------------------------------------------------------------------------------
    // 深拷贝方法
    Game deepCopy() const {
        Game g;
        g.P1 = this->P1;
        g.P2 = this->P2;
        g.NEAR = this->NEAR;
        g.STARTPOSITION = this->STARTPOSITION;
        g.numOfPlayers = this->numOfPlayers;
        g.map = this->map;
        g.action = this->action;
        g.round = this->round;
        g.currentPlayer = this->currentPlayer;
        g.first = this->first;
        g.drawFlag = this->drawFlag;
        g.find = this->find;
        g.angleWeight = this->angleWeight;
        g.axleWeight = this->axleWeight;
        g.kindWeight = this->kindWeight;
        return g;
    }

    //------------------------------------------------------------------------------
    // 标准化棋盘状态
    pair<vector<vector<vector<int>>>, vector<int>> normalize() {
        if (first == -1) {
            return { map, {} };
        }
        vector<vector<vector<int>>> newMap = map;
        vector<int> center = map[0][first];

        // 平移所有棋子，使得首个棋子位于原点
        for (int player = 0; player < 2; ++player) {
            for (int id = 0; id < 11; ++id) {
                if (newMap[player][id] != STARTPOSITION) {
                    newMap[player][id][0] -= center[0];
                    newMap[player][id][1] -= center[1];
                }
            }
        }

        // 旋转棋盘，使得评分最大化
        int maxScore = 0;
        vector<vector<vector<int>>> bestMap = newMap;

        for (int angle = 0; angle < 6; ++angle) { // 60度一旋转，共6次
            // 旋转
            vector<vector<vector<int>>> rotatedMap = newMap;
            for (int player = 0; player < 2; ++player) {
                for (int id = 0; id < 11; ++id) {
                    if (rotatedMap[player][id] != STARTPOSITION) {
                        int x = rotatedMap[player][id][0];
                        int y = rotatedMap[player][id][1];
                        // 旋转60度
                        rotatedMap[player][id][0] = -y;
                        rotatedMap[player][id][1] = x + y;
                    }
                }
            }

            // 计算分数
            int score = 0;
            for (int player = 0; player < 2; ++player) {
                for (int id = 0; id < 11; ++id) {
                    if (rotatedMap[player][id] != STARTPOSITION) {
                        int x = rotatedMap[player][id][0] + 30;
                        int y = rotatedMap[player][id][1] + 30;
                        if (x >= 0 && x < 60 && y >= 0 && y < 60) {
                            score += angleWeight[x][y] * kindWeight[player][id];
                        }
                    }
                }
            }

            if (score > maxScore) {
                maxScore = score;
                bestMap = rotatedMap;
            }

            newMap = rotatedMap;
        }

        // 反转棋盘轴向
        int axleMax = 0;
        for (int player = 0; player < 2; ++player) {
            for (int id = 0; id < 11; ++id) {
                if (bestMap[player][id] != STARTPOSITION) {
                    axleMax += axleWeight[bestMap[player][id][0] + 30] * kindWeight[player][id];
                }
            }
        }

        // 计算反转后的分数
        vector<vector<vector<int>>> reversedMap = bestMap;
        for (int player = 0; player < 2; ++player) {
            for (int id = 0; id < 11; ++id) {
                if (reversedMap[player][id] != STARTPOSITION) {
                    int a = reversedMap[player][id][0];
                    int b = reversedMap[player][id][1];
                    reversedMap[player][id][0] = -a;
                    reversedMap[player][id][1] = a + b;
                }
            }
        }

        // 计算反转后的分数
        int reversedScore = 0;
        for (int player = 0; player < 2; ++player) {
            for (int id = 0; id < 11; ++id) {
                if (reversedMap[player][id] != STARTPOSITION) {
                    int x = reversedMap[player][id][0] + 30;
                    int y = reversedMap[player][id][1] + 30;
                    if (x >= 0 && x < 60 && y >= 0 && y < 60) {
                        reversedScore += angleWeight[x][y] * kindWeight[player][id];
                    }
                }
            }
        }

        if (reversedScore > maxScore) {
            bestMap = reversedMap;
        }

        return { bestMap, {0, 0, -1} };
    }
};

#endif //GAME_GAME_H

//-------------------------------主程序和界面部分---------------------------------//

#ifdef GAME_IMPLEMENTATION

#include <SFML/Graphics.hpp>
#include <sstream>
#include <fstream>

using namespace sf;

// 定义棋子类型颜色
const Color PLAYER_COLORS[2] = { Color::Black, Color::White };
const Color HIGHLIGHT_COLOR = Color::Red;

// 棋盘参数
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int CELL_SIZE = 40;

// 棋子半径
const float PIECE_RADIUS = 15.0f;

// 棋子类型
enum InsectType {
    Bee = 0,
    Ant = 1,
    Spider = 2,
    Cricket = 3,
    Beetle = 4
};

// 主函数
int main() {
    // 创建游戏对象
    Game game;

    // 创建窗口
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "昆虫棋游戏");

    // 玩家设置
    string playerNames[2] = { "玩家一", "玩家二" };
    bool isAI[2] = { false, true }; // 默认玩家一是人类，玩家二是 AI

    // 字体设置
    Font font;
    if (!font.loadFromFile("Roboto-Regular.ttf")) {
        cout << "无法加载字体文件 Roboto.ttf" << endl;
        return -1;
    }

    // AI 延迟控制
    bool aiThinking = false;
    Clock aiClock;
    const float AI_DELAY = 0.5f; // AI 思考延迟 0.5 秒

    // 游戏结束标志
    bool gameEnded = false;
    string endMessage = "";

    // 游戏循环
    while (window.isOpen()) {
        // 处理事件
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            // 处理鼠标点击
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left && !gameEnded) {
                if (!isAI[game.getCurrentPlayer()]) {
                    Vector2i mousePos = Mouse::getPosition(window);
                    int x = mousePos.x / CELL_SIZE;
                    int y = mousePos.y / CELL_SIZE;

                    // 选择要移动的棋子
                    vector<int> movablePieces;
                    for (int id = 0; id < 11; ++id) {
                        auto successors = game.getSuccessor(id);
                        if (!successors.empty()) {
                            movablePieces.push_back(id);
                        }
                    }

                    if (movablePieces.empty()) {
                        game.skipRound();
                        continue;
                    }

                    int selectedId = movablePieces[0];
                    auto successors = game.getSuccessor(selectedId);

                    // 选择位置，简化为点击最近的一个可移动位置
                    // 实际应用中应根据鼠标位置选择最近的位置
                    if (!successors.empty()) {
                        // 找到与鼠标点击位置最接近的可移动位置
                        int bestIdx = 0;
                        float minDist = std::numeric_limits<float>::max();
                        for (size_t i = 0; i < successors.size(); ++i) {
                            float dx = successors[i][0] * CELL_SIZE - mousePos.x;
                            float dy = successors[i][1] * CELL_SIZE - mousePos.y;
                            float dist = sqrt(dx * dx + dy * dy);
                            if (dist < minDist) {
                                minDist = dist;
                                bestIdx = static_cast<int>(i);
                            }
                        }
                        auto move = successors[bestIdx];
                        game.moveChess(selectedId, move);

                        // 检查游戏结束
                        if (game.isEnd()) {
                            gameEnded = true;
                            if (game.isWin(0)) {
                                endMessage = "游戏结束，玩家一获胜！";
                            }
                            else if (game.isWin(1)) {
                                endMessage = "游戏结束，玩家二获胜！";
                            }
                            else {
                                endMessage = "游戏结束，平局！";
                            }
                        }
                    }
                }
            }
        }

        // AI 玩家操作
        if (!gameEnded && isAI[game.getCurrentPlayer()] && !aiThinking) {
            aiThinking = true;
            aiClock.restart();
        }

        if (aiThinking && aiClock.getElapsedTime().asSeconds() >= AI_DELAY && !gameEnded) {
            auto [id, move] = AI::getRandomMove(game);
            if (id != -1 && !move.empty()) {
                game.moveChess(id, move);
            }
            else {
                game.skipRound();
            }
            aiThinking = false;

            // 检查游戏结束
            if (game.isEnd()) {
                gameEnded = true;
                if (game.isWin(0)) {
                    endMessage = "游戏结束，玩家一获胜！";
                }
                else if (game.isWin(1)) {
                    endMessage = "游戏结束，玩家二获胜！";
                }
                else {
                    endMessage = "游戏结束，平局！";
                }
            }
        }

        // 清屏
        window.clear(Color::White);

        // 绘制棋盘（简化为网格）
        for (int i = 0; i <= WINDOW_WIDTH / CELL_SIZE; ++i) {
            Vertex line[] =
            {
                Vertex(Vector2f(i * CELL_SIZE, 0), Color::Black),
                Vertex(Vector2f(i * CELL_SIZE, WINDOW_HEIGHT), Color::Black)
            };
            window.draw(line, 2, Lines);
        }
        for (int i = 0; i <= WINDOW_HEIGHT / CELL_SIZE; ++i) {
            Vertex line[] =
            {
                Vertex(Vector2f(0, i * CELL_SIZE), Color::Black),
                Vertex(Vector2f(WINDOW_WIDTH, i * CELL_SIZE), Color::Black)
            };
            window.draw(line, 2, Lines);
        }

        // 绘制棋子
        for (int player = 0; player < 2; ++player) {
            for (int id = 0; id < 11; ++id) {
                auto pos = game.getCurrentState().first[player][id];
                if (pos[2] == -1) continue; // 未下出的棋子
                CircleShape circle(PIECE_RADIUS);
                circle.setFillColor(PLAYER_COLORS[player]);
                circle.setOutlineThickness(2);
                circle.setOutlineColor(Color::Black);
                circle.setPosition(pos[0] * CELL_SIZE - PIECE_RADIUS, pos[1] * CELL_SIZE - PIECE_RADIUS);
                window.draw(circle);
            }
        }

        // 绘制最后一步棋子高亮
        auto lastAction = game.getCurrentState().second;
        if (!lastAction.empty() && lastAction[2] != -1) {
            CircleShape highlight(PIECE_RADIUS + 5);
            highlight.setFillColor(Color::Transparent);
            highlight.setOutlineThickness(3);
            highlight.setOutlineColor(HIGHLIGHT_COLOR);
            highlight.setPosition(lastAction[0] * CELL_SIZE - PIECE_RADIUS - 5, lastAction[1] * CELL_SIZE - PIECE_RADIUS - 5);
            window.draw(highlight);
        }

        // 显示回合信息
        Text infoText;
        infoText.setFont(font);
        stringstream ss;
        ss << "当前回合: " << game.getRound() << "\n";
        ss << "当前玩家: " << (game.getCurrentPlayer() == 0 ? playerNames[0] : playerNames[1]) << "\n";
        infoText.setString(ss.str());
        infoText.setCharacterSize(20);
        infoText.setFillColor(Color::Black);
        infoText.setPosition(10, WINDOW_HEIGHT - 100);
        window.draw(infoText);

        // 显示游戏结束信息
        if (gameEnded) {
            Text endText;
            endText.setFont(font);
            endText.setString(endMessage);
            endText.setCharacterSize(30);
            endText.setFillColor(Color::Red);
            endText.setPosition(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 15);
            window.draw(endText);
        }

        // 显示内容
        window.display();
    }

#endif // GAME_IMPLEMENTATION
