//////////////////////////////////////////////////////////
//
// file: cool-tree.cc
//
// This file defines the functions of each class
//
//////////////////////////////////////////////////////////

#include "ryml_all.hpp"

#include "cool-tree.h"
#include "cool-tree.handcode.h"
#include "tree.h"
#include <type_traits>

// This implements the helper to convert a arbitrary list of AST nodes to a YAML
// sequence. We use a template function so that different phylums are supported.
template <typename phylum, typename = std::enable_if_t<std::is_base_of_v<tree_node, phylum>>>
void list_to_yaml(std::list<phylum *> *tree_nodes, ryml::NodeRef *n) {
  *n |= ryml::SEQ;
  static_assert(std::is_base_of<tree_node, phylum>::value);
  for (auto node = tree_nodes->begin(); node != tree_nodes->end(); ++node) {
    ryml::NodeRef child = n->append_child();
    (*node)->to_yaml(&child);
  }
}

template <typename phylum, typename = std::enable_if_t<std::is_base_of_v<tree_node, phylum>>>
void yaml_to_list(std::list<phylum *> *tree_nodes,
                  phylum *(*yaml_to_treenode)(ryml::ConstNodeRef const &),
                  const ryml::ConstNodeRef &n) {
  if (!n.is_seq()) {
    // This should never be called with a non-seq node.
    // In this case we return leaving tree_nodes unchanged.
    std::cerr << "Unexpected non-sequence from the input yaml" << endl;
    return;
  }
  static_assert(std::is_base_of<tree_node, phylum>::value);
  for (const ryml::ConstNodeRef yaml_node : n.children()) {
    tree_nodes->push_back(yaml_to_treenode(yaml_node));
  }
}

void emit_lineno(tree_node const *t, ryml::NodeRef *n) {
  assert(n->is_map());
  n->append_child() << ryml::key("lineno") << t->get_line_number();
}

void emit_yaml(std::ostream &o, tree_node const *t) {
  ryml::Tree wtree;
  ryml::NodeRef wroot = wtree.rootref();
  t->to_yaml(&wroot);
  o << wroot;
}

Program *parse_yaml(std::istream &in) {
  // Read from the passed filename
  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string content = buffer.str();

  // Parse the entire tree assuming it is valid YAML
  ryml::Tree tree = ryml::parse_in_place(ryml::to_substr(content));
  const ryml::ConstNodeRef root = tree.rootref();

  // Run the serializer on the parsed tree assuming it is an AST
  Program *prog = yaml_to_program(root);

  return prog;
}

// constructors' functions
Program *program_class::copy_Program() {
  return new program_class(classes);
}

void program_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  n->append_child() << ryml::key("class") << "program";
  ryml::NodeRef classes_node = (*n)["classes"];
  list_to_yaml<Class_>(classes, &classes_node);
}

Class_ *class__class::copy_Class_() {
  return new class__class(copy_Symbol(name), copy_Symbol(parent), features, copy_Symbol(filename));
}

void class__class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  n->append_child() << ryml::key("class") << "class_";
  n->append_child() << ryml::key("name") << name->get_string();
  n->append_child() << ryml::key("parent") << parent->get_string();
  ryml::NodeRef features_node = (*n)["features"];
  list_to_yaml<Feature>(features, &features_node);
  n->append_child() << ryml::key("filename") << filename->get_string();
}

Feature *method_class::copy_Feature() {
  return new method_class(copy_Symbol(name),
                          formals,
                          copy_Symbol(return_type),
                          expr->copy_Expression());
}

void method_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  n->append_child() << ryml::key("class") << "method";
  n->append_child() << ryml::key("name") << name->get_string();
  ryml::NodeRef formals_node = (*n)["formals"];
  list_to_yaml<Formal>(formals, &formals_node);
  n->append_child() << ryml::key("return_type") << return_type->get_string();
  ryml::NodeRef expr_node = (*n)["expr"];
  expr->to_yaml(&expr_node);
}

Feature *attr_class::copy_Feature() {
  return new attr_class(copy_Symbol(name), copy_Symbol(type_decl), init->copy_Expression());
}

void attr_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  n->append_child() << ryml::key("class") << "attr";
  n->append_child() << ryml::key("name") << name->get_string();
  n->append_child() << ryml::key("type_decl") << type_decl->get_string();
  ryml::NodeRef init_node = (*n)["init"];
  init->to_yaml(&init_node);
}

Formal *formal_class::copy_Formal() {
  return new formal_class(copy_Symbol(name), copy_Symbol(type_decl));
}

