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

int64_t raceLength(const int64_t time, const int64_t hold) {
    const int64_t run = time - hold;
    return hold * run;
    // return -1 * hold * hold + time * hold;
}

int64_t raceDeriv(const int64_t time, const int64_t hold) { return -2 * hold + time; }

int64_t findFirstWin(const RaceType &race) {
    int64_t hold_n = 0;
    int64_t length_n = raceLength(race.time, hold_n);
    // fmt::print("x_0: f({}) = {}\n", hold_n, length_n);
    while (true) {
        const int64_t hold_n1 = hold_n - (length_n - race.record) / raceDeriv(race.time, hold_n);
        if (hold_n == hold_n1) {
            while (length_n <= race.record) {
                ++hold_n;
                length_n = raceLength(race.time, hold_n);
                // fmt::print("inc: f({}) = {}\n", hold_n, length_n);
            }
            return hold_n;
        }
        hold_n = hold_n1;
        length_n = raceLength(race.time, hold_n);
        // fmt::print("x_n: f({}) = {}\n", hold_n, length_n);
    }
    // this is currently not reached, races that can't be won don't finish
    fmt::print("Error: no win in {}ms\n", race.time);
    return race.time;
}

int64_t findWinningGames(const RaceType &race) {
    const auto first = findFirstWin(race);
    const auto last = race.time - first;
    return last - first + 1;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scanner{argv[1]};
    std::vector<RaceType> competition{};

    scanner.skipToken("Time:");
    std::string time2{};
    while (!scanner.isEof() and std::isdigit(scanner.peekChar())) {
        const auto time = scanner.getInt64();
        competition.emplace_back(time);
        time2 += fmt::format("{}", time);
    }
    scanner.skipToken("Distance:");
    std::string record2{};
    int id = 0;
    while (!scanner.isEof() and std::isdigit(scanner.peekChar())) {
        const auto record = scanner.getInt64();
        competition[id].record = record;
        record2 += fmt::format("{}", record);
        ++id;
    }
    RaceType competition2{std::stoll(time2), std::stoll(record2)};

    int64_t prod1 = 1;
    for (const auto &race : competition) {
        prod1 *= findWinningGames(race);
    }
    fmt::print("Answer: {} [Submit]\n", prod1);

    fmt::print("Answer: {} [Submit]\n", findWinningGames(competition2));
}
