/* Elven Truck Simulator */
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <stack>
#include <string>
#include <vector>

#include "simpleparser.hpp"
#include "vec2.hpp"

using std::views::iota;
using std::views::reverse; // for iota(0,10) | reverse

constexpr static std::string rType = "xmas";

struct Part {
    std::array<int64_t, 4> r{};

    Part(SimpleParser &scan) {
        scan.skipToken("{x=");
        r[0] = scan.getInt64();
        scan.skipToken(",m=");
        r[1] = scan.getInt64();
        scan.skipToken(",a=");
        r[2] = scan.getInt64();
        scan.skipToken(",s=");
        r[3] = scan.getInt64();
        scan.skipChar('}');
    }
    int64_t rating() const { return r[0] + r[1] + r[2] + r[3]; }
};

struct PartRange {
    std::string workflow{"in"};
    std::array<Vec2l, 4> r = {{{1, 4000}, {1, 4000}, {1, 4000}, {1, 4000}}};

    int64_t totalParts() const {
        int64_t prod = 1;
        for (const auto &p : r) {
            if (p.x <= p.y) {
                prod *= p.y - p.x + 1;
            } else {
                return 0;
            }
        }
        return prod;
    }
};

struct Rule {
    size_t param;
    char cond;
    int64_t value;
    std::string dest;

    bool accept(const Part &part) const {
        int64_t pval = part.r[param];
        switch (cond) {
        case '<':
            return pval < value;
        case '>':
            return pval > value;
        default:
            return true;
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
                rules.emplace_back(0, '-', 0, dest);
                scan.skipChar('}');
                return; // abort here
            }
            scan.skipChar(cond);
            const auto value = scan.getInt64();
            scan.skipChar(':');
            const auto dest = scan.getAlNum();
            rules.emplace_back(rType.find(param), cond, value, dest);
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

    std::vector<PartRange> process(PartRange current) const {
        std::vector<PartRange> result{};
        for (const auto &rule : rules) {
            auto temp = current;
            temp.workflow = rule.dest;
            const auto value = rule.value;
            switch (rule.cond) {
            case '<':
                temp.r[rule.param].y = value - 1;
                result.push_back(temp);
                current.r[rule.param].x = value;
                break;
            case '>':
                temp.r[rule.param].x = value + 1;
                result.push_back(temp);
                current.r[rule.param].y = value;
                break;
            default:
                result.push_back(temp);
                break;
            }
        }
        return result;
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

    int64_t totalParts = 0;
    std::stack<PartRange> processing{};
    processing.emplace(); // put all parts here
    while (!processing.empty()) {
        const auto current = processing.top();
        processing.pop();

        for (const auto &result : workflows[current.workflow].process(current)) {
            const auto partCount = result.totalParts();
            if (result.workflow == "A") {
                totalParts += partCount;
            } else if (partCount > 0) {
                processing.push(result);
            }
        }
    }
    fmt::print("There are {} accepted parts\n", totalParts);
}