void formal_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  n->append_child() << ryml::key("class") << "formal";
  n->append_child() << ryml::key("name") << name->get_string();
  n->append_child() << ryml::key("type_decl") << type_decl->get_string();
}

Case *branch_class::copy_Case() {
  return new branch_class(copy_Symbol(name), copy_Symbol(type_decl), expr->copy_Expression());
}

void branch_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  n->append_child() << ryml::key("class") << "branch";
  n->append_child() << ryml::key("name") << name->get_string();
  n->append_child() << ryml::key("type_decl") << type_decl->get_string();
  ryml::NodeRef expr_node = (*n)["expr"];
  expr->to_yaml(&expr_node);
}

// emit_type is intended to work for Expression only
// Since it is really relevant to ryml and expects no external usage
// defining it inside the implementation but not on the Expression class
void emit_type(Expression const *e, ryml::NodeRef *n) {
  assert(n->is_map());
  if (e->type) {
    n->append_child() << ryml::key("type") << e->type->get_string();
  } else {
    n->append_child() << ryml::key("type") << "_no_type";
  }
}

Expression *assign_class::copy_Expression() {
  return new assign_class(copy_Symbol(name), expr->copy_Expression());
}

void assign_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "assign";
  n->append_child() << ryml::key("name") << name->get_string();
  ryml::NodeRef expr_node = (*n)["expr"];
  expr->to_yaml(&expr_node);
}

Expression *static_dispatch_class::copy_Expression() {
  return new static_dispatch_class(expr->copy_Expression(),
                                   copy_Symbol(type_name),
                                   copy_Symbol(name),
                                   actual);
}

void static_dispatch_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "static_dispatch";
  ryml::NodeRef expr_node = (*n)["expr"];
  expr->to_yaml(&expr_node);
  n->append_child() << ryml::key("type_name") << type_name->get_string();
  n->append_child() << ryml::key("name") << name->get_string();
  ryml::NodeRef actual_node = (*n)["actual"];
  list_to_yaml<Expression>(actual, &actual_node);
}

Expression *dispatch_class::copy_Expression() {
  return new dispatch_class(expr->copy_Expression(), copy_Symbol(name), actual);
}

void dispatch_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "dispatch";
  ryml::NodeRef expr_node = (*n)["expr"];
  expr->to_yaml(&expr_node);
  n->append_child() << ryml::key("name") << name->get_string();
  ryml::NodeRef actual_node = (*n)["actual"];
  list_to_yaml<Expression>(actual, &actual_node);
}

Expression *cond_class::copy_Expression() {
  return new cond_class(pred->copy_Expression(),
                        then_exp->copy_Expression(),
                        else_exp->copy_Expression());
}

void cond_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "cond";
  ryml::NodeRef pred_node = (*n)["pred"];
  pred->to_yaml(&pred_node);
  ryml::NodeRef then_exp_node = (*n)["then_exp"];
  then_exp->to_yaml(&then_exp_node);
  ryml::NodeRef else_exp_node = (*n)["else_exp"];
  else_exp->to_yaml(&else_exp_node);
}

Expression *loop_class::copy_Expression() {
  return new loop_class(pred->copy_Expression(), body->copy_Expression());
}

void loop_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "loop";
  ryml::NodeRef pred_node = (*n)["pred"];
  pred->to_yaml(&pred_node);
  ryml::NodeRef body_node = (*n)["body"];
  body->to_yaml(&body_node);
}

Expression *typcase_class::copy_Expression() {
  return new typcase_class(expr->copy_Expression(), cases);
}

void typcase_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "typcase";
  ryml::NodeRef expr_node = (*n)["expr"];
  expr->to_yaml(&expr_node);
  ryml::NodeRef cases_node = (*n)["cases"];
  list_to_yaml<Case>(cases, &cases_node);
}

Expression *block_class::copy_Expression() {
  return new block_class(body);
}

void block_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "block";
  ryml::NodeRef body_node = (*n)["body"];
  list_to_yaml<Expression>(body, &body_node);
}

Expression *let_class::copy_Expression() {
  return new let_class(copy_Symbol(identifier),
                       copy_Symbol(type_decl),
                       init->copy_Expression(),
                       body->copy_Expression());
}

void let_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "let";
  n->append_child() << ryml::key("identifier") << identifier->get_string();
  n->append_child() << ryml::key("type_decl") << type_decl->get_string();
  ryml::NodeRef init_node = (*n)["init"];
  init->to_yaml(&init_node);
  ryml::NodeRef body_node = (*n)["body"];
  body->to_yaml(&body_node);
}

