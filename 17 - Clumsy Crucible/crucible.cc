#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>

#include "grid.hpp"
#include "vec2.hpp"
#include "vec3.hpp"

using std::views::iota;

// constexpr std::array<Vec2<int64_t>, 4> neighbours4{{{1, 0}, {0, -1}, {-1, 0}, {0, 1}}};

enum Direction { right = 0, up = 1, left = 2, down = 3 };
constexpr std::array<std::string, 4> DirectionStr{"right", "up", "left", "down"};

enum Bool : int8_t { no = 0, yes = 1 };

constexpr auto moveTo(const Direction dir) { return neighbours4[dir]; }

struct Crucible {
    Vec2l pos{};
    int64_t loss{};
    std::array<Direction, 3> hist{up, up, up};
    bool hasMoved{false};
    // std::vector<Direction> path{};

    bool canMove(const Direction dir) const {
        if (!hasMoved) {
            return true;
        }
        // can't turn back
        if (dir == (hist[2] + 2) % 4) {
            return false;
        }
        return hist[0] != dir or hist[1] != dir or hist[2] != dir;
    }

    constexpr bool canStop() const { return true; }

    void move(const Direction dir) {
        pos += neighbours4[dir];
        hist[0] = hist[1];
        hist[1] = hist[2];
        hist[2] = dir;
        hasMoved = true;
        // path.push_back(dir);
    }

    operator Vec2l() const { return pos; }

    operator Vec3l() const { return {pos.x, pos.y, hist[0] + hist[1] * 4 + hist[2] * 16}; }

    // quasi sort for pri queue
    auto operator<=>(const Crucible &other) const {
        return std::pair(-loss, pos) <=> std::pair(-other.loss, other.pos);
    }
};

struct UltraCrucible {
    Vec2l pos{};
    int64_t loss{};
    std::array<Direction, 10> hist{up, up, up, up, up, up, up, up, up, up};
    bool hasMoved{false};
    // std::vector<Direction> path{};

    bool canMove(const Direction dir) const {
        if (!hasMoved) {
            return true;
        }
        if (dir == (hist[9] + 2) % 4) {
            // can't turn back
            return false;
        }
        if (dir != hist[9]) {
            // turn only after 4 moves
            for (const auto i : iota(6, 9)) {
                if (hist[i] != hist[9]) {
                    return false;
                }
            }
            return true;
        } else {
            // must turn after 10 moves
            for (const auto i : iota(0, 9)) {
                if (hist[i] != hist[9]) {
                    return true;
                }
            }
            return false;
        }
    }

    bool canStop() const {
        for (const auto i : iota(6, 9)) {
            if (hist[i] != hist[9]) {
                return false;
            }
        }
        return true;
    }

    void move(const Direction dir) {
        pos += neighbours4[dir];
        for (const auto i : iota(0, 9)) {
            hist[i] = hist[i + 1];
        }
        hist[9] = dir;
        hasMoved = true;
        // path.push_back(dir);
    }

    operator Vec2l() const { return pos; }

    operator Vec3l() const {
        int64_t z = 0;
        for (const auto h : hist) {
            z = z * 4 + h;
        }
        return {pos.x, pos.y, z};
    }

    // quasi sort for pri queue
    auto operator<=>(const UltraCrucible &other) const {
        return std::pair(-loss, pos) <=> std::pair(-other.loss, other.pos);
    }
};

template <typename Cart> int64_t findPath(const Grid<int8_t> &floor, const Cart &start) {
    const Vec2l destination{floor.width - 1, floor.height - 1};
    std::unordered_set<Vec3l> visited{};
    std::priority_queue<Cart> frontier{};
    frontier.push(start);

    while (!frontier.empty()) {
        const auto position = frontier.top();
        frontier.pop();

        // fmt::print("checking {}\n", position.pos);
        visited.insert(position);
        if (position == destination and position.canStop()) {
            // fmt::print("Path =");
            // for (const auto dir : position.path) {
            //     fmt::print(" {}", DirectionStr[dir]);
            // }
            // fmt::print("\n");
            return position.loss;
        }
        for (const auto direction : {right, up, left, down}) {
            if (position.canMove(direction)) {
                auto newPosition = position;
                newPosition.move(direction);
                if (floor.validPos(newPosition) and !visited.contains(newPosition)) {
                    visited.insert(newPosition);
                    newPosition.loss += floor[newPosition] - '0';
                    frontier.push(newPosition);
                }
            }
        }
    }
    return -1; // no path found
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    // encode last 3 movements as position
    Grid<int8_t> floor(argv[1], -1);

    const auto loss = findPath(floor, Crucible{{0, 0}, 0});
    fmt::print("You lose {} heat\n", loss);

    const auto ultraloss = findPath(floor, UltraCrucible{{0, 0}, 0});
    fmt::print("You ultra lose {} heat\n", ultraloss);
}
