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

struct Vertice {
    Vec2l position{};
    // directed edges
    std::unordered_set<Edge> edgeFrom{};
    std::unordered_set<Edge> edgeTo{};
    // undirected edges
    std::unordered_set<Edge> unEdgeFrom{};
    std::unordered_set<Edge> unEdgeTo{};

    bool operator==(const Vertice &other) const noexcept { return position == other.position; }
};

template <> struct std::hash<Vertice> {
    constexpr std::size_t operator()(const Vertice &v) const noexcept {
        return std::hash<Vec2l>{}(v.position);
    }
};

struct Island {
    Grid<char> hikingMap{};
    Grid<int8_t> hikingArea{};
    std::unordered_map<Vec2l, Vertice> vertices{};
    std::vector<Vec2l> sortedVertices{};

    const Vec2l startHiking{};
    const Vec2l endHiking{};

    void findVertices() {
        for (const auto y : iota(0, hikingMap.height)) {
            for (const auto x : iota(0, hikingMap.width)) {
                if (hikingMap[x, y] != '#' and hikingMap[x, y] != '.') {
                    vertices[{x, y}] = Vertice{Vec2l{x, y}};
                }
            }
        }
    }

    void findEdges() {
        for (const auto &[start, vertice_] : vertices) {
            findDirectedPaths(start);
            findUndirectedPaths(start);
        }
    }

    void findDirectedPaths(const Vec2l &startPos) {
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
        int64_t edgeCount = 0;

        while (!frontier.empty()) {
            const auto [prevPos, curPos, steps] = frontier.front();
            frontier.pop();

            switch (hikingMap[curPos]) {
            case 'E':
                vertices[startPos].edgeTo.emplace(startPos, curPos, steps);
                vertices[curPos].edgeFrom.emplace(startPos, curPos, steps);
                ++edgeCount;
                break;
            case '>':
                if (curPos - prevPos == neighbours4[0]) {
                    vertices[startPos].edgeTo.emplace(startPos, curPos, steps);
                    vertices[curPos].edgeFrom.emplace(startPos, curPos, steps);
                    ++edgeCount;
                }
                break;
            case 'v':
                if (curPos - prevPos == neighbours4[3]) {
                    vertices[startPos].edgeTo.emplace(startPos, curPos, steps);
                    vertices[curPos].edgeFrom.emplace(startPos, curPos, steps);
                    ++edgeCount;
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
        if (edgeCount > 2) {
            fmt::print("Vertice {} has {} edges.\n", startPos, edgeCount);
        }
    }

    void findUndirectedPaths(const Vec2l &startPos) {
        std::queue<Edge> frontier{};
        frontier.emplace(startPos, startPos, 0);
        int64_t edgeCount = 0;

        while (!frontier.empty()) {
            const auto [prevPos, curPos, steps] = frontier.front();
            frontier.pop();

            switch (curPos == startPos ? '.' : hikingMap[curPos]) {
            case 'S':
            case 'E':
            case '>':
            case 'v':
                vertices[startPos].unEdgeTo.emplace(startPos, curPos, steps);
                vertices[curPos].unEdgeFrom.emplace(startPos, curPos, steps);
                ++edgeCount;
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
        if (edgeCount > 4) {
            fmt::print("Vertice {} has {} undirected edges.\n", startPos, edgeCount);
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
            for (const auto &edge : vertices[current].edgeTo) {
                if (!visitedEdges.contains(edge)) {
                    // remove edge e from the graph
                    visitedEdges.insert(edge);
                    // if m has no other incoming edges then
                    bool hasIncomingEdge = false;
                    for (const auto &incomingEdge : vertices[edge.to].edgeFrom) {
                        if (!visitedEdges.contains(incomingEdge)) {
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
    }

    int64_t findLongestPath() const {
        std::unordered_map<Vec2l, int64_t> distances{};
        for (const auto &vertice : sortedVertices) {
            distances[vertice] = 0;
        }

        for (const auto &vertice : sortedVertices) {
            for (const auto &edge : vertices.at(vertice).edgeTo) {
                distances[edge.to] = std::max(distances[edge.to], distances[vertice] + edge.len);
            }
        }

        return distances[endHiking];
    }

    int64_t findUpslopePath() const {
        std::unordered_set<Vec2l> visited{};
        return dfs2(startHiking, visited);
    }

    int64_t dfs2(const Vec2l &position, std::unordered_set<Vec2l> &visited) const {
        int64_t result = 0;
        for (const auto &edge : vertices.at(position).unEdgeTo) {
            const auto &nextPos = edge.to;
            if (nextPos == endHiking) {
                result = std::max(result, edge.len);
            } else if (!visited.contains(nextPos)) {
                visited.insert(nextPos);
                result = std::max(result, dfs2(edge.to, visited) + edge.len);
                visited.erase(nextPos);
            }
        }
        return result;
    }

    Island(const char *file)
        : hikingMap(file, '#'), hikingArea(hikingMap.width, hikingMap.height, -1),
          startHiking(1, 0), endHiking(hikingMap.width - 2, hikingMap.height - 1) {
        hikingMap[startHiking] = 'S';
        hikingMap[endHiking] = 'E';
        findVertices();
        findEdges();
        sortVertices();
        fmt::print("The graph has {} vertices\n", vertices.size());
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Island snowIsland{argv[1]};

    fmt::print("The longest path has {} steps\n", snowIsland.findLongestPath());
}