Expression *plus_class::copy_Expression() {
  return new plus_class(e1->copy_Expression(), e2->copy_Expression());
}

void plus_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "plus";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
  ryml::NodeRef e2_node = (*n)["e2"];
  e2->to_yaml(&e2_node);
}

Expression *sub_class::copy_Expression() {
  return new sub_class(e1->copy_Expression(), e2->copy_Expression());
}

void sub_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "sub";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
  ryml::NodeRef e2_node = (*n)["e2"];
  e2->to_yaml(&e2_node);
}

Expression *mul_class::copy_Expression() {
  return new mul_class(e1->copy_Expression(), e2->copy_Expression());
}

void mul_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "mul";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
  ryml::NodeRef e2_node = (*n)["e2"];
  e2->to_yaml(&e2_node);
}

Expression *divide_class::copy_Expression() {
  return new divide_class(e1->copy_Expression(), e2->copy_Expression());
}

void divide_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "divide";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
  ryml::NodeRef e2_node = (*n)["e2"];
  e2->to_yaml(&e2_node);
}

Expression *neg_class::copy_Expression() {
  return new neg_class(e1->copy_Expression());
}

void neg_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "neg";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
}

Expression *lt_class::copy_Expression() {
  return new lt_class(e1->copy_Expression(), e2->copy_Expression());
}

void lt_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "lt";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
  ryml::NodeRef e2_node = (*n)["e2"];
  e2->to_yaml(&e2_node);
}

Expression *eq_class::copy_Expression() {
  return new eq_class(e1->copy_Expression(), e2->copy_Expression());
}

void eq_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "eq";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
  ryml::NodeRef e2_node = (*n)["e2"];
  e2->to_yaml(&e2_node);
}

Expression *leq_class::copy_Expression() {
  return new leq_class(e1->copy_Expression(), e2->copy_Expression());
}

void leq_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "leq";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
  ryml::NodeRef e2_node = (*n)["e2"];
  e2->to_yaml(&e2_node);
}

Expression *comp_class::copy_Expression() {
  return new comp_class(e1->copy_Expression());
}

void comp_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "comp";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
}

Expression *int_const_class::copy_Expression() {
  return new int_const_class(copy_Symbol(token));
}

void int_const_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "int_const";
  n->append_child() << ryml::key("token") << token->get_string();
}

Expression *bool_const_class::copy_Expression() {
  return new bool_const_class(copy_Boolean(val));
}

void bool_const_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "bool_const";
  n->append_child() << ryml::key("val") << val;
}

Expression *string_const_class::copy_Expression() {
  return new string_const_class(copy_Symbol(token));
}

void string_const_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "string_const";
  n->append_child() << ryml::key("token") << token->get_string();
}

Expression *new__class::copy_Expression() {
  return new new__class(copy_Symbol(type_name));
}

void new__class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "new_";
  n->append_child() << ryml::key("type_name") << type_name->get_string();
}

Expression *isvoid_class::copy_Expression() {
  return new isvoid_class(e1->copy_Expression());
}

void isvoid_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "isvoid";
  ryml::NodeRef e1_node = (*n)["e1"];
  e1->to_yaml(&e1_node);
}

Expression *no_expr_class::copy_Expression() {
  return new no_expr_class();
}

void no_expr_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "no_expr";
}

Expression *object_class::copy_Expression() {
  return new object_class(copy_Symbol(name));
}

void object_class::to_yaml(ryml::NodeRef *n) const {
  *n |= ryml::MAP;
  emit_lineno(this, n);
  emit_type(this, n);
  n->append_child() << ryml::key("class") << "object";
  n->append_child() << ryml::key("name") << name->get_string();
}

// interfaces used by Bison
Classes nil_Classes() {
  return new std::list<Class_ *>();
}

Classes single_Classes(Class_ *e) {
  return new std::list<Class_ *>(1, e);
}

Classes append_Classes(Classes p1, Classes p2) {
  p1->splice(p1->end(), *p2);
  return p1;
}

Features nil_Features() {
  return new std::list<Feature *>;
}

Features single_Features(Feature *e) {
  return new std::list<Feature *>(1, e);
}

Features append_Features(Features p1, Features p2) {
  p1->splice(p1->end(), *p2);
  return p1;
}

Formals nil_Formals() {
  return new std::list<Formal *>;
}

Formals single_Formals(Formal *e) {
  return new std::list<Formal *>(1, e);
}

