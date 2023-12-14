#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>
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

void tiltWest(auto &platform) {
    for (const auto y : iota(0, platform.height)) {
        for (const auto x : iota(1, platform.width)) {
            for (const auto o : iota(0, x)) {
                if (platform[x - o, y] == 'O' and platform[x - o - 1, y] == '.') {
                    platform[x - o - 1, y] = 'O';
                    platform[x - o, y] = '.';
                } else {
                    break;
                }
            }
        }
    }
}

void tiltSouth(auto &platform) {
    for (const auto x : iota(0, platform.width)) {
        for (const auto y : iota(1, platform.height)) {
            for (const auto o : iota(0, y)) {
                if (platform[x, platform.height - 1 - (y - o)] == 'O' and
                    platform[x, platform.height - 1 - (y - o - 1)] == '.') {
                    platform[x, platform.height - 1 - (y - o - 1)] = 'O';
                    platform[x, platform.height - 1 - (y - o)] = '.';
                } else {
                    break;
                }
            }
        }
    }
}

void tiltEast(auto &platform) {
    for (const auto y : iota(0, platform.height)) {
        for (const auto x : iota(1, platform.width)) {
            for (const auto o : iota(0, x)) {
                if (platform[platform.width - 1 - (x - o), y] == 'O' and
                    platform[platform.width - 1 - (x - o - 1), y] == '.') {
                    platform[platform.width - 1 - (x - o - 1), y] = 'O';
                    platform[platform.width - 1 - (x - o), y] = '.';
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

std::string getString(auto &platform) {
    std::string result{};
    for (const auto x : iota(0, platform.width)) {
        for (const auto y : iota(0, platform.height)) {
            result += platform[x, y];
        }
    }
    return result;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Grid<char> platform{argv[1]};

    std::unordered_map<std::string, int64_t> cycle{};
    std::unordered_map<int64_t, int64_t> weights{};

    tiltNorth(platform);
    platform.print();
    fmt::print("The platform needs to support {} metric O\n", getWeight(platform));

    // complete cycle 0
    tiltWest(platform);
    tiltSouth(platform);
    tiltEast(platform);
    cycle[getString(platform)] = 1;
    weights[1] = getWeight(platform);

    const int64_t cyclesTotal = 1000000000;
    for ([[maybe_unused]] const auto n : iota(2, cyclesTotal + 1)) {
        tiltNorth(platform);
        tiltWest(platform);
        tiltSouth(platform);
        tiltEast(platform);
        const auto platStr = getString(platform);
        if (cycle.contains(platStr)) {
            const auto cStart = cycle[platStr];
            const auto cLen = n - cStart;
            const auto cLeft = (cyclesTotal - cStart) % cLen;
            fmt::print("Detected cycle from {} to {}\n", cStart, n);
            // platform.print();
            fmt::print("After {} cycles there are {} metric O\n", cyclesTotal,
                       weights[cStart + cLeft]);
            break;
        } else {
            cycle[platStr] = n;
            weights[n] = getWeight(platform);
        }
    }
}
