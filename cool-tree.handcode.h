//
// See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//
#include "copyright.h"

//
// The following include files must come first.

#ifndef COOL_TREE_HANDCODE_H
#define COOL_TREE_HANDCODE_H

#include "cool-phylum.h"
#include "cool.h"
#include "stringtab.h"
#include "tree.h"
#define yylineno curr_lineno
extern int yylineno;

inline Boolean copy_Boolean(Boolean b) {
  return b;
}
inline void assert_Boolean(Boolean) {}

void assert_Symbol(Symbol b);
Symbol copy_Symbol(Symbol b);


typedef std::list<Class_ *> Classes_class;
typedef Classes_class *Classes;

typedef std::list<Feature *> Features_class;
typedef Features_class *Features;

typedef std::list<Formal *> Formals_class;
typedef Formals_class *Formals;

typedef std::list<Expression *> Expressions_class;
typedef Expressions_class *Expressions;

typedef std::list<Case *> Cases_class;
typedef Cases_class *Cases;

#define Program_EXTRAS                            \

#define program_EXTRAS                \

#define Class__EXTRAS                  \
  virtual Symbol get_name() = 0;       \
  virtual Symbol get_parent() = 0;     \
  virtual Symbol get_filename() = 0;

#define class__EXTRAS                          \
  Symbol get_name() { return name; }           \
  Symbol get_parent() { return parent; }       \
  Symbol get_filename() { return filename; }

#define Feature_EXTRAS                                                  \

#define Feature_SHARED_EXTRAS                               \



#define Formal_EXTRAS                                         \

#define formal_EXTRAS                                   \

#define Case_EXTRAS                                                           \

#define branch_EXTRAS                                             \

#define Expression_EXTRAS                                                    \
  Symbol type;                                                               \
  Symbol get_type() { return type; }                                         \
  Expression *set_type(Symbol s) {                                           \
    type = s;                                                                \
    return this;                                                             \
  }                                                                          \
  Expression() { type = (Symbol)NULL; }

#define Expression_SHARED_EXTRAS                                 \


#endif
