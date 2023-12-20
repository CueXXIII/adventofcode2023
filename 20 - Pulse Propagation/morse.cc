#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;
using std::views::reverse;

constexpr static std::array<std::array<std::string, 14>, 4> subgraphs = {
    {{"km", "dr", "kg", "lv", "jc", "qr", "dk", "vj", "ps", "xf", "bd", "gg", "tp", "db"},
     {"lr", "vg", "lf", "nb", "cg", "hx", "sb", "cx", "gp", "sj", "rm", "st", "vd", "tf"},
     {"xh", "ql", "zx", "rq", "gr", "mn", "jh", "lm", "tr", "vp", "lp", "jt", "bk", "ln"},
     {"rf", "dj", "gc", "cm", "rg", "sd", "jx", "cn", "mv", "hq", "fl", "sk", "pt", "vq"}}};
constexpr static std::string finalModule = "tg";

std::array<std::map<std::string, int64_t>, 4> substates{};
int64_t buttonPresses = 0;
std::array<bool, 4> finalHigh{};
std::array<int64_t, 4> subloop{};

enum Pulse { low, high, none };

const std::string pulseName(const Pulse &p) {
    switch (p) {
    case low:
        return "low";
    case high:
        return "high";
    case none:
        return "none";
    default:
        return "INV";
    }
}

struct Module {
    std::string name; // broadcaster name is "roadcaster"
    char type;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

    bool stateOn{false};
    std::map<std::string, Pulse> recentPulses{};

    Module() = default;
    Module(SimpleParser &scan) {
        type = scan.getChar();
        name = scan.getToken();
        scan.skipToken("->");
        do {
            outputs.push_back(scan.getAlNum());
        } while (scan.skipChar(','));
    }

    void addInput(const std::string &in) {
        inputs.push_back(in);
        recentPulses[in] = low;
    }

    Pulse pulse(const std ::string &in, const Pulse pulse) {
        if (pulse == none) {
            fmt::print("none pulse sent to {}!", name);
            return none;
        }
        switch (type) {
        case 'b':
            return pulse;
        case '%':
            if (pulse == high) {
                return none;
            }
            stateOn = !stateOn;
            if (stateOn) {
                return high;
            } else {
                return low;
            }
        case '&': {
            // debug pulses sent to dest
            Pulse result = low;
            recentPulses[in] = pulse;
            for (const auto &[name, last] : recentPulses) {
                if (last == low) {
                    result = high;
                }
            }
            if (name == finalModule) [[unlikely]] {
                for (const auto num : iota(0, 4)) {
                    if (recentPulses.at(subgraphs[num][13]) == high) {
                        finalHigh[num] = true;
                    }
                }
            }
            return result;
        }
        default:
            fmt::print("Invalid type {} on '{}'\n", type, name);
            return none;
        }
    }

    std::string state() const {
        switch (type) {
        case '%':
            return {stateOn ? "h" : "l"};
        case '&': {
            std::string result{'['};
            for (const auto &[_, p] : recentPulses) {
                result += "lh-"[p];
            }
            return result + ']';
        }
        default:
            return "?";
        }
    }
};

struct Machine {
    std::map<std::string, Module> modules;
    std::queue<std::tuple<std::string, std::string, Pulse>> pulses;
    int64_t countLow = 0;
    int64_t countHigh = 0;

    Machine(SimpleParser &scan) {
        while (!scan.isEof()) {
            Module m{scan};
            modules[m.name] = m;
        }
        // connect inputs
        for (const auto &[name, mod] : modules) {
            for (const auto &out : modules[name].outputs) {
                if (modules.contains(out)) {
                    modules[out].addInput(name);
                } else {
                    fmt::print("Dangling connection {} -> {}\n", name, out);
                }
            }
        }
    }

    void push(const bool debug = false) {
        // button pulse
        ++countLow;
        pulses.emplace("button", "roadcaster", low);
        if (debug) [[unlikely]] {
            fmt::print("button -low-> broadcaster\n");
        }

        while (!pulses.empty()) {
            const auto [from, here, pulse] = pulses.front();
            pulses.pop();

            if (!modules.contains(here)) {
                if (pulse == low) {
                    fmt::print("==>{} received {} from {} on button press {}\n", here,
                               pulseName(pulse), from, buttonPresses);
                }
            } else {
                auto &currentModule = modules[here];
                const auto next = currentModule.pulse(from, pulse);
                if (next != none) {
                    for (const auto &dest : currentModule.outputs) {
                        if (next == low) {
                            ++countLow;
                        } else if (next == high) {
                            ++countHigh;
                        }
                        pulses.emplace(here, dest, next);

                        if (debug) [[unlikely]] {
                            fmt::print("{}{} -{}-> {}\n", currentModule.type, currentModule.name,
                                       pulseName(next), dest);
                        }
                    }
                }
            }
        }
        if (debug) [[unlikely]] {
            fmt::print("\n");
        }
    }

    std::string state(size_t subgraph = 0) const {
        std::string result{};
        for (const auto &name : subgraphs[subgraph]) {
            if (modules.contains(name)) {
                result += modules.at(name).state();
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

    SimpleParser scan{argv[1]};
    Machine m{scan};
    int64_t loopSize = 0;
    for (buttonPresses = 1;; ++buttonPresses) {
        if (m.modules.contains(finalModule)) {
            for (const auto num : iota(0, 4)) {
                const auto state = m.state(num);
                if (substates[num].contains(state)) {
                    if (subloop[num] == 0) {
                        fmt::print("Found loop on SG-{} length {} on {}th button press\n", num,
                                   buttonPresses - 1 - substates[num][state], buttonPresses - 1);
                        subloop[num] = buttonPresses - 1 - substates[num][state];
                    }
                } else {
                    substates[num][state] = buttonPresses - 1;
                }
            }
        }
        finalHigh.fill(false);
        m.push();

        if (buttonPresses == 1000) {
            fmt::print("The elves calculate {} * {} = {}\n\n", m.countLow, m.countHigh,
                       m.countLow * m.countHigh);
            if (!m.modules.contains(finalModule)) {
                break;
            }
        }

        for (const auto num : iota(0, 4)) {
            if (finalHigh[num] == true) {
                fmt::print("Module \"{}\" received high from \"{}\" on button press {}\n",
                           finalModule, subgraphs[num][13], buttonPresses);
            }
        }

        loopSize = std::ranges::fold_left(subloop, 1, std::multiplies<int64_t>());
        if (loopSize > 0) {
            fmt::print("Detected a total loop size of {}\n", loopSize);
            break;
        }
    }

    // clear finalModule
    m.push();
    m.push();
    m.push();

    // TODO: set state to loopSize - 1 and press button
}
