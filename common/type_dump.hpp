#pragma once

// type debugging: get the type from the compiler error
// found on https://stackoverflow.com/questions/38820579/using-auto-type-deducti
template <typename T> struct TypeDump;

#define DumpType(var) TypeDump<decltype(var)> typedump_##var;
#define DumpMemberType(var) TypeDump<decltype(var)> typedump;
