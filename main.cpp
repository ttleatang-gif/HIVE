#include "Game.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 窗口和棋盘参数
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
const float HEX_SIZE = 40.0f;
const float HEX_HEIGHT = HEX_SIZE * 2;
const float HEX_WIDTH = HEX_SIZE * std::sqrt(3);
const sf::Vector2f BOARD_OFFSET(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

// 棋子列表参数
const float PIECE_LIST_X = WINDOW_WIDTH - 200;
const std::wstring PIECE_NAMES[] = {
    L"蜂后", L"工蚁1", L"工蚁2", L"工蚁3",
    L"蜘蛛1", L"蜘蛛2",
    L"蟋蟀1", L"蟋蟀2", L"蟋蟀3",
    L"甲虫1", L"甲虫2"
};

// 棋子颜色
const sf::Color PLAYER1_COLOR = sf::Color::Black;
const sf::Color PLAYER2_COLOR = sf::Color::White;
const sf::Color SELECTED_COLOR = sf::Color(255, 255, 0, 128);
const sf::Color VALID_MOVE_COLOR = sf::Color(0, 255, 0, 128);
const sf::Color SYMBOL_COLOR = sf::Color::Cyan;  // 棋子标识符的颜色

// 创建六边形形状
sf::ConvexShape createHexagon(float size) {
    sf::ConvexShape hexagon;
    hexagon.setPointCount(6);

    for (int i = 0; i < 6; i++) {
        float angle = i * 60.0f * static_cast<float>(M_PI) / 180.0f;
        float x = size * std::cos(angle);
        float y = size * std::sin(angle);
        hexagon.setPoint(i, sf::Vector2f(x, y));
    }

    return hexagon;
}

// 坐标转换：游戏坐标到屏幕坐标
sf::Vector2f gameToScreen(const std::vector<int>& pos) {
    float x = pos[0] * HEX_WIDTH * 0.75f;
    float y = pos[1] * HEX_HEIGHT + (pos[0] % 2) * HEX_HEIGHT / 2;
    return sf::Vector2f(x + BOARD_OFFSET.x, y + BOARD_OFFSET.y);
}

// 坐标转换：屏幕坐标到游戏坐标
std::vector<int> screenToGame(const sf::Vector2f& screenPos) {
    float relX = screenPos.x - BOARD_OFFSET.x;
    float relY = screenPos.y - BOARD_OFFSET.y;

    int col = static_cast<int>(std::round(relX / (HEX_WIDTH * 0.75f)));
    int row = static_cast<int>(std::round((relY - (col % 2) * HEX_HEIGHT / 2) / HEX_HEIGHT));

    return { col, row, 1 };
}

// 绘制棋子标识
void drawPieceSymbol(sf::RenderWindow& window, int pieceType, sf::Vector2f pos, int number = 0) {
    const float symbolSize = HEX_SIZE * 0.3f;

    switch (pieceType) {
    case 0: { // 蜂后 - 皇冠形状
        sf::ConvexShape crown;
        crown.setPointCount(5);
        crown.setPoint(0, sf::Vector2f(pos.x - symbolSize, pos.y));
        crown.setPoint(1, sf::Vector2f(pos.x - symbolSize / 2, pos.y - symbolSize));
        crown.setPoint(2, sf::Vector2f(pos.x, pos.y));
        crown.setPoint(3, sf::Vector2f(pos.x + symbolSize / 2, pos.y - symbolSize));
        crown.setPoint(4, sf::Vector2f(pos.x + symbolSize, pos.y));
        crown.setFillColor(sf::Color::Transparent);
        crown.setOutlineColor(SYMBOL_COLOR);
        crown.setOutlineThickness(2.0f);
        window.draw(crown);
        break;
    }
    case 1: case 2: case 3: { // 工蚁 - 圆点加数字
        sf::CircleShape dot(symbolSize * 0.5f);
        dot.setPosition(pos.x - symbolSize * 0.5f, pos.y - symbolSize * 0.5f);
        dot.setFillColor(SYMBOL_COLOR);
        window.draw(dot);

        // 添加数字标识
        sf::Text numText;
        numText.setString(std::to_string(pieceType));
        numText.setCharacterSize(static_cast<unsigned int>(symbolSize * 1.5f));
        numText.setFillColor(SYMBOL_COLOR);
        numText.setPosition(pos.x + symbolSize, pos.y - symbolSize);
        window.draw(numText);
        break;
    }
    case 4: case 5: { // 蜘蛛 - 交叉线
        sf::RectangleShape line1(sf::Vector2f(symbolSize * 2, 2.0f));
        sf::RectangleShape line2(sf::Vector2f(symbolSize * 2, 2.0f));
        line1.setOrigin(symbolSize, 1.0f);
        line2.setOrigin(symbolSize, 1.0f);
        line1.setPosition(pos);
        line2.setPosition(pos);
        line1.setRotation(45);
        line2.setRotation(-45);
        line1.setFillColor(SYMBOL_COLOR);
        line2.setFillColor(SYMBOL_COLOR);
        window.draw(line1);
        window.draw(line2);

        // 添加数字标识
        if (pieceType == 5) {
            sf::Text numText;
            numText.setString("2");
            numText.setCharacterSize(static_cast<unsigned int>(symbolSize * 1.5f));
            numText.setFillColor(SYMBOL_COLOR);
            numText.setPosition(pos.x + symbolSize, pos.y - symbolSize);
            window.draw(numText);
        }
        break;
    }
    case 6: case 7: case 8: { // 蟋蟀 - 三角形
        sf::ConvexShape triangle;
        triangle.setPointCount(3);
        triangle.setPoint(0, sf::Vector2f(pos.x, pos.y - symbolSize));
        triangle.setPoint(1, sf::Vector2f(pos.x - symbolSize, pos.y + symbolSize));
        triangle.setPoint(2, sf::Vector2f(pos.x + symbolSize, pos.y + symbolSize));
        triangle.setFillColor(sf::Color::Transparent);
        triangle.setOutlineColor(SYMBOL_COLOR);
        triangle.setOutlineThickness(2.0f);
        window.draw(triangle);

        // 添加数字标识
        int number = pieceType - 5;
        if (number > 1) {
            sf::Text numText;
            numText.setString(std::to_string(number));
            numText.setCharacterSize(static_cast<unsigned int>(symbolSize * 1.5f));
            numText.setFillColor(SYMBOL_COLOR);
            numText.setPosition(pos.x + symbolSize, pos.y - symbolSize);
            window.draw(numText);
        }
        break;
    }
    case 9: case 10: { // 甲虫 - 方块
        sf::RectangleShape square(sf::Vector2f(symbolSize * 1.5f, symbolSize * 1.5f));
        square.setOrigin(symbolSize * 0.75f, symbolSize * 0.75f);
        square.setPosition(pos);
        square.setFillColor(sf::Color::Transparent);
        square.setOutlineColor(SYMBOL_COLOR);
        square.setOutlineThickness(2.0f);
        window.draw(square);

        // 添加数字标识
        if (pieceType == 10) {
            sf::Text numText;
            numText.setString("2");
            numText.setCharacterSize(static_cast<unsigned int>(symbolSize * 1.5f));
            numText.setFillColor(SYMBOL_COLOR);
            numText.setPosition(pos.x + symbolSize, pos.y - symbolSize);
            window.draw(numText);
        }
        break;
    }
    }
}
int main() {
    // 创建游戏窗口
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), L"蜂巢棋");
    window.setFramerateLimit(60);

    // 创建游戏实例
    Game game;

    // 创建基础六边形形状
    sf::ConvexShape hexagon = createHexagon(HEX_SIZE);

    // 加载字体
    sf::Font font;
    bool fontLoaded = false;

    std::vector<std::wstring> fontPaths = {
        L"simhei.ttf",
        L"C:\\Windows\\Fonts\\simhei.ttf",
        L"C:\\Windows\\Fonts\\msyh.ttc",
        L"C:\\Windows\\Fonts\\simsun.ttc",
        L"C:\\Windows\\Fonts\\simfang.ttf",
        L"C:\\Windows\\Fonts\\simkai.ttf",
        L"msyh.ttf",
        L"simsun.ttc"
    };

    for (const auto& path : fontPaths) {
        if (font.loadFromFile(std::string(path.begin(), path.end()))) {
            fontLoaded = true;
            std::wcout << L"成功加载字体: " << path << std::endl;
            break;
        }
    }

    if (!fontLoaded) {
        std::wcout << L"错误：无法加载任何中文字体!" << std::endl;
        return -1;
    }

    // 游戏状态变量
    int selectedPiece = -1;
    bool isPlacingNewPiece = false;
    std::vector<std::vector<int>> validMoves;

    // 主循环
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    // 检查右侧未使用的棋子列表
                    if (mousePos.x > PIECE_LIST_X) {
                        int index = (mousePos.y - 50) / 40;
                        if (index >= 0 && index < 11) {
                            auto state = game.getCurrentState();
                            if (state.first[game.getCurrentPlayer()][index][2] == -1) {
                                selectedPiece = index;
                                isPlacingNewPiece = true;
                                validMoves = game.getSuccessor(index);
                            }
                        }
                    }
                    // 点击棋盘区域
                    else {
                        std::vector<int> gamePos = screenToGame(sf::Vector2f(
                            static_cast<float>(mousePos.x),
                            static_cast<float>(mousePos.y)));

                        if (selectedPiece == -1) {
                            // 选择棋盘上的棋子
                            auto [map, action] = game.getCurrentState();
                            float minDist = HEX_SIZE;

                            for (int id = 0; id < 11; id++) {
                                if (map[game.getCurrentPlayer()][id][2] != -1) {
                                    sf::Vector2f piecePos = gameToScreen(map[game.getCurrentPlayer()][id]);
                                    float dx = piecePos.x - mousePos.x;
                                    float dy = piecePos.y - mousePos.y;
                                    float dist = std::sqrt(dx * dx + dy * dy);

                                    if (dist < minDist) {
                                        auto moves = game.getSuccessor(id);
                                        if (!moves.empty()) {
                                            selectedPiece = id;
                                            isPlacingNewPiece = false;
                                            validMoves = moves;
                                            minDist = dist;
                                        }
                                    }
                                }
                            }
                        }
                        else {
                            // 执行移动
                            for (const auto& move : validMoves) {
                                if (move[0] == gamePos[0] && move[1] == gamePos[1]) {
                                    game.moveChess(selectedPiece, move);
                                    selectedPiece = -1;
                                    isPlacingNewPiece = false;
                                    validMoves.clear();
                                    break;
                                }
                            }
                        }
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    // 右键取消选择
                    selectedPiece = -1;
                    isPlacingNewPiece = false;
                    validMoves.clear();
                }
            }
        }

        // 渲染
        window.clear(sf::Color(200, 200, 200));  // 浅灰色背景

        // 绘制棋盘网格
        for (int x = -10; x <= 10; x++) {
            for (int y = -10; y <= 10; y++) {
                sf::Vector2f pos = gameToScreen({ x, y, 1 });
                hexagon.setPosition(pos);
                hexagon.setFillColor(sf::Color(230, 230, 230));
                hexagon.setOutlineThickness(1);
                hexagon.setOutlineColor(sf::Color(180, 180, 180));
                window.draw(hexagon);
            }
        }

        // 绘制有效移动提示
        for (const auto& move : validMoves) {
            sf::Vector2f pos = gameToScreen(move);
            hexagon.setPosition(pos);
            hexagon.setFillColor(VALID_MOVE_COLOR);
            window.draw(hexagon);
        }

        // 绘制棋子
        auto [map, action] = game.getCurrentState();
        for (int player = 0; player < 2; player++) {
            for (int id = 0; id < 11; id++) {
                if (map[player][id][2] != -1) {  // 棋子在棋盘上
                    sf::Vector2f pos = gameToScreen(map[player][id]);

                    // 绘制基础棋子
                    sf::CircleShape piece(HEX_SIZE * 0.8f);
                    piece.setPosition(pos.x - piece.getRadius(), pos.y - piece.getRadius());
                    piece.setFillColor(player == 0 ? PLAYER1_COLOR : PLAYER2_COLOR);
                    piece.setOutlineThickness(2);
                    piece.setOutlineColor(sf::Color::Cyan);

                    if (id == selectedPiece && player == game.getCurrentPlayer()) {
                        piece.setOutlineColor(SELECTED_COLOR);
                        piece.setOutlineThickness(4);
                    }

                    window.draw(piece);

                    // 绘制棋子标识
                    drawPieceSymbol(window, id, pos);
                }
            }
        }

        // 绘制右侧未使用的棋子列表
        for (int id = 0; id < 11; id++) {
            if (map[game.getCurrentPlayer()][id][2] == -1) {  // 未使用的棋子
                sf::CircleShape piece(15);
                piece.setPosition(PIECE_LIST_X, 50 + id * 40);
                piece.setFillColor(game.getCurrentPlayer() == 0 ? PLAYER1_COLOR : PLAYER2_COLOR);
                piece.setOutlineThickness(2);
                piece.setOutlineColor(sf::Color::Cyan);

                if (id == selectedPiece) {
                    piece.setOutlineColor(SELECTED_COLOR);
                    piece.setOutlineThickness(4);
                }

                window.draw(piece);

                // 绘制侧边栏棋子的标识
                drawPieceSymbol(window, id,
                    sf::Vector2f(PIECE_LIST_X + 15, 50 + id * 40 + 15));

                // 绘制棋子名称
                sf::Text text;
                text.setFont(font);
                text.setString(sf::String(PIECE_NAMES[id]));
                text.setCharacterSize(20);
                text.setFillColor(sf::Color::Black);
                text.setPosition(PIECE_LIST_X + 40, 45 + id * 40);
                window.draw(text);
            }
        }

        // 显示当前玩家
        sf::Text playerText;
        playerText.setFont(font);
        std::wstring playerStr = L"当前玩家: " + std::wstring(game.getCurrentPlayer() == 0 ? L"黑棋" : L"白棋");
        playerText.setString(sf::String(playerStr));
        playerText.setCharacterSize(24);
        playerText.setFillColor(sf::Color::Black);
        playerText.setPosition(PIECE_LIST_X, 10);
        window.draw(playerText);

        // 显示回合数
        sf::Text roundText;
        roundText.setFont(font);
        std::wstring roundStr = L"回合: " + std::to_wstring(game.getRound());
        roundText.setString(sf::String(roundStr));
        roundText.setCharacterSize(24);
        roundText.setFillColor(sf::Color::Black);
        roundText.setPosition(10, 10);
        window.draw(roundText);

        // 检查游戏是否结束并显示结果
        if (game.isEnd()) {
            std::wstring resultStr;
            if (game.isDraw()) {
                resultStr = L"游戏平局！";
            }
            else {
                resultStr = L"玩家" + std::wstring(game.isWin(0) ? L"黑棋" : L"白棋") + L"获胜！";
            }

            // 显示结果文本
            sf::Text resultText;
            resultText.setFont(font);
            resultText.setString(sf::String(resultStr));
            resultText.setCharacterSize(48);
            resultText.setFillColor(sf::Color::Red);

            // 居中显示结果
            sf::FloatRect textBounds = resultText.getLocalBounds();
            resultText.setPosition(
                (WINDOW_WIDTH - textBounds.width) / 2,
                (WINDOW_HEIGHT - textBounds.height) / 2
            );

            window.clear(sf::Color(200, 200, 200));
            window.draw(resultText);
            window.display();

            sf::sleep(sf::seconds(3));
            break;
        }

        window.display();
    }

    return 0;
}