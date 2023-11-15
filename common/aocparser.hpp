#pragma once

#include <cctype>
#include <cstdint>
#include <fstream>
#include <optional>
#include <string>

class AOCParser {
  public:
    AOCParser(std::ifstream &);
    AOCParser(const char *);

    // configure parser
    void setWhitespace(const std::string &allowed = " \t\n");

    // Get token.
    std::optional<int64_t> getInt64();
    std::optional<uint64_t> getUInt64();
    std::optional<char> getChar();
    std::optional<std::string> getWord(auto isTokenChar = std::isalpha);

    // Try to get token.
    // Does not advance the cursor, but remembers new position.
    std::optional<int64_t> peekInt64();
    std::optional<uint64_t> peekUIn64();
    std::optional<char> peekChar();
    std::optional<std::string> peekWord(auto isTokenChar = std::isalpha);

    // If the previous peek was successful, advance the cursor.
    // No idea what to do on error.
    bool skipPeek();

    // Try to skip this token (and any leading white space).
    // Returns true if successful.
    //
    // Skip any int (may include leading sign).
    bool skipInt();
    // Also check if the integer matches.
    bool skipInt(const int64_t);
    bool skipInt(const uint64_t);
    // Check if the token matches.
    bool skipChar(const char);
    bool skipWord(const std::string &);

    // Line based IO. These advance the cursor after the newline token.

    // Gets the remainder of the line. Leading whitespace is not skipped.
    // May return empty string if at EOL. Returns std::nullopt at EOF.
    std::optional<std::string> getLine();
    // Return true if an empty line follows the current position.
    bool skipEmptyLine();
};
