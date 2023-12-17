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

enum Direction : uint8_t { right = 0, up = 1, left = 2, down = 3, none = 7 };

enum Bool : int8_t { no = 0, yes = 1 };

struct Crucible {
    Vec2l pos{0, 0};
    int64_t loss{0};
    Direction movingDirection{none};
    uint8_t movingDist{0};
    bool hasMoved{false};
    // std::vector<Direction> path{};

    bool canMove(const Direction dir) const {
        if (!hasMoved) {
            return true;
        }
        if (dir == ((movingDirection + 2) & 3)) {
            // can't turn back
            return false;
        }
        if (dir != movingDirection) {
            // turn always
            return true;
        } else {
            // must turn after 3 moves
            return movingDist < 3;
        }
    }

    constexpr bool canStop() const { return true; }

    void move(const Direction dir) {
        pos += neighbours4[dir];
        if (movingDirection != dir) {
            movingDirection = dir;
            movingDist = 1;
        } else {
            ++movingDist;
        }
        hasMoved = true;
        // path.push_back(dir);
    }

    operator Vec2l() const { return pos; }

    operator Vec3l() const {
        const int64_t z = movingDist + movingDirection * 256;
        return {pos.x, pos.y, z};
    }

    // quasi sort for pri queue
    auto operator<=>(const Crucible &other) const {
        return std::pair(-loss, pos) <=> std::pair(-other.loss, other.pos);
    }
};

struct UltraCrucible {
    Vec2l pos{0, 0};
    int64_t loss{0};
    Direction movingDirection{none};
    uint8_t movingDist{0};
    bool hasMoved{false};
    // std::vector<Direction> path{};

    bool canMove(const Direction dir) const {
        if (!hasMoved) {
            return true;
        }
        if (dir == ((movingDirection + 2) & 3)) {
            // can't turn back
            return false;
        }
        if (dir != movingDirection) {
            // turn only after 4 moves
            return movingDist >= 4;
        } else {
            // must turn after 10 moves
            return movingDist < 10;
        }
    }

    bool canStop() const {
        // stop only after 4 moves
        return movingDist >= 4;
    }

    void move(const Direction dir) {
        pos += neighbours4[dir];
        if (movingDirection != dir) {
            movingDirection = dir;
            movingDist = 1;
        } else {
            ++movingDist;
        }
        hasMoved = true;
        // path.push_back(dir);
    }

    operator Vec2l() const { return pos; }

    operator Vec3l() const {
        const int64_t z = movingDist + movingDirection * 256;
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

    const auto loss = findPath(floor, Crucible{});
    fmt::print("You lose {} heat\n", loss);

    const auto ultraloss = findPath(floor, UltraCrucible{});
    fmt::print("You ultra lose {} heat\n", ultraloss);
}
