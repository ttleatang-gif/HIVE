#ifndef GAME_AI_H
#define GAME_AI_H

// Make sure game.h is included first
#include "game.h"
#include <random>
#include <utility>
#include <vector>

namespace hive {  //

    class AI {
    public:
        //
        static std::pair<int, std::vector<int>> getRandomMove(Game& game) {
            std::vector<std::pair<int, std::vector<std::vector<int>>>> allMoves;

            //
            for (int id = 0; id < 11; ++id) {
                std::vector<std::vector<int>> successors = game.getSuccessor(id);  //
                if (!successors.empty()) {
                    allMoves.emplace_back(std::make_pair(id, successors));
                }
            }

            if (allMoves.empty()) {
                //
                return std::make_pair(-1, std::vector<int>());
            }

            //
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, static_cast<int>(allMoves.size()) - 1);

            int idx = dis(gen);
            int id = allMoves[idx].first;
            const auto& moves = allMoves[idx].second;

            std::uniform_int_distribution<> moveDis(0, static_cast<int>(moves.size()) - 1);
            int moveIdx = moveDis(gen);

            return std::make_pair(id, moves[moveIdx]);
        }
    };

} // namespace hive

#endif //GAME_AI_H