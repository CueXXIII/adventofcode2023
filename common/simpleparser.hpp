#pragma once

#include <cctype>
#include <fstream>
#include <string>

class SimpleParser {
    std::ifstream localStream{};
    std::ifstream &in;

    // buffer will always contain at least 1 char, or eof==true
    std::string buffer;
    size_t pos;
    bool eof;
    bool eol;

    void bufferSaturate() {
        while (!eof && pos >= buffer.size()) {
            eof = !std::getline(in, buffer);
            eol = true;
            pos = 0;
        }
    }

    char bufferPeek() {
        if (eof) {
            return -1;
        }
        return buffer[pos];
    }

    void bufferNextChar() {
        if (!eof) {
            eol = false;
            pos++;
            bufferSaturate();
        }
    }

    char bufferGet() {
        const auto c = bufferPeek();
        bufferNextChar();
        return c;
    }

  public:
    SimpleParser(std::ifstream &);
    SimpleParser(const char *);

    bool isEof() const;
    bool isEol() const;

    int64_t getInt64();
    std::string getToken(const char terminate = '\0');
    std::string getAlNum();
    std::string getLine();

    char getChar();
    char peekChar();

    void skipWhitespace();
    bool skipChar(const char);
    bool skipToken(const std::string &);
};

SimpleParser::SimpleParser(std::ifstream &stream)
    : in(stream), buffer(""), pos(0), eof(false), eol(false) {
    bufferSaturate();
    eol = false;
}

SimpleParser::SimpleParser(const char *infile)
    : localStream(std::ifstream(infile)), in(localStream), buffer(""), pos(0), eof(false),
      eol(false) {
    bufferSaturate();
    eol = false;
}

int64_t SimpleParser::getInt64() {
    skipWhitespace();
    size_t processed;
    const int64_t value = std::stoll(buffer.substr(pos), &processed);
    eol = false;
    pos += processed;
    bufferSaturate();
    return value;
}

std::string SimpleParser::getToken(const char terminate) {
    skipWhitespace();
    auto end = pos;
    while (end < buffer.size() && !std::isspace(buffer[end]) && buffer[end] != terminate) {
        ++end;
    }
    auto token = buffer.substr(pos, end - pos);
    eol = false;
    pos = end;
    bufferSaturate();
    return token;
}

std::string SimpleParser::getAlNum() {
    skipWhitespace();
    auto end = pos;
    while (end < buffer.size() && std::isalnum(buffer[end])) {
        ++end;
    }
    auto token = buffer.substr(pos, end - pos);
    eol = false;
    pos = end;
    bufferSaturate();
    return token;
}

std::string SimpleParser::getLine() {
    skipWhitespace();
    auto end = buffer.size();
    auto token = buffer.substr(pos, end - pos);
    pos = end;
    bufferSaturate();
    return token;
}

bool SimpleParser::isEof() const { return eof; }

bool SimpleParser::isEol() const { return eol; }

char SimpleParser::getChar() {
    skipWhitespace();
    if (!eof) {
        const auto c = buffer[pos];
        bufferNextChar();
        return c;
    }
    return -1;
}

char SimpleParser::peekChar() {
    skipWhitespace();
    if (!eof) {
        return buffer[pos];
    }
    return -1;
}

void SimpleParser::skipWhitespace() {
    while (!eof && std::isspace(buffer[pos])) {
        bufferNextChar();
    }
}

bool SimpleParser::skipChar(const char c) {
    skipWhitespace();
    if (!eof && buffer[pos] == c) {
        bufferNextChar();
        return true;
    }
    return false;
}

// If token follows in the input, skip over it and return true
bool SimpleParser::skipToken(const std::string &token) {
    skipWhitespace();
    if (buffer.find(token.data(), pos, token.size()) == pos) {
        pos += token.size();
        bufferSaturate();
        return true;
    }
    return false;
}
