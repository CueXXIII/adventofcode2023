#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <list>
#include <ranges>
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
    constexpr static const std::string ElfDir = "RDLU";

    Instruction(SimpleParser &scan) {
        dir = scan.getChar();
        len = static_cast<int8_t>(scan.getInt64());
        scan.skipChar('(');
        scan.skipChar('#');
        const auto color = scan.getAlNum();
        len2 = std::stoll(color, nullptr, 16) >> 4;
        dir2 = ElfDir[color[color.size() - 1] - '0'];
        scan.skipChar(')');
    }

    const auto &getDir() const { return neighbours4.at(DirName.find(dir)); }
    const auto &getDir2() const { return neighbours4.at(DirName.find(dir2)); }
};

struct Poly {
    std::vector<std::pair<Vec2l, Vec2l>> edges{};
    int64_t minY = 0;
    Poly() = default;
    Poly(const auto &plan, bool decode = false) {
        Vec2l position{0, 0};
        for (const auto &inst : plan) {
            const auto next =
                position + (decode ? inst.getDir2() * inst.len2 : inst.getDir() * inst.len);
            if (position.x <= next.x and position.y <= next.y) {
                edges.emplace_back(position, next);
            } else {
                edges.emplace_back(next, position);
            }
            minY = std::min(minY, position.y);
            position = next;
        }
        std::ranges::sort(edges);
    }

    int64_t findNextScanline(const int64_t scanline) {
        int64_t next = std::numeric_limits<int64_t>::max();
        for (const auto &edge : edges) {
            if (edge.first.y > scanline) {
                next = std::min(next, edge.first.y);
            }
            if (edge.second.y > scanline) {
                next = std::min(next, edge.second.y);
            }
        }
        return next;
    }
};

struct Ground {
    std::vector<Instruction> plan{};
    Poly poly1;
    Poly poly2;

    Ground(SimpleParser &scan) {
        while (!scan.isEof()) {
            plan.emplace_back(scan);
        }
        poly1 = std::move(Poly{plan, false});
        poly2 = std::move(Poly{plan, true});
    }

    // count tiles in inclusive interval
    static inline int64_t interval(const int64_t from, const int64_t to) { return to - from + 1; }

    int64_t fill2(const bool part2 = true) {
        int64_t filled = 0;
        std::list<int64_t> vertEdges{};

        auto &poly = part2 ? poly2 : poly1;

        // scan y lines
        int64_t scanline = poly.minY;

        while (scanline < std::numeric_limits<int64_t>::max()) {
            // every corner is 90° (checked manually)
            std::list<std::pair<int64_t, int64_t>> horizEdges{};
            for (const auto &edge : poly.edges) {
                if (edge.first.y == scanline and edge.second.y == scanline) {
                    horizEdges.emplace_back(edge.first.x, edge.second.x);
                }
            }

            // fill on horizontal digs
            bool inside = false;
            int64_t xPos{}; // filled up to this pos
            std::list<int64_t> newVertEdges{};
            while (!horizEdges.empty() or !vertEdges.empty()) {
                // horizontal edge is first
                if (!horizEdges.empty() and
                    (vertEdges.empty() or vertEdges.front() >= horizEdges.front().first)) {
                    if (inside) {
                        filled += interval(xPos + 1, horizEdges.front().first - 1);
                    }
                    filled += interval(horizEdges.front().first, horizEdges.front().second);
                    xPos = horizEdges.front().second;

                    // connect begin
                    if (vertEdges.empty() or vertEdges.front() != horizEdges.front().first) {
                        inside = !inside;
                        newVertEdges.emplace_back(horizEdges.front().first);
                    }
                    if (!vertEdges.empty() and vertEdges.front() == horizEdges.front().first) {
                        vertEdges.pop_front();
                    }

                    // connect end
                    if (vertEdges.empty() or vertEdges.front() != horizEdges.front().second) {
                        inside = !inside;
                        newVertEdges.emplace_back(horizEdges.front().second);
                    }
                    if (!vertEdges.empty() and vertEdges.front() == horizEdges.front().second) {
                        vertEdges.pop_front();
                    }
                    horizEdges.pop_front();
                } else {
                    // vertical edge is first
                    newVertEdges.push_back(vertEdges.front());
                    if (!inside) {
                        ++filled;
                        xPos = vertEdges.front();
                    } else {
                        filled += interval(xPos + 1, vertEdges.front());
                        xPos = vertEdges.front();
                    }
                    vertEdges.pop_front();
                    inside = !inside;
                }
            }
            vertEdges = std::move(newVertEdges);

            // fill between horizontal digs
            const auto nextScanline = poly.findNextScanline(scanline);
            if (nextScanline > scanline + 1) {
                const int64_t height = nextScanline - scanline - 1;
                int64_t fillTiles = 0;
                for (const auto &edge : vertEdges) {
                    if (!inside) {
                        ++fillTiles;
                    } else {
                        fillTiles += interval(xPos + 1, edge);
                    }
                    inside = !inside;
                    xPos = edge;
                }
                filled += fillTiles * height;
            }

            scanline = nextScanline;
        }
        return filled;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scan{argv[1]};
    Ground ground{scan};
    const auto tiles1 = ground.fill2(false);
    fmt::print("There are {} filles tiles\n", tiles1);
    const auto tiles2 = ground.fill2(true);
    fmt::print("There are {} painted tiles\n", tiles2);
}
