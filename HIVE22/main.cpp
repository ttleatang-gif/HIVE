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
const float HEX_SIZE = 40.0f;  // 六边形的大小
const float HEX_HEIGHT = HEX_SIZE * 2;
const float HEX_WIDTH = HEX_SIZE * std::sqrt(3);
const sf::Vector2f BOARD_OFFSET(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

// 棋子颜色
const sf::Color PLAYER1_COLOR = sf::Color::Black;
const sf::Color PLAYER2_COLOR = sf::Color::White;
const sf::Color SELECTED_COLOR = sf::Color(255, 255, 0, 128);
const sf::Color VALID_MOVE_COLOR = sf::Color(0, 255, 0, 128);

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

int main() {
    // 创建游戏窗口
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "蜂巢棋");
    window.setFramerateLimit(60);

    // 创建游戏实例
    Game game;

    // 创建基础六边形形状
    sf::ConvexShape hexagon = createHexagon(HEX_SIZE);

    // 游戏状态变量
    int selectedPiece = -1;
    std::vector<std::vector<int>> validMoves;

    // 游戏主循环
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // 处理鼠标点击
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    std::vector<int> gamePos = screenToGame(sf::Vector2f(static_cast<float>(mousePos.x),
                        static_cast<float>(mousePos.y)));

                    // 如果没有选中棋子，尝试选择一个
                    if (selectedPiece == -1) {
                        // 遍历当前玩家的所有棋子，查找是否有可移动的棋子
                        for (int id = 0; id < 11; id++) {
                            auto moves = game.getSuccessor(id);
                            if (!moves.empty()) {
                                selectedPiece = id;
                                validMoves = moves;
                                break;
                            }
                        }
                    }
                    // 如果已选中棋子，尝试移动
                    else {
                        // 检查是否是有效移动
                        bool validMove = false;
                        for (const auto& move : validMoves) {
                            if (move == gamePos) {
                                validMove = true;
                                break;
                            }
                        }

                        if (validMove) {
                            game.moveChess(selectedPiece, gamePos);
                            selectedPiece = -1;
                            validMoves.clear();
                        }
                    }
                }
            }
        }

        // 清空窗口
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

        // 绘制有效移动位置提示
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
                if (map[player][id][2] != -1) {  // 如果棋子在棋盘上
                    sf::Vector2f pos = gameToScreen(map[player][id]);

                    // 创建棋子形状
                    sf::CircleShape piece(HEX_SIZE * 0.8f);
                    piece.setPosition(pos.x - piece.getRadius(), pos.y - piece.getRadius());
                    piece.setFillColor(player == 0 ? PLAYER1_COLOR : PLAYER2_COLOR);
                    piece.setOutlineThickness(2);
                    piece.setOutlineColor(sf::Color::Yellow);

                    // 如果是选中的棋子，添加高亮效果
                    if (id == selectedPiece && player == game.getCurrentPlayer()) {
                        piece.setOutlineColor(SELECTED_COLOR);
                        piece.setOutlineThickness(4);
                    }

                    window.draw(piece);
                }
            }
        }

        // 显示窗口内容
        window.display();

        // 检查游戏是否结束
        if (game.isEnd()) {
            if (game.isDraw()) {
                std::cout << "游戏平局！" << std::endl;
            }
            else {
                std::cout << "玩家" << (game.getCurrentPlayer() == 0 ? "白棋" : "黑棋") << "获胜！" << std::endl;
            }
            break;
        }
    }

    return 0;
}