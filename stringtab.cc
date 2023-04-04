//
// See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//
#include "copyright.h"

#include "stringtab.h"
#include "stringtab_functions.h"
#include <assert.h>

//
// Explicit template instantiations.
// Comment out for versions of g++ prior to 2.7
//
template class StringTable<IdEntry>;
template class StringTable<StringEntry>;
template class StringTable<IntEntry>;

Entry::Entry(std::string s, int l, int i) : str(s), len(l), index(i) {}

int Entry::equal_string(std::string string, int length) const {
  return str.compare(string) == 0;
}

ostream &Entry::print(ostream &s) const {
  return s << "{" << str << ", " << len << ", " << index << "}\n";
}

ostream &operator<<(ostream &s, const Entry &sym) {
  return s << sym.get_string();
}

ostream &operator<<(ostream &s, Symbol sym) {
  return s << *sym;
}

std::string Entry::get_string() const {
  return std::string(str);
}

int Entry::get_len() const {
  return len;
}

// A Symbol is a pointer to an Entry.  Symbols are stored directly
// as nodes of the abstract syntax tree defined by the cool-tree.h.
//
Symbol copy_Symbol(const Symbol s) {
  return s;
}

StringEntry::StringEntry(std::string s, int l, int i) : Entry(s, l, i) {}
IdEntry::IdEntry(std::string s, int l, int i) : Entry(s, l, i) {}
IntEntry::IntEntry(std::string s, int l, int i) : Entry(s, l, i) {}

IdTable idtable;
IntTable inttable;
StrTable stringtable;
