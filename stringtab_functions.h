//
// See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//
#include "copyright.h"

#include "cool-io.h"
#define MAXSIZE   1000000
#define min(a, b) (a > b ? b : a)

#include "stringtab.h"
#include <stdio.h>
#include <vector>

//
// A string table is implemented a linked list of Entrys.  Each Entry
// in the list has a unique string.
//

template <class Elem> Elem *StringTable<Elem>::add_string(std::string s) {
  return add_string(s, MAXSIZE);
}

//
// Add a string requires two steps.  First, the list is searched; if the
// string is found, a pointer to the existing Entry for that string is
// returned.  If the string is not found, a new Entry is created and added
// to the list.
//
template <class Elem> Elem *StringTable<Elem>::add_string(std::string s, int maxchars) {
  int len = min((int)s.length(), maxchars);
  for (auto &entry : *tbl)
    if (entry->equal_string(s, len)) return entry;

  Elem *e = new Elem(s, len, index++);
  tbl->push_back(e);
  return e;
}

//
// To look up a string, the list is scanned until a matching Entry is located.
// If no such entry is found, an assertion failure occurs.  Thus, this function
// is used only for strings that one expects to find in the table.
//
template <class Elem> Elem *StringTable<Elem>::lookup_string(std::string s) {
  int len = s.length();
  for (auto &entry : *tbl)
    if (entry->equal_string(s, len)) return entry;
  assert(0);   // fail if string is not found
  return NULL; // to avoid compiler warning
}

//
// lookup is similar to lookup_string, but uses the index of the string
// as the key.
//
template <class Elem> Elem *StringTable<Elem>::lookup(int ind) {
  for (auto &entry : *tbl)
    if (entry->equal_index(ind)) return entry;
  assert(0);   // fail if string is not found
  return NULL; // to avoid compiler warning
}

//
// add_int adds the string representation of an integer to the list.
//
template <class Elem> Elem *StringTable<Elem>::add_int(int i) {
  static char *buf = new char[20];
  snprintf(buf, 20, "%d", i);
  return add_string(buf);
}
template <class Elem> int StringTable<Elem>::first() {
  return 0;
}

template <class Elem> int StringTable<Elem>::more(int i) {
  return i < index;
}

template <class Elem> int StringTable<Elem>::next(int i) {
  assert(i < index);
  return i + 1;
}
