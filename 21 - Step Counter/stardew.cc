#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "grid.hpp"
#include "utility.hpp"
#include "vec2.hpp"

using std::views::iota;
using std::views::reverse;

constexpr int64_t modulus(const int64_t divident, const int64_t divisor) {
    return (divident % divisor + divisor) % divisor;
}

constexpr int64_t floordiv(const int64_t divident, const int64_t divisor) {
    if (divident >= 0) {
        return divident / divisor;
    }
    return (divident + 1) / divisor - 1;
}

struct Garden {
    Grid<char> garden{};
    Grid<int64_t> distances{};
    std::unordered_map<Vec2l, int64_t> distancesMod{};
    Vec2l start{};
    std::vector<int64_t> compassSteps{};

    //                 +---+
    //                 | N |
    //             +---+---+---+
    //             | NW| N | NE|
    //         +---+---+---+---+---+
    //         | NW|   |   |   | NE|
    //     +---+---+---+---+---+---+---+
    //     | NW|   | NW|   | NE|   | NE|
    // +---+---+---+---+===+---+---+---+---+
    // | W | W |   |   |###|   |   | E | E |
    // +---+---+---+---+===+---+---+---+---+
    //     | SW|   | SW|   | SE|   | SE|
    //     +---+---+---+---+---+---+---+
    //         | SW|   |   |   | SE|
    //         +---+---+---+---+---+
    //             | SW| S | SE|
    //             +---+---+---+
    //                 | S |
    //                 +---+
    // example needs +2/+3 in W/E, input only +1
    // clang-format off
    static constexpr std::array<std::tuple<const char *, Vec2l, Vec2l>, 16> repeats = {{
            {"E",   { 3,  0}, { 4,  0}},
            {"ENE", { 1, -1}, { 3, -1}},
            {"NE",  { 1, -1}, { 2, -2}},
            {"NNE", { 1, -1}, { 1, -3}},

            {"N",   { 0, -3}, { 0, -4}},
            {"NNW", {-1, -1}, {-1, -3}},
            {"NW",  {-1, -1}, {-2, -2}},
            {"WNW", {-1, -1}, {-3, -1}},

            {"W",   {-3,  0}, {-4,  0}},
            {"WSW", {-1,  1}, {-3,  1}},
            {"SW",  {-1,  1}, {-2,  2}},
            {"SSW", {-1,  1}, {-1,  3}},

            {"S",   { 0,  3}, { 0,  4}},
            {"SSE", { 1,  1}, { 1,  3}},
            {"SE",  { 1,  1}, { 2,  2}},
            {"ESE", { 1,  1}, { 3,  1}}
        }};
    // clang-format on

    Garden(const char *file) : garden(file, '#'), distances(garden.width, garden.height, -1) {
        for (const auto y : iota(0, garden.height)) {
            for (const auto x : iota(0, garden.width)) {
                if (garden[x, y] == 'S') {
                    start = {x, y};
                }
            }
        }
        calcDist();
        calcModDist();
        // checkLoops();
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

    void calcModDist() {
        std::queue<std::pair<Vec2l, int64_t>> frontier{};
        frontier.emplace(start, 0);
        distancesMod[start] = 0;

        while (!frontier.empty()) {
            const auto [position, distance] = frontier.front();
            frontier.pop();

            // precalculate only a few gardens (3* dist)
            if (distance > 3 * (garden.width + garden.height)) {
                return;
            }

            for (const auto direction : neighbours4) {
                const auto destination = position + direction;
                const auto destinationMod = Vec2l{modulus(destination.x, garden.width),
                                                  modulus(destination.y, garden.height)};
                if (garden[destinationMod] != '#' and !distancesMod.contains(destination)) {
                    frontier.emplace(destination, distance + 1);
                    distancesMod[destination] = distance + 1;
                }
            }
        }
    }

    int64_t findSteps(const int64_t steps = 64) const {
        int64_t count = 0;
        for (const auto tile : distances.data) {
            if (tile >= 0 and tile <= steps and ((tile ^ steps) & 1) == 0) {
                count++;
            }
        }
        return count;
    }

    int64_t findSteps2(const int64_t steps = 64) const {
        int64_t count = 0;
        for (const auto y : iota(-steps, steps + 1)) {
            for (const auto x : iota(-steps + std::abs(y), steps - std::abs(y) + 1)) {
                const auto tile = getModDist(start + Vec2l{x, y});
                if (tile >= 0 and tile <= steps and ((tile ^ steps) & 1) == 0) {
                    count++;
                }
            }
        }
        return count;
    }

    void checkLoops() {
        for (const auto &[compass, offset1, offset2] : repeats) {
            fmt::print("Checking {}: ", compass);
            int64_t lastDist = 0;
            for (const auto y : iota(0, garden.height)) {
                for (const auto x : iota(0, garden.width)) {
                    if (distances[x, y] != -1) {
                        const auto dist1 = distancesMod.at(
                            {x + offset1.x * garden.width, y + offset1.y * garden.height});
                        const auto dist2 = distancesMod.at(
                            {x + offset2.x * garden.height, y + offset2.y * garden.height});
                        if (lastDist != dist2 - dist1) {
                            if (lastDist != 0) {
                                fmt::print("distances differ: {} and {}\n", lastDist,
                                           dist2 - dist1);
                                throw std::runtime_error("distances differ");
                            }
                            lastDist = dist2 - dist1;
                            fmt::print("{}\n", lastDist);
                        }
                    }
                }
            }
            compassSteps.push_back(lastDist);
        }
    }

    int64_t getModDist(const Vec2l &pos) const {
        const auto quad = Vec2l{floordiv(pos.x, garden.width), floordiv(pos.y, garden.height)};
        Vec2l qStep{0, 0};

        if (quad.x == 0 or quad.y == 0) {
            if (std::abs(quad.x) > 3 or std::abs(quad.y) > 3) {
                // *3 is needed for the example input
                qStep = quad - signum(quad) * 3;
            }
        } else {
            qStep = quad - signum(quad);
        }

        const auto adjustedPos = pos - qStep * garden.width;
        const auto adjustedSteps = manhattan(qStep, {0, 0}) * garden.width;
        if (distancesMod.contains(adjustedPos)) {
            return distancesMod.at(adjustedPos) + adjustedSteps;
        } else {
            return -1;
        }
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Garden garden{argv[1]};
    for (const auto n : {6, 10, 50, 64}) {
        fmt::print("After {} steps you reach {} / {} tiles.\n", n, garden.findSteps(n),
                   garden.findSteps2(n));
    }
    const auto actualSteps = 26501365;
    const auto seriesOffset = actualSteps % garden.garden.width;
    for (const auto m : iota(0, 10)) {
        const auto n = seriesOffset + m * garden.garden.width;
        fmt::print("{},{}\n", n, garden.findSteps2(n));
        // put result into WA :D
    }
    for (const auto n : {100, 500, 1000, 5000}) {
        fmt::print("After {} steps you reach {} / {} tiles.\n", n, garden.findSteps(n),
                   garden.findSteps2(n));
    }
}
