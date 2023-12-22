#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "simpleparser.hpp"
#include "utility.hpp"
#include "vec3.hpp"

using std::views::iota;
using std::views::reverse;

constexpr static bool debug = false;
constexpr static size_t groundId = 2023; // std::numeric_limits<size_t>::max();

struct Brick {
    const char name{'#'};
    Vec3l start{};
    Vec3l end{};

    std::vector<size_t> supports{};
    std::vector<size_t> supported_by{};

    Brick() = default;
    Brick(SimpleParser &scan, const char name = '#') : name(name) {
        start.x = scan.getInt64();
        scan.skipChar(',');
        start.y = scan.getInt64();
        scan.skipChar(',');
        start.z = scan.getInt64();
        scan.skipChar('~');
        end.x = scan.getInt64();
        scan.skipChar(',');
        end.y = scan.getInt64();
        scan.skipChar(',');
        end.z = scan.getInt64();

        if (start.z > end.z) {
            std::swap(start, end);
        }
    }

    int64_t size() const { return manhattan(start, end); }
};

struct Edge {
    size_t from;
    size_t to;
    bool operator==(const Edge &other) const { return from == other.from and to == other.to; }
};

template <> struct std::hash<Edge> {
    constexpr std::size_t operator()(const Edge &e) const noexcept {
        return e.from * 1001 + e.to * 2642257;
    }
};

struct Stack {
    std::vector<Brick> bricks{};
    std::unordered_map<Vec3l, size_t> occupied{};
    Brick ground{};
    std::unordered_set<Edge> edges{};

    Stack(SimpleParser &scan) {
        char brickId = 0;
        while (!scan.isEof()) {
            bricks.emplace_back(scan, static_cast<char>(++brickId) + '@');
            if (brickId == 26) {
                brickId = 0;
            }
            putBrick(bricks.size() - 1);
        }
    }

    void putBrick(size_t id) {
        const auto &brick = bricks[id];
        for (const auto x : iota(brick.start.x, brick.end.x + 1)) {
            for (const auto y : iota(brick.start.y, brick.end.y + 1)) {
                for (const auto z : iota(brick.start.z, brick.end.z + 1)) {
                    if (debug and occupied.contains({x, y, z})) {
                        fmt::print("Failed to place brick {} at {}!\n", id, Vec3l{x, y, z});
                    }
                    occupied[{x, y, z}] = id;
                }
            }
        }
    }

    void eraseBrick(size_t id) {
        const auto &brick = bricks[id];
        for (const auto x : iota(brick.start.x, brick.end.x + 1)) {
            for (const auto y : iota(brick.start.y, brick.end.y + 1)) {
                for (const auto z : iota(brick.start.z, brick.end.z + 1)) {
                    if (debug and occupied.at({x, y, z}) != id) {
                        fmt::print("Failed to remove brick {} [{}, {}] from {}!\n", id, brick.start,
                                   brick.end, Vec3l{x, y, z});
                        const auto &oid = occupied.at({x, y, z});
                        fmt::print("Found brick {} [{}, {}] there\n", oid, bricks[oid].start,
                                   bricks[oid].end);
                    }
                    occupied.erase({x, y, z});
                }
            }
        }
    }

    bool dropBrick(size_t id) {
        auto &brick = bricks[id];
        const auto zOld = brick.start.z;
        auto zNew = zOld;

        for (const auto z : iota(1, zOld) | reverse) {
            bool otherBrick = false;
            for (const auto x : iota(brick.start.x, brick.end.x + 1)) {
                for (const auto y : iota(brick.start.y, brick.end.y + 1)) {
                    if (occupied.contains({x, y, z})) {
                        otherBrick = true;
                    }
                }
            }
            if (otherBrick) {
                break;
            }
            zNew = z;
        }
        if (zOld == zNew) {
            return false;
        }
        if constexpr (false) {
            fmt::print("Moving brick {} down {}\n", id, zOld - zNew);
        }
        eraseBrick(id);
        brick.start -= {0, 0, zOld - zNew};
        brick.end -= {0, 0, zOld - zNew};
        putBrick(id);
        return true;
    }

