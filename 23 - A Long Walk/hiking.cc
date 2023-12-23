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

#include "grid.hpp"

using std::views::iota;
using std::views::reverse;

struct Edge {
    Vec2l from{};
    Vec2l to{};
    int64_t len{};

    bool operator==(const Edge &other) const noexcept {
        return from == other.from and to == other.to and len == other.len;
    }
};

template <> struct std::hash<Edge> {
    constexpr std::size_t operator()(const Edge &e) const noexcept {
        return std::hash<Vec2l>{}(e.from) * 2237ull + std::hash<Vec2l>{}(e.to) * 4294967701ull +
               std::hash<int64_t>{}(e.len) * 9007199254741921ull;
    }
};

struct Island {
    Grid<char> hikingMap{};
    Grid<int8_t> hikingArea{};
    std::unordered_set<Vec2l> vertices{};
    std::unordered_set<Edge> edges{}; // unsorted
    std::vector<Vec2l> sortedVertices{};

    const Vec2l startHiking{};
    const Vec2l endHiking{};

    void findVertices() {
        for (const auto y : iota(0, hikingMap.height)) {
            for (const auto x : iota(0, hikingMap.width)) {
                if (hikingMap[x, y] != '#' and hikingMap[x, y] != '.') {
                    vertices.emplace(x, y);
                }
            }
        }
    }

    void findEdges() {
        for (const auto &start : vertices) {
            findPaths(start);
        }
    }

    void findPaths(const Vec2l &startPos) {
        Vec2l pos{};
        switch (hikingMap[startPos]) {
        case '>':
            pos = startPos + neighbours4[0];
            break;
        case 'S':
        case 'v':
            pos = startPos + neighbours4[3];
            break;
        case 'E':
            return;
        default:
            fmt::print("from slope not implemented\n");
            throw std::runtime_error("invalid from slope");
        }

        std::queue<Edge> frontier{};
        frontier.emplace(startPos, pos, 1);

        while (!frontier.empty()) {
            const auto [prevPos, curPos, steps] = frontier.front();
            frontier.pop();
            if (edges.contains({startPos, curPos, steps})) [[unlikely]] {
                fmt::print("loop found\n");
                throw std::runtime_error("loop");
            }
            switch (hikingMap[curPos]) {
            case 'E':
                edges.emplace(startPos, curPos, steps);
                break;
            case '>':
                if (curPos - prevPos == neighbours4[0]) {
                    edges.emplace(startPos, curPos, steps);
                }
                break;
            case 'v':
                if (curPos - prevPos == neighbours4[3]) {
                    edges.emplace(startPos, curPos, steps);
                }
                break;
            case '.':
                for (const auto &dir : neighbours4) {
                    const auto nextPos = curPos + dir;
                    if (nextPos != prevPos) {
                        if (hikingMap[nextPos] != '#') {
                            frontier.emplace(curPos, nextPos, steps + 1);
                        }
                    }
                }
                break;
            default:
                fmt::print("to slope '{}' at {} not implemented\n", hikingMap[curPos], curPos);
                throw std::runtime_error("invalid to slope");
            }
        }
    }

    void sortVertices() {
        // L <- Empty list that will contain the sorted elements
        // std::vector<Vec2l> sortedVertices{};
        // S <- Set of all nodes with no incoming edge
        std::queue<Vec2l> frontier{};
        std::unordered_set<Edge> visitedEdges{};
        frontier.emplace(startHiking);

        // while S is not empty do
        while (!frontier.empty()) {
            // remove a node n from S
            const auto current = frontier.front();
            frontier.pop();
            // add n to L
            sortedVertices.push_back(current);
            // for each node m with an edge e from n to m do
            for (const auto &edge : edges) {
                if (!visitedEdges.contains(edge) and edge.from == current) {
                    // remove edge e from the graph
                    visitedEdges.insert(edge);
                    // if m has no other incoming edges then
                    bool hasIncomingEdge = false;
                    for (const auto &incomingEdge : edges) {
                        if (!visitedEdges.contains(incomingEdge) and incomingEdge.to == edge.to) {
                            hasIncomingEdge = true;
                            break;
                        }
                    }
                    if (!hasIncomingEdge) {
                        // insert m into S
                        frontier.push(edge.to);
                    }
                }
            }
        }
        if (edges.size() != visitedEdges.size()) {
            fmt::print("error: graph has at least one cycle\n");
        }
    }

    void findLongestPath() const {
        std::unordered_map<Vec2l, int64_t> distances{};
        for (const auto &vertice : sortedVertices) {
            distances[vertice] = 0;
        }

        for (const auto &vertice : sortedVertices) {
            if (distances[vertice] == 0) {
                fmt::print("not yet visited: {}\n", vertice);
            }
            for (const auto &edge : edges) {
                if (edge.from == vertice) {
                    distances[edge.to] =
                        std::max(distances[edge.to], distances[vertice] + edge.len);
                }
            }
        }

        for (const auto &vertice : sortedVertices) {
            fmt::print("Longest path to ({}, {})vi is {}\n", vertice.y + 1, vertice.x + 1,
                       distances[vertice]);
        }

        fmt::print("The longest path is {}\n", distances[endHiking]);
    }

    Island(const char *file)
        : hikingMap(file, '#'), hikingArea(hikingMap.width, hikingMap.height, -1),
          startHiking(1, 0), endHiking(hikingMap.width - 2, hikingMap.height - 1) {
        hikingMap[startHiking] = 'S';
        hikingMap[endHiking] = 'E';
        findVertices();
        findEdges();
        sortVertices();
        findLongestPath();

        for (const auto &e : edges) {
            fmt::print("Edge {} -> {} size {}\n", e.from, e.to, e.len);
        }
        for (const auto &v : sortedVertices) {
            fmt::print("Vertice {}\n", v);
        }
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Island snow{argv[1]};
}
