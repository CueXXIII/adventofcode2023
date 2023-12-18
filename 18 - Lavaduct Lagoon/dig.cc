#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <stack>
#include <string>
#include <vector>

#include "grid.hpp"
#include "simpleparser.hpp"
#include "vec2.hpp"

using std::views::iota;

struct Instruction {
    int64_t len{};
    int64_t len2{};
    char dir{};
    char dir2{};

    // for neighbours4[]
    constexpr static const std::string DirName = "RULD";
    // ordered as encoded in the input
    constexpr static const std::string Dir2Name = "RDLU";

    Instruction(SimpleParser &scan) {
        dir = scan.getChar();
        len = static_cast<int8_t>(scan.getInt64());
        scan.skipChar('(');
        scan.skipChar('#');
        const auto color = scan.getAlNum();
        scan.skipChar(')');
        len2 = std::stoll(color, nullptr, 16) >> 4;
        dir2 = Dir2Name[color[color.size() - 1] - '0'];
    }

    const auto &getDir() const { return neighbours4.at(DirName.find(dir)); }
    const auto &getDir2() const { return neighbours4.at(DirName.find(dir2)); }
};

struct Ground {
    std::vector<Instruction> plan{};
    Vec2l offset{};
    Vec2l min2{0, 0};
    Vec2l max2{0, 0};
    Grid<char> ground{380, 187 + 233, '.'};
    int64_t filled = 0;

    Ground(SimpleParser &scan) {
        Vec2l pos{0, 0};
        Vec2l min{0, 0};
        Vec2l max{0, 0};

        Vec2l pos2{0, 0};
        while (!scan.isEof()) {
            plan.emplace_back(scan);
            const auto &dir = plan.back();
            pos += dir.getDir() * dir.len;
            pos2 += dir.getDir2() * dir.len2;
            min = {std::min(min.x, pos.x), std::min(min.y, pos.y)};
            max = {std::max(max.x, pos.x), std::max(max.y, pos.y)};
            min2 = {std::min(min2.x, pos2.x), std::min(min2.y, pos2.y)};
            max2 = {std::max(max2.x, pos2.x), std::max(max2.y, pos2.y)};
        }
        offset = min * -1;
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

    // --- part 2 ---
    // every corner is 90° (checked manually)
    struct DigBorder {
        int64_t x;
        bool toInside;
    };

    std::vector<DigBorder> digBorder{};

    std::vector<std::pair<Vec2l, Vec2l>> poly{};

    // init polygon
    void getPoly() {
        Vec2l position{0, 0};
        for (const auto &inst : plan) {
            const auto next = position + inst.getDir2() * inst.len2;
            if (position.x <= next.x and position.y <= next.y) {
                poly.emplace_back(position, next);
            } else {
                poly.emplace_back(next, position);
            }
            position = next;
        }
        std::ranges::sort(poly);
    }

    // get vertical adjacent poly
    const auto &getAdj(const auto &pos) {
        for (const auto &stroke : poly) {
            if (stroke.first == pos or stroke.second == pos) {
                if (stroke.first.y == stroke.second.y) {
                    return stroke;
                }
            }
        }
        throw "Missing segment";
    }

    int64_t findNextScanline(const int64_t scanline) {
        int64_t next = std::numeric_limits<int64_t>::max();
        for (const auto &stroke : poly) {
            if (stroke.first.y > scanline) {
                next = std::min(next, stroke.first.y);
            }
            if (stroke.second.y > scanline) {
                next = std::min(next, stroke.second.y);
            }
        }
        return next;
    }

    // count tiles in inclusive interval
    static inline int64_t interval(const int64_t from, const int64_t to) { return to - from + 1; }

    int64_t fill2() {
        int64_t filled = 0;

        getPoly();

        // scan y lines
        int64_t scanline = min2.y;

        while (scanline <= max2.y) {
            std::vector<std::pair<Vec2l, Vec2l>> thisDigLines{};
            for (const auto &digLine : poly) {
                if (digLine.first.y == scanline and digLine.second.y == scanline) {
                    thisDigLines.push_back(digLine);
                }
            }

            // fill on horizontal digs
            bool inside = false;
            int64_t xPos = min2.x; // filled up to this pos
            std::vector<DigBorder> newBorder{};
            auto horIt = thisDigLines.begin();
            auto verIt = digBorder.begin();
            while (horIt != thisDigLines.end() or verIt != digBorder.end()) {
                std::optional<Vec2l> horPos{};
                std::optional<Vec2l> horEnd{};
                std::optional<Vec2l> verPos{};
                if (horIt != thisDigLines.end()) {
                    horPos = horIt->first;
                    horEnd = horIt->second;
                }
                if (verIt != digBorder.end()) {
                    verPos = Vec2l{verIt->x, scanline};
                }
                if (!horPos and !verPos) {
                    throw("we have nothing?!");
                }
                // assume at least 1 horPos in the input
                if (horPos and (!verPos or verPos->x >= horPos->x)) {
                    // fmt::print("inside {}\n", *horPos);
                    if (inside) {
                        filled += interval(xPos + 1, horPos->x - 1);
                    }
                    filled += interval(horPos->x, horEnd->x);
                    xPos = horEnd->x;

                    // connect begin
                    if (!verPos or verPos->x != horPos->x) {
                        inside = !inside;
                        newBorder.emplace_back(horPos->x, inside);
                    }
                    if (verPos and verPos->x == horPos->x) {
                        ++verIt;
                        if (verIt == digBorder.end()) {
                            verPos = {};
                        } else {
                            verPos = Vec2l{verIt->x, scanline};
                        }
                    }

                    // connect end
                    if (!verPos or verPos->x != horEnd->x) {
                        inside = !inside;
                        newBorder.emplace_back(horEnd->x, inside);
                    }
                    if (verPos and verPos->x == horEnd->x) {
                        ++verIt;
                        if (verIt == digBorder.end()) {
                            verPos = {};
                        } else {
                            verPos = Vec2l{verIt->x, scanline};
                        }
                    }
                    ++horIt;
                } else if (verPos) {
                    newBorder.push_back(*verIt);
                    if (!inside) {
                        ++filled;
                        xPos = verPos->x;
                    } else {
                        filled += interval(xPos + 1, verPos->x);
                        xPos = verPos->x;
                    }
                    ++verIt;
                    inside = !inside;
                }
            }
            digBorder = std::move(newBorder);
            if (inside) {
                fmt::print("Still inside on y={}?\n", scanline);
                throw("inside out");
            }

            const auto nextScanline = findNextScanline(scanline);

            // fill between horizontal digs
            if (nextScanline > scanline + 1) {
                const int64_t height = nextScanline - scanline - 1;
                int64_t fillTiles = 0;
                inside = false;
                xPos = min2.x;
                for (const auto &stroke : digBorder) {
                    if (!inside) {
                        ++fillTiles;
                    } else {
                        fillTiles += interval(xPos + 1, stroke.x);
                    }
                    inside = !inside;
                    xPos = stroke.x;
                }
                filled += fillTiles * height;
            }

            scanline = nextScanline;
        }
        return filled;
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
    fmt::print("There are {} filled tiles\n", ground.filled);
    const auto tiles2 = ground.fill2();
    fmt::print("There are {} painted tiles\n", tiles2);
}
