#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

struct RaceType {
    int64_t time;
    int64_t record;
};

int64_t race_length(int64_t time, int64_t hold) {
    const int64_t run = time - hold;
    return hold * run;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scanner{argv[1]};
    std::vector<RaceType> competition{};

    while (!scanner.isEof()) {
        scanner.skipToken("Time:");
        while (!scanner.isEof() and std::isdigit(scanner.peekChar())) {
            competition.emplace_back(scanner.getInt64());
        }
        scanner.skipToken("Distance:");
        int id = 0;
        while (!scanner.isEof() and std::isdigit(scanner.peekChar())) {
            competition[id].record = scanner.getInt64();
            ++id;
        }
    }

    int64_t prod1 = 1;
    for (const auto &race : competition) {
        int64_t won = 0;
        for (const auto hold : iota(1, race.time)) {
            if (race_length(race.time, hold) > race.record) {
                ++won;
            }
        }
        prod1 *= won;
    }

    fmt::print("Answer: {} [Submit]\n", prod1);
}
