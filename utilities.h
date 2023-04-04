// -*-Mode: C++;-*-
//
// See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//
#include "copyright.h"

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "cool-io.h"

#include "ryml_all.hpp"

const char *cool_token_to_string(int tok);
void print_cool_token(int tok);
std::string get_escaped_string(std::string s);
std::string get_unescaped_string(std::string s);

#ifdef _COOL_PARSE_H
static const std::unordered_map<std::string, int> string_to_cool_token = {
    {"EOF", 0},
    {"CLASS", CLASS},
    {"ELSE", ELSE},
    {"FI", FI},
    {"IF", IF},
    {"IN", IN},
    {"INHERITS", INHERITS},
    {"LET", LET},
    {"LOOP", LOOP},
    {"POOL", POOL},
    {"THEN", THEN},
    {"WHILE", WHILE},
    {"ASSIGN", ASSIGN},
    {"CASE", CASE},
    {"ESAC", ESAC},
    {"OF", OF},
    {"DARROW", DARROW},
    {"NEW", NEW},
    {"STR_CONST", STR_CONST},
    {"INT_CONST", INT_CONST},
    {"BOOL_CONST", BOOL_CONST},
    {"TYPEID", TYPEID},
    {"OBJECTID", OBJECTID},
    {"ERROR", ERROR},
    {"LE", LE},
    {"NOT", NOT},
    {"ISVOID", ISVOID},
    {"+", '+'},
    {"/", '/'},
    {"-", '-'},
    {"*", '*'},
    {"=", '='},
    {"<", '<'},
    {".", '.'},
    {"~", '~'},
    {",", ','},
    {";", ';'},
    {":", ':'},
    {"(", '('},
    {")", ')'},
    {"@", '@'},
    {"{", '{'},
    {"}", '}'},
};
#endif

#endif
