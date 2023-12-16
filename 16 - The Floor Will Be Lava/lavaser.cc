#include <algorithm>
#include <execution>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>

#include "grid.hpp"
#include "vec2.hpp"

using std::views::iota;
// using std::views::reverse; // for iota(0,10) | reverse

enum Direction { right, up, left, down };

struct Photon {
    Vec2l position;
    Vec2l direction;
    bool operator==(const Photon &other) const {
        return std::pair(position, direction) == std::pair(other.position, other.direction);
    }
    auto operator<=>(const Photon &other) const {
        return std::pair(position, direction) <=> std::pair(other.position, other.direction);
    }
};

template <> struct std::hash<Photon> {
    constexpr std::size_t operator()(const Photon &p) const noexcept {
        // just draw a few primes > 100
        return std::hash<Vec2l>{}(p.position) * 1637161u +
               std::hash<Vec2l>{}(p.direction) * 926617u;
    }
};

struct VulcanRoom {
    const Grid<char> mirrors{};
    const int64_t width{};
    const int64_t height{};

    VulcanRoom(const char *file) : mirrors(file), width(mirrors.width), height(mirrors.height){};

    int64_t energize1() const { return energize{*this}(Photon{{-1, 0}, {1, 0}}); }

    // all
    int64_t energize2() const {
        std::vector<int64_t> maxValues{};
        maxValues.resize(std::max(width, height));
        std::iota(maxValues.begin(), maxValues.end(), 0);

        std::transform(
            std::execution::par_unseq, maxValues.begin(), maxValues.end(), maxValues.begin(),
            [this](const auto &n) {
                int64_t maxPower = 0;
                if (n < width) {
                    maxPower = energize{*this}({{n, -1}, {0, 1}});
                    maxPower = std::max(maxPower, energize{*this}({{n, height}, {0, -1}}));
                }
                if (n < height) {
                    maxPower = std::max(maxPower, energize{*this}({{-1, n}, {1, 0}}));
                    maxPower = std::max(maxPower, energize{*this}({{width, n}, {-1, 0}}));
                }
                return maxPower;
            });

        return std::ranges::max(maxValues);
    }

    struct energize {
        const VulcanRoom &v;
        Grid<int8_t> floor{};
        std::unordered_set<Photon> photons{};

        energize(const VulcanRoom &v) : v(v), floor(v.width, v.height, 0) {}
        int64_t energizedTiles() const {
            return std::ranges::count_if(floor.data, [](auto v) { return v != 0; });
        }

        int64_t operator()(const Photon &beam) {
            follow(beam);
            return energizedTiles();
        }

        void follow(Photon beam) {
            beam.position += beam.direction;
            // fmt::print("follow(({}, {}))\n", beam.position, beam.direction);
            if (beam.position.x < 0 or beam.position.x >= v.width or beam.position.y < 0 or
                beam.position.y >= v.height) {
                // fmt::print("left the floor\n");
                return;
            }
            if (photons.contains(beam)) {
                // fmt::print("looping beam\n");
                return;
            }
            photons.insert(beam);
            floor[beam.position] = 1;

            switch (v.mirrors[beam.position]) {
            case '.':
                follow(beam);
                break;
            case '/':
                follow({beam.position, {-beam.direction.y, -beam.direction.x}});
                break;
            case '\\':
                follow({beam.position, {+beam.direction.y, +beam.direction.x}});
                break;
            case '|':
                if (beam.direction.x == 0) {
                    follow(beam);
                } else {
                    follow({beam.position, {0, -1}});
                    follow({beam.position, {0, +1}});
                }
                break;
            case '-':
                if (beam.direction.y == 0) {
                    follow(beam);
                } else {
                    follow({beam.position, {-1, 0}});
                    follow({beam.position, {+1, 0}});
                }
                break;
            }
        }
    };
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
