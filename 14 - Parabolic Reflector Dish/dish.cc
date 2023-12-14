#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "grid.hpp"

using std::views::iota;

void tiltNorth(auto &platform) {
    for (const auto x : iota(0, platform.width)) {
        for (const auto y : iota(1, platform.height)) {
            for (const auto o : iota(0, y)) {
                if (platform[x, y - o] == 'O' and platform[x, y - o - 1] == '.') {
                    platform[x, y - o - 1] = 'O';
                    platform[x, y - o] = '.';
                } else {
                    break;
                }
            }
        }
    }
}

int64_t getWeight(auto &platform) {
    int64_t sum = 0;
    for (const auto x : iota(0, platform.width)) {
        for (const auto y : iota(0, platform.height)) {
            if (platform[x, y] == 'O') {
                sum += platform.height - y;
            }
        }
    }
    return sum;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Grid<char> platform{argv[1]};

    tiltNorth(platform);
    platform.print();
    fmt::print("The platform needs to support {} metric O\n", getWeight(platform));
}
