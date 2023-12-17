#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <ranges>
#include <string>
#include <vector>

#include "grid3d.hpp"
#include "vec2.hpp"
#include "vec3.hpp"

using std::views::iota;

constexpr std::array<Vec2<int64_t>, 4> neighbours4{{{1, 0}, {0, -1}, {-1, 0}, {0, 1}}};

enum Direction { right = 0, up = 1, left = 2, down = 3 };
constexpr std::array<std::string, 4> DirectionStr{"right", "up", "left", "down"};

enum Bool : int8_t { no = 0, yes = 1 };

constexpr auto moveTo(const Direction dir) { return neighbours6[dir]; }

struct Position {
    Vec2l pos{};
    int64_t loss{};
    std::array<Direction, 3> hist{};
    // std::vector<Direction> path{};

    bool canMove(const Direction dir) const {
        // can't turn
        if (dir == (hist[2] + 2) % 4) {
            return false;
        }
        return hist[0] != dir or hist[1] != dir or hist[2] != dir;
    }

    void move(const Direction dir) {
        pos += neighbours4[dir];
        hist[0] = hist[1];
        hist[1] = hist[2];
        hist[2] = dir;
        // path.push_back(dir);
    }

    operator Vec2l() const { return pos; }

    operator Vec3l() const { return {pos.x, pos.y, hist[0] + hist[1] * 4 + hist[2] * 16}; }

    // quasi sort for pri queue
    auto operator<=>(const Position &other) const {
        return std::pair(-loss, pos) <=> std::pair(-other.loss, other.pos);
    }
};

int64_t findPath(const Grid3d<int8_t> &floor, const Position &start) {
    const Vec2l destination{floor.width - 1, floor.height - 1};
    Grid3d<Bool> visited{floor.width, floor.height, floor.depth, no, yes};
    std::priority_queue<Position> frontier{};
    frontier.push(start);

    while (!frontier.empty()) {
        const auto position = frontier.top();
        frontier.pop();

        // fmt::print("checking {}\n", position.pos);
        visited[position] = yes;
        if (position == destination) {
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
                if (visited[newPosition] == no) {
                    visited[newPosition] = yes;
                    newPosition.loss += floor[newPosition];
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
    Grid3d<int8_t> floor(argv[1], 4 * 4 * 4, -1);

    // starting path with invalid history
    const auto loss = findPath(floor, Position{{0, 0}, 0, {up, down, left}});

    fmt::print("You lose {} heat\n", loss);
}
