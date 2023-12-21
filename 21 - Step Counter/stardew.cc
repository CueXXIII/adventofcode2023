#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <ranges>
#include <string>
#include <vector>

#include "grid.hpp"
#include "vec2.hpp"

using std::views::iota;
using std::views::reverse;

struct Garden {
    Grid<char> garden{};
    Grid<int64_t> distances{};
    Vec2l start{};

    Garden(const char *file) : garden(file, '#'), distances(garden.width, garden.height, -1) {
        for (const auto y : iota(0, garden.height)) {
            for (const auto x : iota(0, garden.width)) {
                if (garden[x, y] == 'S') {
                    start = {x, y};
                }
            }
        }
        calcDist();
    }

    void calcDist() {
        std::queue<std::pair<Vec2l, int64_t>> frontier{};
        frontier.emplace(start, 0);
        distances[start] = 0;

        while (!frontier.empty()) {
            const auto [position, distance] = frontier.front();
            frontier.pop();
            for (const auto direction : neighbours4) {
                const auto destination = position + direction;
                if (garden[destination] == '.' and distances[destination] == -1) {
                    frontier.emplace(destination, distance + 1);
                    distances[destination] = distance + 1;
                }
            }
        }
    }

    int64_t findSteps(const int steps = 64) {
        int64_t count = 0;
        for (const auto tile : distances.data) {
            if (tile <= steps and ((tile ^ steps) & 1) == 0) {
                count++;
            }
        }
        return count;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Garden garden{argv[1]};
    fmt::print("After 6 steps you reach {} tiles.\n", garden.findSteps(6));
    fmt::print("After 64 steps you reach {} tiles.\n", garden.findSteps(64));
}
