#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <ranges>
#include <set>
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
constexpr static size_t groundId = 2023;

struct Brick {
    Vec3l start{};
    Vec3l end{};

    std::set<size_t> supports{};
    std::set<size_t> supported_by{};

    Brick() = default;
    Brick(SimpleParser &scan) {
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

struct Stack {
    std::vector<Brick> bricks{};
    std::unordered_map<Vec3l, size_t> occupied{};
    Brick ground{};

    Stack(SimpleParser &scan) {
        while (!scan.isEof()) {
            bricks.emplace_back(scan);
        }

        std::ranges::sort(
            bricks, [](const auto &lhs, const auto &rhs) { return lhs.start.z < rhs.start.z; });

        for (const auto id : iota(0u, bricks.size())) {
            dropBrick(id);
        }
        findSupport();
    }

    void putBrick(size_t id) {
        const auto &brick = bricks[id];
        for (const auto x : iota(brick.start.x, brick.end.x + 1)) {
            for (const auto y : iota(brick.start.y, brick.end.y + 1)) {
                for (const auto z : iota(brick.start.z, brick.end.z + 1)) {
                    if constexpr (debug) {
                        if (occupied.contains({x, y, z})) {
                            fmt::print("Failed to place brick {} at {}!\n", id, Vec3l{x, y, z});
                        }
                    }
                    occupied[{x, y, z}] = id;
                }
            }
        }
    }

    void dropBrick(size_t id) {
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
        if constexpr (false) {
            fmt::print("Moving brick {} down {}\n", id, zOld - zNew);
        }
        brick.start -= {0, 0, zOld - zNew};
        brick.end -= {0, 0, zOld - zNew};
        putBrick(id);
    }

    void findSupport() {
        for (const auto &[pos, fromId] : occupied) {
            const auto above = pos + Vec3l{0, 0, 1};
            if (occupied.contains(above)) {
                const auto toId = occupied.at(above);
                if (toId != fromId) {
                    bricks[fromId].supports.insert(toId);
                    bricks[toId].supported_by.insert(fromId);
                }
            }
            if (pos.z == 1) {
                if constexpr (debug) {
                    fmt::print("{} at ground level\n", fromId);
                }
                ground.supports.insert(fromId);
            }
        }
    };

    std::pair<int64_t, int64_t> evaluateSupport() const {
        std::pair<int64_t, int64_t> result{0, 0};
        auto &disintegrateCount = result.first;
        auto &failCount = result.second;
        for (const auto id : iota(0u, bricks.size())) {
            bool isSingleSupport = false;
            for (const auto suppId : bricks[id].supports) {
                if (bricks[suppId].supported_by.size() == 1) {
                    isSingleSupport = true;
                }
            }
            if (!isSingleSupport) {
                if constexpr (debug) {
                    fmt::print("can disintegrate brick {}\n", id);
                }
                ++disintegrateCount;
            } else {
                const auto fails = fallIfDisintegrated(id);
                if constexpr (debug) {
                    fmt::print("Disintegrating brick {} leaves {} bricks dangling\n", id, fails);
                }
                failCount += fails;
            }
        }
        return result;
    }

    int64_t fallIfDisintegrated(const size_t id) const {
        std::unordered_set<size_t> visited{};
        bfs(groundId, id, visited);
        return bfs(id, id, visited);
    }
    int64_t bfs(const auto &start, const auto &disintegrated, auto &visited) const {
        int64_t count = 0;
        std::queue<size_t> frontier{};
        frontier.push(start);
        while (!frontier.empty()) {
            const auto currentId = frontier.front();
            frontier.pop();

            for (const auto nextId :
                 currentId == groundId ? ground.supports : bricks[currentId].supports) {
                if (nextId == disintegrated) {
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
    const auto brickReport = jenga.evaluateSupport();
    fmt::print("You may disintegrate any of {} bricks\n", brickReport.first);
    fmt::print("You can make {} bricks fall\n", brickReport.second);
}
