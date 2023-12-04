#include <fmt/format.h>
#include <iostream>
#include <map>
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
    std::map<int64_t, int64_t> cardCount{};
    int64_t totalScore = 0;
    int64_t totalCards = 0;
    while (!scanner.isEof()) {
        scanner.skipToken("Card");
        const auto card = scanner.getInt64();
        ++cardCount[card];
        scanner.skipChar(':');
        std::set<int64_t> winning{};
        while (!scanner.skipChar('|')) {
            winning.insert(scanner.getInt64());
        }
        int64_t wins = 0;
        while (!scanner.isEof() and scanner.peekChar() != 'C') {
            const auto number = scanner.getInt64();
            if (winning.contains(number)) {
                ++wins;
            }
        }
        const auto points = wins > 0 ? 1 << (wins - 1) : 0;
        totalScore += points;

        for (const auto id : iota(card + 1, card + wins + 1)) {
            cardCount[id] += cardCount[card];
        }
        totalCards += cardCount[card];
    }
    fmt::print("You have {} points\n", totalScore);
    fmt::print("You have {} cards\n", totalCards);
}
