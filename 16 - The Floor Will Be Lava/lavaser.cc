#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <set>
#include <string>
#include <vector>

#include "grid.hpp"
#include "vec2.hpp"

using std::views::iota;
// using std::views::reverse; // for iota(0,10) | reverse

enum Direction { right, up, left, down };

struct Photon {
    Vec2l position;
    Vec2l direction;
    auto operator<=>(const Photon &other) const {
        return std::pair(position, direction) <=> std::pair(other.position, other.direction);
    }
};

struct VulcanRoom {
    Grid<char> mirrors{};
    int64_t width{};
    int64_t height{};
    Grid<int64_t> floor{};
    std::set<Photon> photons{};

    VulcanRoom(const char *file)
        : mirrors(file), width(mirrors.width), height(mirrors.height), floor(width, height, 0){};

    void clearFloor() {
        for (auto &tile : floor.data) {
            tile = 0;
        }
        photons.clear();
    }

    int64_t energizedTiles() const {
        int64_t count = 0;
        for (const auto &tile : floor.data) {
            if (tile != 0) {
                ++count;
            }
        }
        return count;
    }

    int64_t energize1() {
        energize(Photon{{-1, 0}, {1, 0}});
        // return number of energized tiles
        return energizedTiles();
    }

    // all
    int64_t energize2() {
        int64_t maxPower = 0;
        for (const auto x : iota(0, width)) {
            clearFloor();
            energize({{x, -1}, {0, 1}});
            maxPower = std::max(maxPower, energizedTiles());

            clearFloor();
            energize({{x, height}, {0, -1}});
            maxPower = std::max(maxPower, energizedTiles());
        }

        for (const auto y : iota(0, height)) {
            clearFloor();
            energize({{-1, y}, {1, 0}});
            maxPower = std::max(maxPower, energizedTiles());

            clearFloor();
            energize({{width, y}, {-1, 0}});
            maxPower = std::max(maxPower, energizedTiles());
        }
        return maxPower;
    }

    void energize(Photon beam) {
        beam.position += beam.direction;
        // fmt::print("energize(({}, {}))\n", beam.position, beam.direction);
        if (beam.position.x < 0 or beam.position.x >= width or beam.position.y < 0 or
            beam.position.y >= height) {
            // fmt::print("left the floor\n");
            return;
        }
        if (photons.contains(beam)) {
            // fmt::print("looping beam\n");
            return;
        }
        photons.insert(beam);
        ++floor[beam.position];

        switch (mirrors[beam.position]) {
        case '.':
            energize(beam);
            break;
        case '/':
            energize({beam.position, {-beam.direction.y, -beam.direction.x}});
            break;
        case '\\':
            energize({beam.position, {+beam.direction.y, +beam.direction.x}});
            break;
        case '|':
            if (beam.direction.x == 0) {
                energize(beam);
            } else {
                energize({beam.position, {0, -1}});
                energize({beam.position, {0, +1}});
            }
            break;
        case '-':
            if (beam.direction.y == 0) {
                energize(beam);
            } else {
                energize({beam.position, {-1, 0}});
                energize({beam.position, {+1, 0}});
            }
            break;
        }
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    VulcanRoom room{argv[1]};
    fmt::print("The beam heats {} tiles\n", room.energize1());
    fmt::print("After adjusting the beam you heat {} tiles\n", room.energize2());
}