Formals append_Formals(Formals p1, Formals p2) {
  p1->splice(p1->end(), *p2);
  return p1;
}

Expressions nil_Expressions() {
  return new std::list<Expression *>;
}

Expressions single_Expressions(Expression *e) {
  return new std::list<Expression *>(1, e);
}

Expressions append_Expressions(Expressions p1, Expressions p2) {
  p1->splice(p1->end(), *p2);
  return p1;
}

Cases nil_Cases() {
  return new std::list<Case *>;
}

Cases single_Cases(Case *e) {
  return new std::list<Case *>(1, e);
}

Cases append_Cases(Cases p1, Cases p2) {
  p1->splice(p1->end(), *p2);
  return p1;
}

Program *program(Classes classes) {
  return new program_class(classes);
}

std::string *get_string_val(ryml::ConstNodeRef const &node) {
  return new std::string(node.val().str, node.val().len);
}

void fail() {
  fprintf(stderr, "failed\n");
  exit(1);
}

extern int node_lineno;

int set_lineno(ryml::ConstNodeRef const &node) {
  int prev_lineno = node_lineno;
  node_lineno = std::stoi(*get_string_val(node["lineno"]));
  return prev_lineno;
}

Program *yaml_to_program(ryml::ConstNodeRef const &node) {
  std::string *tree_node_class = get_string_val(node["class"]);
  int prev_lineno = set_lineno(node);
  Program *program_obj = NULL;
  if (tree_node_class->compare("program") == 0) {
    std::list<Class_ *> *classes = new std::list<Class_ *>();
    yaml_to_list<Class_>(classes, &yaml_to_class_, node["classes"]);
    program_obj = program(classes);
  }
  // We need to restore node_lineno before returning,
  // because the caller does not expect it to be changed in recursive calls.
  node_lineno = prev_lineno;
  if (program_obj == NULL) {
    fail();
  }
  return program_obj;
}

Class_ *yaml_to_class_(ryml::ConstNodeRef const &node) {
  std::string *tree_node_class = get_string_val(node["class"]);
  int prev_lineno = set_lineno(node);
  Class_ *class_obj = NULL;
  if (tree_node_class->compare("class_") == 0) {
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    Symbol parent = idtable.add_string(get_string_val(node["parent"])->c_str());
    std::list<Feature *> *features = new std::list<Feature *>();
    yaml_to_list<Feature>(features, &yaml_to_feature, node["features"]);
    Symbol filename = stringtable.add_string(get_string_val(node["filename"])->c_str());
    class_obj = class_(name, parent, features, filename);
  }
  // We need to restore node_lineno before returning,
  // because the caller does not expect it to be changed in recursive calls.
  node_lineno = prev_lineno;
  if (class_obj == NULL) {
    fail();
  }
  return class_obj;
}

Feature *yaml_to_feature(ryml::ConstNodeRef const &node) {
  std::string *tree_node_class = get_string_val(node["class"]);
  int prev_lineno = set_lineno(node);
  Feature *feature = NULL;
  if (tree_node_class->compare("method") == 0) {
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    std::list<Formal *> *formals = new std::list<Formal *>();
    yaml_to_list<Formal>(formals, &yaml_to_formal, node["formals"]);
    Symbol return_type = idtable.add_string(get_string_val(node["return_type"])->c_str());
    Expression *expr = yaml_to_expression(node["expr"]);
    feature = method(name, formals, return_type, expr);
  } else if (tree_node_class->compare("attr") == 0) {
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    Symbol type_decl = idtable.add_string(get_string_val(node["type_decl"])->c_str());
    Expression *init = yaml_to_expression(node["init"]);
    feature = attr(name, type_decl, init);
  }
  // We need to restore node_lineno before returning,
  // because the caller does not expect it to be changed in recursive calls.
  node_lineno = prev_lineno;
  if (feature == NULL) {
    fail();
  }
  return feature;
}

Formal *yaml_to_formal(ryml::ConstNodeRef const &node) {
  std::string *tree_node_class = get_string_val(node["class"]);
  int prev_lineno = set_lineno(node);
  Formal *formal_obj = NULL;
  if (tree_node_class->compare("formal") == 0) {
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    Symbol type_decl = idtable.add_string(get_string_val(node["type_decl"])->c_str());
    formal_obj = formal(name, type_decl);
  }
  // We need to restore node_lineno before returning,
  // because the caller does not expect it to be changed in recursive calls.
  node_lineno = prev_lineno;
  if (formal_obj == NULL) {
    fail();
  }
  return formal_obj;
}

