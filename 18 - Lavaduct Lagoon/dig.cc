#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <stack>
#include <string>
#include <vector>

#include "grid.hpp"
#include "simpleparser.hpp"
#include "vec2.hpp"

using std::views::iota;
using std::views::reverse; // for iota(0,10) | reverse

struct Instruction {
    char dir{};
    int8_t len{};
    std::string color;

    constexpr static const std::string DirName = "RULD";

    Instruction(SimpleParser &scan) {
        dir = scan.getChar();
        len = static_cast<int8_t>(scan.getInt64());
        scan.skipChar('(');
        scan.skipChar('#');
        color = scan.getAlNum();
        scan.skipChar(')');
    }

    const auto &getDir() const { return neighbours4.at(DirName.find(dir)); }
};

struct Ground {
    std::vector<Instruction> plan{};
    Vec2l offset{};
    Grid<char> ground{380, 187 + 233, '.'};
    int64_t filled = 0;

    Ground(SimpleParser &scan) {
        Vec2l pos{0, 0};
        Vec2l min{0, 0};
        Vec2l max{0, 0};
        while (!scan.isEof()) {
            plan.emplace_back(scan);
            const auto &dir = plan.back();
            pos += dir.getDir() * dir.len;
            min = {std::min(min.x, pos.x), std::min(min.y, pos.y)};
            max = {std::max(max.x, pos.x), std::max(max.y, pos.y)};
        }
        offset = min * -1;
        fmt::print("ground [{}, {}]\n", min, max);
        // ground = Grid{max.x, max.y, '.'};
    }

    void dig() {
        Vec2l position = offset;
        ground[position] = '#';
        for (const auto &inst : plan) {
            const auto direction = inst.getDir();
            for ([[maybe_unused]] const auto n : iota(0, inst.len)) {
                position += direction;
                ground[position] = '#';
                ++filled;
            }
        }
    }

    void fill() {
        const Vec2l start = offset + Vec2l{1, 1};
        if (ground[start] != '.') {
            fmt::print("no place to start flood fill!\n");
            throw;
        }

        std::stack<Vec2l> frontier{};
        ground[start] = '#';
        ++filled;
        frontier.push(start);
        while (!frontier.empty()) {
            const auto pos = frontier.top();
            frontier.pop();
            for (const auto dir : neighbours4) {
                const auto next = pos + dir;
                if (ground[next] == '.') {
                    ground[next] = '#';
                    ++filled;
                    frontier.push(next);
                }
            }
        }
    }

    void print() const { ground.print(); }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scan{argv[1]};
    Ground ground{scan};
    ground.dig();
    ground.fill();
    fmt::print("There are {} filled tiles\n");
}
