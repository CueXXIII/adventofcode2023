#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

struct Card {
    char card;

    operator int64_t() const {
        if (std::isdigit(card)) {
            return card - '0';
        }
        switch (card) {
        case 'A':
            return 14;
        case 'K':
            return 13;
        case 'Q':
            return 12;
        case 'J':
            return 1;
        case 'T':
            return 10;
        }
        return -1;
    }

    auto operator<=>(const Card &other) const {
        return static_cast<int64_t>(*this) <=> static_cast<int64_t>(other);
    }
};

enum HandType { High, OnePair, TwoPair, Three, FullHouse, Four, Five };

struct Hand {
    std::array<Card, 5> cards{};
    std::string cardStr;
    int64_t bid{};
    HandType type{};

    HandType getType() const {
        std::array<int, 15> count{};
        int joker = 0;
        for (const auto &card : cards) {
            if (card.card == 'J') {
                ++joker;
            } else {
                count[card] += 1;
            }
        }
        std::sort(count.begin(), count.end(), std::greater());
        count[0] += joker;
        switch (count[0]) {
        case 5:
            return Five;
        case 4:
            return Four;
        case 3:
            if (count[1] == 2)
                return FullHouse;
            return Three;
        case 2:
            if (count[1] == 2)
                return TwoPair;
            return OnePair;
        case 1:
            return High;
        default:
            fmt::print("Can't find type of hand for {}\n", cardStr);
            throw;
        }
    }

    Hand(const std::string &draw, const int64_t bid) : cardStr(draw), bid(bid) {
        if (draw.size() != 5) {
            fmt::print("Invalid hand '{}'\n", draw);
            throw;
        }
        for (const auto i : iota(0u, 5u)) {
            cards[i] = {draw[i]};
        }
        type = getType();
    }

    auto operator<=>(const Hand &other) const {
        return std::pair(type, cards) <=> std::pair(other.type, other.cards);
    }

    operator std::string() const { return cardStr; }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<Hand> listOfHands{};

    SimpleParser scanner{argv[1]};
    while (!scanner.isEof()) {
        const auto cards = scanner.getToken();
        const auto value = scanner.getInt64();
        listOfHands.emplace_back(cards, value);
    }

    std::sort(listOfHands.begin(), listOfHands.end());

    int64_t total1 = 0;
    int64_t rank = 1;
    for (const auto &hand : listOfHands) {
        total1 += hand.bid * rank;
        ++rank;
    }
    fmt::print("The total winnings are {} cards\n", total1);
}
