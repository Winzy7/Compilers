/* Read from a YAML file of COOL tokens and return one token at time to simulate
 * output from a lexer*/
#include "cool-parse.h"
#include "cool-yaml.h"
#include "stringtab.h"
#include "utilities.h"
#define yylval  cool_yylval
#define YYEOF   0
#define YYerror 256

extern int curr_lineno;
extern FILE *fin;
extern char *curr_filename;

int yy_flex_debug;

bool node_to_token(ryml::ConstNodeRef node, token &tok, const size_t pos) {
  if (!c4::atou<unsigned int>(node["lineno"].val(), &tok.lineno)) {
    cerr << "Invalid lineno at token #" << pos << "; expected an unsigned number, got "
         << node["lineno"].val() << endl;
    return false;
  }
  curr_lineno = tok.lineno;

  std::string str_kind = std::string(node["kind"].val().str, node["kind"].val().len);
  auto c = string_to_cool_token.find(str_kind);
  if (c == string_to_cool_token.end()) {
    cerr << "Invalid kind at token #" << pos << "; expected a kind, got " << str_kind << endl;
    return false;
  }
  tok.kind = c->second;
  if (node.has_child("symbol")) {
    tok.symbol = std::string(node["symbol"].val().str, node["symbol"].val().len);
  }
  if (node.has_child("boolean")) {
    tok.boolean = node["boolean"].val() == "true";
  }
  return true;
}

void populate_tables_from_token(const token tok) {
  switch (tok.kind) {
  case INT_CONST: yylval.symbol = inttable.add_string(tok.symbol); break;
  case STR_CONST: yylval.symbol = stringtable.add_string(get_unescaped_string(tok.symbol)); break;
  case TYPEID:
  case OBJECTID: yylval.symbol = idtable.add_string(tok.symbol); break;
  case BOOL_CONST: yylval.boolean = tok.boolean; break;
  default: break;
  }
}

int cool_yylex(void) {
  // These variables are only used internally within this function's scope
  static string filename;
  static string content;
  static int init = 0;
  static size_t pos = 0;
  static ryml::ConstNodeRef cur_token;
  static ryml::Tree token_root;

  if (init == 0) {
    char buf[512];
    int result;
    while ((result = fread(buf, 1, sizeof(buf), fin)) > 0) {
      content.append(buf, result);
    }
    token_root = ryml::parse_in_place(c4::to_substr(content));
    if (!token_root.is_map(token_root.root_id())) {
      cerr << "Failed to parse the input; expected a YAML token stream." << endl;
      return YYEOF;
    }
    filename = std::string(token_root["name"].val().str, token_root["name"].val().len);
    curr_filename = &filename[0];
    init = 1;
  }

  // Reached the end of the token stream
  if (pos == token_root["tokens"].num_children()) {
    return YYEOF;
  }

  // Try to convert the node to a token object
  token tok;
  if (!node_to_token(token_root["tokens"][pos], tok, pos)) {
    pos++;
    return YYerror;
  }
  pos++;
  // Fill up the tables so that the parser can access them
  populate_tables_from_token(tok);

  return tok.kind;
}