Case *yaml_to_case(ryml::ConstNodeRef const &node) {
  std::string *tree_node_class = get_string_val(node["class"]);
  int prev_lineno = set_lineno(node);
  Case *branch_obj = NULL;
  if (tree_node_class->compare("branch") == 0) {
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    Symbol type_decl = idtable.add_string(get_string_val(node["type_decl"])->c_str());
    Expression *expr = yaml_to_expression(node["expr"]);
    branch_obj = branch(name, type_decl, expr);
  }
  // We need to restore node_lineno before returning,
  // because the caller does not expect it to be changed in recursive calls.
  node_lineno = prev_lineno;
  if (branch_obj == NULL) {
    fail();
  }
  return branch_obj;
}

Expression *yaml_to_expression(ryml::ConstNodeRef const &node) {
  std::string *tree_node_class = get_string_val(node["class"]);
  int prev_lineno = set_lineno(node);
  Expression *expression = NULL;
  if (tree_node_class->compare("assign") == 0) {
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    Expression *expr = yaml_to_expression(node["expr"]);
    expression = assign(name, expr);
  } else if (tree_node_class->compare("static_dispatch") == 0) {
    Expression *expr = yaml_to_expression(node["expr"]);
    Symbol type_name = idtable.add_string(get_string_val(node["type_name"])->c_str());
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    std::list<Expression *> *actual = new std::list<Expression *>();
    yaml_to_list<Expression>(actual, &yaml_to_expression, node["actual"]);
    expression = static_dispatch(expr, type_name, name, actual);
  } else if (tree_node_class->compare("dispatch") == 0) {
    Expression *expr = yaml_to_expression(node["expr"]);
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    std::list<Expression *> *actual = new std::list<Expression *>();
    yaml_to_list<Expression>(actual, &yaml_to_expression, node["actual"]);
    expression = dispatch(expr, name, actual);
  } else if (tree_node_class->compare("cond") == 0) {
    Expression *pred = yaml_to_expression(node["pred"]);
    Expression *then_exp = yaml_to_expression(node["then_exp"]);
    Expression *else_exp = yaml_to_expression(node["else_exp"]);
    expression = cond(pred, then_exp, else_exp);
  } else if (tree_node_class->compare("loop") == 0) {
    Expression *pred = yaml_to_expression(node["pred"]);
    Expression *body = yaml_to_expression(node["body"]);
    expression = loop(pred, body);
  } else if (tree_node_class->compare("typcase") == 0) {
    Expression *expr = yaml_to_expression(node["expr"]);
    std::list<Case *> *cases = new std::list<Case *>();
    yaml_to_list<Case>(cases, &yaml_to_case, node["cases"]);
    expression = typcase(expr, cases);
  } else if (tree_node_class->compare("block") == 0) {
    std::list<Expression *> *body = new std::list<Expression *>();
    yaml_to_list<Expression>(body, &yaml_to_expression, node["body"]);
    expression = block(body);
  } else if (tree_node_class->compare("let") == 0) {
    Symbol identifier = idtable.add_string(get_string_val(node["identifier"])->c_str());
    Symbol type_decl = idtable.add_string(get_string_val(node["type_decl"])->c_str());
    Expression *init = yaml_to_expression(node["init"]);
    Expression *body = yaml_to_expression(node["body"]);
    expression = let(identifier, type_decl, init, body);
  } else if (tree_node_class->compare("plus") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    Expression *e2 = yaml_to_expression(node["e2"]);
    expression = plus(e1, e2);
  } else if (tree_node_class->compare("sub") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    Expression *e2 = yaml_to_expression(node["e2"]);
    expression = sub(e1, e2);
  } else if (tree_node_class->compare("mul") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    Expression *e2 = yaml_to_expression(node["e2"]);
    expression = mul(e1, e2);
  } else if (tree_node_class->compare("divide") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    Expression *e2 = yaml_to_expression(node["e2"]);
    expression = divide(e1, e2);
  } else if (tree_node_class->compare("neg") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    expression = neg(e1);
  } else if (tree_node_class->compare("lt") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    Expression *e2 = yaml_to_expression(node["e2"]);
    expression = lt(e1, e2);
  } else if (tree_node_class->compare("eq") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    Expression *e2 = yaml_to_expression(node["e2"]);
    expression = eq(e1, e2);
  } else if (tree_node_class->compare("leq") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    Expression *e2 = yaml_to_expression(node["e2"]);
    expression = leq(e1, e2);
  } else if (tree_node_class->compare("comp") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    expression = comp(e1);
  } else if (tree_node_class->compare("int_const") == 0) {
    Symbol token = inttable.add_string(get_string_val(node["token"])->c_str());
    expression = int_const(token);
  } else if (tree_node_class->compare("bool_const") == 0) {
    expression = bool_const(node["val"].val() == "1" ? 1 : 0);
  } else if (tree_node_class->compare("string_const") == 0) {
    Symbol token = stringtable.add_string(get_string_val(node["token"])->c_str());
    expression = string_const(token);
  } else if (tree_node_class->compare("isvoid") == 0) {
    Expression *e1 = yaml_to_expression(node["e1"]);
    expression = isvoid(e1);
  } else if (tree_node_class->compare("new_") == 0) {
    Symbol type_name = idtable.add_string(get_string_val(node["type_name"])->c_str());
    expression = new_(type_name);
  } else if (tree_node_class->compare("no_expr") == 0) {
    expression = no_expr();
  } else if (tree_node_class->compare("object") == 0) {
    Symbol name = idtable.add_string(get_string_val(node["name"])->c_str());
    expression = object(name);
  }
  // We need to restore node_lineno before returning,
  // because the caller does not expect it to be changed in recursive calls.
  node_lineno = prev_lineno;
  if (expression == NULL) {
    std::cerr << "Invalid class: " << tree_node_class << endl;
    fail();
  }
  expression->set_type(idtable.add_string(get_string_val(node["type"])->c_str()));
  return expression;
}

