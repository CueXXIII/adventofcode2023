#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <set>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scanner{argv[1]};
    int64_t totalScore = 0;
    while (!scanner.isEof()) {
        scanner.skipToken("Card");
        const auto card = scanner.getInt64();
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
        fmt::print("Card {} scores {} points\n", card, points);
    }
    fmt::print("You have {} points\n", totalScore);
}
