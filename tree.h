//
// See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//
#include "copyright.h"

#ifndef TREE_H
#define TREE_H
///////////////////////////////////////////////////////////////////////////
//
// file: tree.h
//
// This file defines the basic class of tree node and list
//
///////////////////////////////////////////////////////////////////////////

#include "cool-io.h"
#include "ryml_all.hpp"
#include "stringtab.h"

/////////////////////////////////////////////////////////////////////
//
//  tree_node
//
//   All APS nodes are derived from tree_node.  There is a
//   protected field:
//       int line_number     line in the source file from which this node came;
//                           this is read from a global variable when the
//                           node is created.
//
//
//
//   The public methods are:
//       tree_node()
//         builds a new tree_node.  The type field is NULL, the
//         line_number is set to the value of the global yylineno.
//
//       void dump(ostream& s,int n);
//         dump is a pretty printer for tree nodes.  The ostream argument
//         is the output stream on which the node is to be printed; n is
//         the number of spaces to indent the output.
//
//       int get_line_number();  return the line number
//       Symbol get_type();      return the type
//
//       tree_node *set(tree_node *t)
//           sets the line number and type of "this" to the values in
//           the argument tree_node.  Returns "this".
//
//
////////////////////////////////////////////////////////////////////////////
class tree_node {
protected:
  int line_number; // stash the line number when node is made
public:
  tree_node();
  virtual tree_node *copy() = 0;
  virtual ~tree_node() {}
  virtual void to_yaml(ryml::NodeRef *n) const = 0;
  int get_line_number() const;
  tree_node *set(tree_node *);
};

///////////////////////////////////////////////////////////////////
//  Lists of APS objects are implemented by the STL List.
//  The STL list contains excellent documentation that you
//  can reference.
//  https://cplusplus.com/reference/list/list/
//
//  List elements have type Elem. The interface is:
//
//     tree_node *copy()
//
//     The "copy" function is for copying an entire APS tree of
//     which a list is just one component (see the definition of copy()
//     in class tree_node).
//
//     Elem nth(std::list<Elem> *l, size_t n);
//     returns the nth element of a list.  If the list has fewer than n
//     elements, an error is generated.
//
//     The list iterator is defined by the STL iterator.
//     If l is a list, a typical use would be:
//
//    std::list<T>::iterator it;
//    for (it = l->begin(); it != l->end(); it++)
//      ... operate on 'it'
//
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
// nth
//
// function to find the nth element of the list
//
///////////////////////////////////////////////////////////////////////////
template <class Elem> Elem nth(std::list<Elem> *l, size_t n) {
  size_t length = l->size();

  if (n > length) {
    cerr << "error: outside the range of the list\n";
    exit(1);
  }

  typename std::list<Elem>::iterator it = l->begin();
  std::advance(it, n);

  return *it;
}

#endif /* TREE_H */
