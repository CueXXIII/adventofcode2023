#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>

#include "grid.hpp"
#include "simpleparser.hpp"

using std::views::iota;

Vec2l findAnimal(const auto &grid) {
    for (const auto y : iota(0, grid.height)) {
        for (const auto x : iota(0, grid.width)) {
            if (grid[x, y] == 'S') {
                return {x, y};
            }
        }
    }
    fmt::print("No animal found\n");
    return {-1, -1};
}

bool connectsTo(const auto &grid, const Vec2l &src, const Vec2l &dst) {
    const auto diff = src - dst;
    const Vec2l north = neighbours4[1];
    const Vec2l south = neighbours4[3];
    const Vec2l east = neighbours4[0];
    const Vec2l west = neighbours4[2];
    switch (grid[dst]) {
    case '|':
        return diff == north or diff == south;
    case '-':
        return diff == east or diff == west;
    case 'L':
        return diff == north or diff == east;
    case 'J':
        return diff == north or diff == west;
    case '7':
        return diff == south or diff == west;
    case 'F':
        return diff == south or diff == east;
    }
    return false;
}

bool connected(const auto &grid, const Vec2l &a, const Vec2l &b) {
    return connectsTo(grid, a, b) and connectsTo(grid, b, a);
}

void replaceAnimalWithPipe(auto &grid, const Vec2l &src, const Vec2l &dst1, const Vec2l &dst2) {
    for (const auto pipe : {'|', '-', 'L', 'J', '7', 'F'}) {
        grid[src] = pipe;
        if (connected(grid, src, dst1) and connected(grid, src, dst2)) {
            return;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Grid<char> pipes1{argv[1], '.'};

    std::unordered_set<Vec2l> visited1;
    Vec2l animal = findAnimal(pipes1);
    fmt::print("Animal starts at {}\n", animal);
    visited1.insert(animal);
    std::vector<Vec2l> positions{};
    for (const auto diff : neighbours4) {
        const auto dst = animal + diff;
        if (connectsTo(pipes1, animal, dst)) {
            positions.push_back(dst);
        }
    }
    replaceAnimalWithPipe(pipes1, animal, positions[0], positions[1]);

    int64_t steps = 1;
    while (positions[0] != positions[1]) {
        for (auto &pos : positions) {
            visited1.insert(pos);
            for (const auto diff : neighbours4) {
                const auto next = pos + diff;
                if (!visited1.contains(next) and connected(pipes1, pos, next)) {
                    pos = next;
                    break;
                }
            }
        }
        ++steps;
    }
    visited1.insert(positions[0]);
    fmt::print("The farthest point from the animal is {} after {} steps.\n", positions[0], steps);

    Grid<char> nest{pipes1.width, pipes1.height, '.'};
    int64_t inside = 0;
    int64_t outside = 0;
    bool isInside = false;

    for (const auto y : iota(0, pipes1.height)) {
        for (const auto x : iota(0, pipes1.width)) {
            const Vec2l pos{x, y};
            if (visited1.contains(pos)) {
                switch (pipes1[pos]) {
                case '|':
                case 'L':
                case 'J':
                    isInside = !isInside;
                    break;
                }
                nest[pos] = pipes1[pos];
            } else if (isInside) {
                ++inside;
                nest[pos] = 'I';
            } else {
                ++outside;
                nest[pos] = '.';
            }
        }
    }
    fmt::print("There are {} tiles inside the loop and {} outside\n", inside, outside);

    nest[animal] = 'S';
    for (const auto y : iota(0, pipes1.height)) {
        for (const auto x : iota(0, pipes1.width)) {
            const Vec2l pos{x, y};
            switch (nest[pos]) {
            case '|':
                std::cout << "║";
                break;
            case '-':
                std::cout << "═";
                break;
            case 'L':
                std::cout << "╚";
                break;
            case 'J':
                std::cout << "╝";
                break;
            case '7':
                std::cout << "╗";
                break;
            case 'F':
                std::cout << "╔";
                break;
            case 'I':
                std::cout << "▒";
                break;
            case '.':
                std::cout << ' ';
                break;
            case 'S':
                std::cout << "♘";
                break;
            default:
                std::cout << nest[pos];
                break;
            }
        }
        std::cout << '\n';
    }
}
