/* Elven Truck Simulator */
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;
using std::views::reverse; // for iota(0,10) | reverse

struct Part {
    int64_t x, m, a, s;

    Part(SimpleParser &scan) {
        scan.skipToken("{x=");
        x = scan.getInt64();
        scan.skipToken(",m=");
        m = scan.getInt64();
        scan.skipToken(",a=");
        a = scan.getInt64();
        scan.skipToken(",s=");
        s = scan.getInt64();
        scan.skipChar('}');
    }
    int64_t rating() const { return x + m + a + s; }
};

struct Rule {
    char param;
    char cond;
    int64_t value;
    std::string dest;

    bool accept(const Part &part) const {
        int64_t pval = 0;
        switch (param) {
        case 'x':
            pval = part.x;
            break;
        case 'm':
            pval = part.m;
            break;
        case 'a':
            pval = part.a;
            break;
        case 's':
            pval = part.s;
            break;
        default:
            return true;
        }
        switch (cond) {
        case '<':
            return pval < value;
        case '>':
            return pval > value;
        }
        fmt::print("Bad Rule ({}{}{} -> {})\n", param, cond, value, dest);
        return false;
    }
};

struct Workflow {
    std::string name;
    std::vector<Rule> rules;

    Workflow() = default;
    Workflow(SimpleParser &scan) {
        name = scan.getAlNum();
        scan.skipChar('{');
        while (!scan.skipChar('}')) {
            const auto param = scan.getChar();
            const auto cond = scan.peekChar();
            if (cond != '>' and cond != '<') {
                std::string dest{param};
                if (scan.peekChar() != '}') {
                    dest += scan.getAlNum();
                }
                rules.emplace_back('-', '-', 0, dest);
                scan.skipChar('}');
                return; // abort here
            }
            scan.skipChar(cond);
            const auto value = scan.getInt64();
            scan.skipChar(':');
            const auto dest = scan.getAlNum();
            rules.emplace_back(param, cond, value, dest);
            scan.skipChar(','); // may fail
        }
    }

    std::string process(const Part &part) const {
        for (const auto &rule : rules) {
            if (rule.accept(part)) {
                return rule.dest;
            }
        }
        fmt::print("No matching rule in workflow {}\n", name);
        return "R";
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::map<std::string, Workflow> workflows{};

    SimpleParser scan{argv[1]};
    while (scan.peekChar() != '{') {
        Workflow w{scan};
        workflows[w.name] = w;
    }

    int64_t totalRating = 0;
    while (!scan.isEof()) {
        Part part{scan};
        std::string position = "in";
        // fmt::print("in");
        for (;;) {
            position = workflows[position].process(part);
            // fmt::print(" -> {}", position);
            if (position == "R") {
                break;
            }
            if (position == "A") {
                totalRating += part.rating();
                break;
            }
        }
        // fmt::print("\n");
    }
    fmt::print("Your parts are rated {}\n", totalRating);
}