Class_ *class_(Symbol name, Symbol parent, Features features, Symbol filename) {
  return new class__class(name, parent, features, filename);
}

Feature *method(Symbol name, Formals formals, Symbol return_type, Expression *expr) {
  return new method_class(name, formals, return_type, expr);
}

Feature *attr(Symbol name, Symbol type_decl, Expression *init) {
  return new attr_class(name, type_decl, init);
}

Formal *formal(Symbol name, Symbol type_decl) {
  return new formal_class(name, type_decl);
}

Case *branch(Symbol name, Symbol type_decl, Expression *expr) {
  return new branch_class(name, type_decl, expr);
}

Expression *assign(Symbol name, Expression *expr) {
  return new assign_class(name, expr);
}

Expression *static_dispatch(Expression *expr, Symbol type_name, Symbol name, Expressions actual) {
  return new static_dispatch_class(expr, type_name, name, actual);
}

Expression *dispatch(Expression *expr, Symbol name, Expressions actual) {
  return new dispatch_class(expr, name, actual);
}

Expression *cond(Expression *pred, Expression *then_exp, Expression *else_exp) {
  return new cond_class(pred, then_exp, else_exp);
}

Expression *loop(Expression *pred, Expression *body) {
  return new loop_class(pred, body);
}

Expression *typcase(Expression *expr, Cases cases) {
  return new typcase_class(expr, cases);
}

Expression *block(Expressions body) {
  return new block_class(body);
}

Expression *let(Symbol identifier, Symbol type_decl, Expression *init, Expression *body) {
  return new let_class(identifier, type_decl, init, body);
}

Expression *plus(Expression *e1, Expression *e2) {
  return new plus_class(e1, e2);
}

Expression *sub(Expression *e1, Expression *e2) {
  return new sub_class(e1, e2);
}

Expression *mul(Expression *e1, Expression *e2) {
  return new mul_class(e1, e2);
}

Expression *divide(Expression *e1, Expression *e2) {
  return new divide_class(e1, e2);
}

Expression *neg(Expression *e1) {
  return new neg_class(e1);
}

Expression *lt(Expression *e1, Expression *e2) {
  return new lt_class(e1, e2);
}

Expression *eq(Expression *e1, Expression *e2) {
  return new eq_class(e1, e2);
}

Expression *leq(Expression *e1, Expression *e2) {
  return new leq_class(e1, e2);
}

Expression *comp(Expression *e1) {
  return new comp_class(e1);
}

Expression *int_const(Symbol token) {
  return new int_const_class(token);
}

Expression *bool_const(Boolean val) {
  return new bool_const_class(val);
}

Expression *string_const(Symbol token) {
  return new string_const_class(token);
}

Expression *new_(Symbol type_name) {
  return new new__class(type_name);
}

Expression *isvoid(Expression *e1) {
  return new isvoid_class(e1);
}

Expression *no_expr() {
  return new no_expr_class();
}

Expression *object(Symbol name) {
  return new object_class(name);
}
