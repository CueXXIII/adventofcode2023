#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "grid.hpp"
#include "vec2.hpp"

using std::views::iota;

struct Universe {
    Grid<char> observed{};
    std::vector<bool> emptyX{};
    std::vector<bool> emptyY{};
    std::vector<Vec2l> galaxies{};

    int64_t realX(int64_t x1, int64_t x2) const {
        int64_t add = 0;
        if (x1 > x2) {
            std::swap(x1, x2);
        }
        for (const auto x : iota(x1, x2)) {
            if (emptyX[x]) {
                ++add;
            }
        }
        return x2 - x1 + add;
    }

    int64_t realY(int64_t y1, int64_t y2) const {
        int64_t add = 0;
        if (y1 > y2) {
            std::swap(y1, y2);
        }
        for (const auto y : iota(y1, y2)) {
            if (emptyY[y]) {
                ++add;
            }
        }
        return y2 - y1 + add;
    }

    Universe(const char *file) : observed(file) {
        emptyX.resize(observed.width, true);
        emptyY.resize(observed.height, true);
        for (const auto y : iota(0, observed.height)) {
            for (const auto x : iota(0, observed.width)) {
                if (observed[x, y] == '#') {
                    emptyX[x] = false;
                    emptyY[y] = false;
                    galaxies.emplace_back(x, y);
                }
            }
        }
    }

    // returns sum of paths
    int64_t findPaths() const {
        int64_t sum = 0;

        for (const auto from : iota(0u, galaxies.size())) {
            for (const auto to : iota(from + 1, galaxies.size())) {

                const auto lenX = realX(galaxies[from].x, galaxies[to].x);
                const auto lenY = realY(galaxies[from].y, galaxies[to].y);
                sum += lenX + lenY;
                // fmt::print("Between galaxy {} and {}: {}\n", from + 1, to + 1, lenX + lenY);
            }
        }
        return sum;
    }

    void printObserved() const {
        fmt::print("Empty Columns: ");
        for (const auto x : iota(0, observed.width)) {
            if (emptyX[x]) {
                fmt::print("{}, ", x);
            }
        }
        fmt::print("\n");

        fmt::print("Empty Rows: ");
        for (const auto y : iota(0, observed.height)) {
            if (emptyY[y]) {
                fmt::print("{}, ", y);
            }
        }
        fmt::print("\n");

        fmt::print("{} galaxies\n", galaxies.size());
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Universe universe{argv[1]};
    universe.printObserved();
    fmt::print("The sum of all paths is {}\n", universe.findPaths());
}
