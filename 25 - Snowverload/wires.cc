#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

using Vertex = int64_t;

std::default_random_engine prng{std::random_device{}()};

struct Id {
    using type = int64_t;
    type lastId{0};
    std::unordered_map<std::string, type> mapping{};
    std::vector<std::string> names{};

    constexpr type get(std::string const &name) {
        if (mapping.contains(name)) {
            return mapping[name];
        } else {
            mapping[name] = ++lastId;
            names.push_back(name);
            return lastId;
        }
    }
    constexpr type get(void) {
        std::string const name = fmt::format("#{}", ++lastId);
        mapping[name] = lastId;
        names.push_back(name);
        return lastId;
    }
    constexpr std::string const &str(type const id) const { return names.at(id - 1); }
    constexpr size_t size() const { return lastId; }

    constexpr void print() const {
        fmt::print("lastId = {}\n", lastId);
        for (auto const &[name, id] : mapping) {
            fmt::print("  \"{}\" -> {}\n", name, id);
        }
    }
};

struct Edge {
    Vertex v1;
    Vertex v2;

    constexpr bool adjacent(Vertex const &v) const { return v1 == v or v2 == v; }
    constexpr bool connects(Vertex const &lhs, Vertex const &rhs) const {
        return (v1 == lhs and v2 == rhs) or (v1 == rhs and v2 == lhs);
    }
    bool operator==(Edge const &other) const {
        return (v1 == other.v1 and v2 == other.v2) or (v1 == other.v2 and v2 == other.v1);
    }
};

template <> struct std::hash<Edge> {
    constexpr std::size_t operator()(Edge const &e) const noexcept {
        return std::hash<Vertex>{}(e.v1) * 3 + std::hash<Vertex>{}(e.v2) * 2642257;
    }
};

struct Graph {
    Id vertexIds{};
    std::vector<Edge> edges{};

    Graph(SimpleParser &scan) {
        while (!scan.isEof()) {
            std::string const src = scan.getToken(':');
            scan.skipChar(':');
            while (!scan.isEol()) {
                std::string const dst = scan.getToken(':');
                edges.emplace_back(vertexIds.get(src), vertexIds.get(dst));
            }
        }
    }

    static Edge const &chooseRandomEdge(std::vector<Edge> const &edges) {
        return edges[std::uniform_int_distribution<size_t>(0, edges.size() - 1)(prng)];
    }

    // Karger's algorithm, constraint to succeed at 3 edges
    bool contract(std::unordered_map<Vertex, int64_t> vertices, std::vector<Edge> edges,
                  int64_t lastVerticeId) const {
        while (vertices.size() > 2) {
            auto const &randomEdge = chooseRandomEdge(edges);

            Vertex newVertex = ++lastVerticeId;
            vertices.emplace(newVertex, vertices.at(randomEdge.v1) + vertices.at(randomEdge.v2));
            vertices.erase(randomEdge.v1);
            vertices.erase(randomEdge.v2);

            std::vector<Edge> newEdges{};
            newEdges.reserve(edges.size() - 1);
            for (Edge const &edge : edges) {
                if (randomEdge.adjacent(edge.v1)) {
                    if (!randomEdge.adjacent(edge.v2)) {
                        newEdges.emplace_back(edge.v2, newVertex);
                    }
                } else if (randomEdge.adjacent(edge.v2)) {
                    newEdges.emplace_back(edge.v1, newVertex);
                } else {
                    newEdges.push_back(edge);
                }
            }
            edges = std::move(newEdges);
        }
        // fmt::print("Left with {} edges, {} vertices\n", edges.size(), vertices.size());
        if (edges.size() <= 3) {
            int64_t prod = 1;
            for (auto const &[name, vertexCount] : vertices) {
                if (std::cmp_less(name, vertexIds.size())) {
                    fmt::print("#{}: {} vertices\n", vertexIds.str(name), vertexCount);
                } else {
                    fmt::print("#{}: {} vertices\n", name, vertexCount);
                }
                prod *= vertexCount;
            }
            fmt::print("Cut wires according to plan {}\n", prod);
        }
        return edges.size() <= 3;
    }

    void find3Cut(bool const benchmark = false) const {
        std::unordered_map<Vertex, int64_t> contractedVertices{};
        for (auto const &vertex : iota(Vertex{1}, static_cast<Vertex>(vertexIds.size() + 1))) {
            contractedVertices[vertex] = 1;
        }
        int64_t success = 0;
        int64_t const benchmark_runs = 10000;
        for (int64_t run = 1; !benchmark || run <= benchmark_runs; ++run) {
            // fmt::print("Try {}: ", run);
            auto const result = contract(contractedVertices, edges, vertexIds.size());
            if (result) {
                ++success;
                if (!benchmark) {
                    return;
                }
            }
        }
        fmt::print("Succeeded {} times in {} tries\n", success, benchmark_runs);
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scan{argv[1]};
    Graph wires{scan};
    wires.find3Cut();
}
