#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

using Vertex = int64_t;

struct Id {
    using type = Vertex;
    type lastId{0};
    std::unordered_map<std::string, type> mapping{};
    std::vector<std::string> names{};

    type get(std::string const &name) {
        if (mapping.contains(name)) {
            return mapping[name];
        } else {
            mapping[name] = ++lastId;
            names.push_back(name);
            return lastId;
        }
    }
    std::string const &str(type const id) { return names[id]; }
    size_t size() const { return lastId; }
};

struct Edge {
    Vertex v1;
    Vertex v2;
};

struct Graph {
    Id vertexIds{};
    std::vector<Edge> edges{};

    Graph(SimpleParser &scan) {
        fmt::print("graph{{\n");
        while (!scan.isEof()) {
            std::string const src = scan.getToken(':');
            scan.skipChar(':');
            while (!scan.isEol()) {
                std::string const dst = scan.getToken(':');
                edges.emplace_back(vertexIds.get(src), vertexIds.get(dst));
                fmt::print("{}--{};\n", src, dst);
            }
        }
        fmt::print("}}\n");
    }

    // in input.dot: between fkx and gcz
    // fkx [pos="24605,882"] gcz [pos="25061,738"]
    // #1 - #707, #708 - #1408
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt> | dot\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scan{argv[1]};
    Graph wires{scan};
}
