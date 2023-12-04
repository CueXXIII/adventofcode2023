#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <queue>
#include <ranges>
#include <set>
#include <string>

#include "simpleparser.hpp"

using std::views::iota;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scanner{argv[1]};
    std::priority_queue<std::pair<int64_t, int64_t>> wonCards{};
    std::set<int64_t> winningNumbers{};
    int64_t totalScore = 0;
    int64_t totalCards = 0;
    while (!scanner.isEof()) {
        scanner.skipToken("Card");
        const auto cardId = scanner.getInt64();
        int64_t cardCount = 1;
        while (!wonCards.empty() and wonCards.top().first == -cardId) {
            cardCount += wonCards.top().second;
            wonCards.pop();
        }
        scanner.skipChar(':');
        winningNumbers.clear();
        while (!scanner.skipChar('|')) {
            winningNumbers.insert(scanner.getInt64());
        }
        int64_t wins = 0;
        while (!scanner.isEof() and scanner.peekChar() != 'C') {
            const auto number = scanner.getInt64();
            if (winningNumbers.contains(number)) {
                ++wins;
            }
        }
        const auto points = wins > 0 ? 1 << (wins - 1) : 0;
        totalScore += points;

        for (const auto wonId : iota(cardId + 1, cardId + wins + 1)) {
            wonCards.emplace(-wonId, cardCount);
        }
        totalCards += cardCount;
    }
    fmt::print("You have {} points\n", totalScore);
    fmt::print("You have {} cards\n", totalCards);
}