    void dropAllBricks() {
        bool brickDropped = false;
        do {
            brickDropped = false;
            for (const auto id : iota(0u, bricks.size())) {
                brickDropped |= dropBrick(id);
            }
        } while (brickDropped);
    }

    void findSupport() {
        edges.clear();
        for (const auto &[pos, id] : occupied) {
            const auto above = pos + Vec3l{0, 0, 1};
            if (occupied.contains(above)) {
                const auto aid = occupied.at(above);
                if (aid != id) {
                    edges.emplace(id, aid);
                }
            }
            if (pos.z == 1) {
                if constexpr (debug) {
                    fmt::print("{} at ground level\n", id);
                }
                edges.emplace(groundId, id);
            }
        }
        for (const auto &[from, to] : edges) {
            if constexpr (debug) {
                fmt::print("Edge [{}, {}]\n", from, to);
            }
            if (from != groundId) {
                bricks[from].supports.push_back(to);
                bricks[to].supported_by.push_back(from);
            } else {
                ground.supports.push_back(to);
            }
        }
    };

    std::pair<std::vector<int64_t>, std::vector<int64_t>> findRemovables() const {
        std::vector<int64_t> removables{};
        std::vector<int64_t> essentials{};
        for (const auto id : iota(0u, bricks.size())) {
            bool isSingleSupport = false;
            for (const auto suppId : bricks[id].supports) {
                if (bricks[suppId].supported_by.size() == 1) {
                    isSingleSupport = true;
                }
            }
            if (!isSingleSupport) {
                removables.push_back(id);
            } else {
                essentials.push_back(id);
            }
        }
        return {removables, essentials};
    }

    void printExample() const {
        for (const auto z : iota(0, 10) | reverse) {
            for (const auto x : iota(0, 3)) {
                char pos = '.';
                for (const auto y : iota(0, 3)) {
                    if (occupied.contains({x, y, z})) {
                        pos = bricks[occupied.at({x, y, z})].name;
                    }
                }
                std::cout << pos;
            }
            std::cout << '\n';
        }

        std::cout << '\n';
        for (const auto z : iota(0, 10) | reverse) {
            for (const auto y : iota(0, 3)) {
                char pos = '.';
                for (const auto x : iota(0, 3)) {
                    if (occupied.contains({x, y, z})) {
                        pos = bricks[occupied.at({x, y, z})].name;
                    }
                }
                std::cout << pos;
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    int64_t fallIfDisintegrated(const size_t id) const {
        std::unordered_set<size_t> visited{};
        bfs(groundId, id, visited);
        return bfs(id, id, visited);
    }
    int64_t bfs(const auto &start, const auto &removed, auto &visited) const {
        int64_t count = 0;
        std::queue<size_t> frontier{};
        frontier.push(start);
        while (!frontier.empty()) {
            const auto currentId = frontier.front();
            frontier.pop();

            for (const auto nextId :
                 currentId == groundId ? ground.supports : bricks[currentId].supports) {
                if (nextId == removed) {
                    continue;
                }
                if (visited.contains(nextId)) {
                    continue;
                }
                frontier.push(nextId);
                visited.insert(nextId);
                ++count;
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

    SimpleParser scan{argv[1]};
    Stack jenga{scan};
    // jenga.printExample();
    jenga.dropAllBricks();
    // jenga.printExample();
    jenga.findSupport();
    const auto removables = jenga.findRemovables();
    if constexpr (debug) {
        for (const auto &r : removables.first) {
            fmt::print("can disintegrate brick {}\n", r);
        }
    }
    fmt::print("You may disintegrate any of {} bricks\n", removables.first.size());

    int64_t sumDanglingBricks = 0;
    for (const auto id : removables.second) {
        const auto dangling = jenga.fallIfDisintegrated(id);
        sumDanglingBricks += dangling;
        if constexpr (debug) {
            fmt::print("Disintegrating brick {} leaves {} bricks dangling\n", id, dangling);
        }
    }
    fmt::print("You can make {} bricks fall\n", sumDanglingBricks);
}
