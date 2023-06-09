//
// See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//
#include "copyright.h"

//////////////////////////////////////////////////////////////////////////////
//
//  parser-phase.cc
//
//  Reads a COOL token stream from a file and builds the abstract syntax tree.
//
//////////////////////////////////////////////////////////////////////////////

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "ryml_all.hpp" // needs to be included first

#include "cool-io.h" //includes iostream
#include "cool-parse.h"
#include "cool-tree.h"
#include "cool-tree.handcode.h"
#include <stdio.h>  // for Linux system
#include <unistd.h> // for getopt

//
// These globals keep everything working.
//
FILE *fin = stdin;            // we read from this file
extern Classes parse_results; // list of classes; used for multiple files
extern Program *ast_root;     // the AST produced by the parse

const char *curr_filename = "<stdin>";

extern int omerrs; // a count of lex and parse errors

extern int cool_yyparse();
void handle_flags(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  handle_flags(argc, argv);
  cool_yyparse();
  if (omerrs != 0) {
    cerr << "Compilation halted due to lex and parse errors\n";
    exit(1);
  }
  emit_yaml(std::cout, ast_root);
  return 0;
}
