//
// See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//
#include "copyright.h"

//////////////////////////////////////////////////////////////////////////////
//
//  utilities.c
//
//  General support code for lexer and parser.
//
//  This file contains:
//      get_escaped_string   print a string showing escape characters
//      print_cool_token       print a cool token and its semantic value
//      dump_cool_token        dump a readable token representation
//      strdup                 duplicate a string (missing from some libraries)
//
///////////////////////////////////////////////////////////////////////////////

#include "cool-io.h"    // for cerr, <<, manipulators
#include "cool-parse.h" // defines tokens
#include "stringtab.h"  // Symbol <-> String conversions
#include <ctype.h>      // for isprint
#include <regex>

#include "utilities.h"

// #define CHECK_TABLES

static const char *padding =
    "                                                                                ";
//   01234567890123456789012345678901234567890123456789012345678901234567890123456789
//             1         2         3         4         5         6         7
// 80 spaces for padding

string get_escaped_string(std::string s) {
  std::ostringstream str;
  for (auto ch = s.begin(); ch != s.end(); ++ch) {
    switch (*ch) {
    case '\\': str << "\\\\"; break;
    case '\"': str << "\\\""; break;
    case '\n': str << "\\n"; break;
    case '\t': str << "\\t"; break;
    case '\b': str << "\\b"; break;
    case '\f': str << "\\f"; break;

    default:
      if (isprint(*ch))
        str << *ch;
      else
        //
        // Unprintable characters are printed using octal equivalents.
        // To get the sign of the octal number correct, the character
        // must be cast to an unsigned char before coverting it to an
        // integer.
        //
        str << '\\' << oct << setfill('0') << setw(3) << (int)((unsigned char)(*ch));
      break;
    }
  }
  return str.str();
}

const std::regex octal_pattern("^[0-3][0-7]{2}$");

string get_unescaped_string(std::string s) {
  std::ostringstream str;
  for (size_t i = 0; i < s.length(); ++i) {
    // The default case is that there is not an escape sequence
    if (s[i] != '\\') {
      str << s[i];
      continue;
    }

    // If the current byte is a backslash, we check for an escape sequence.
    // The backslash is consumed, and the following 1 or 3 bytes are pattern
    // matched and unescaped if needed.
    i++;
    if (i == s.length()) {
      // Any backslash should be followed by at least one byte in a cool string
      // literal.
      cerr << "Unexpected end of string" << endl;
      return "";
    }
    switch (s[i]) {
    case '\\': str << "\\"; break;
    case '\"': str << "\""; break;
    case 'n': str << "\n"; break;
    case 't': str << "\t"; break;
    case 'b': str << "\b"; break;
    case 'f': str << "\f"; break;
    default:
      // Non-printable characters are represented as octal numbers
      // left-padded with leading zeros with a width of 3
      std::string oct_string = s.substr(i, 3);
      // Check if there is a octal escape sequence, and convert it if it is one
      if (!std::regex_match(oct_string, octal_pattern)) {
        // If this catch-all case still not matches a legal escape sequence,
        // give up here and emit an error message
        cerr << "Unexpected escape sequence; expected \\\\, \\\", \\n, \\t, "
                "\\b, \\f, or \\xxx where xxx is an octal number"
             << endl;
        return "";
      }
      unsigned char num = stoi(oct_string, 0, 8);
      // Advance by 3 bytes since an octal number string consumes 4 bytes,
      // while it is only increment by 1 in the for loop
      i += 3;
      str << num;
      break;
    }
  }
  return str.str();
}

//
// The following two functions are used for debugging the parser.
//
const char *cool_token_to_string(int tok) {
  switch (tok) {
  case 0: return ("EOF"); break;
  case (CLASS): return ("CLASS"); break;
  case (ELSE): return ("ELSE"); break;
  case (FI): return ("FI"); break;
  case (IF): return ("IF"); break;
  case (IN): return ("IN"); break;
  case (INHERITS): return ("INHERITS"); break;
  case (LET): return ("LET"); break;
  case (LOOP): return ("LOOP"); break;
  case (POOL): return ("POOL"); break;
  case (THEN): return ("THEN"); break;
  case (WHILE): return ("WHILE"); break;
  case (ASSIGN): return ("ASSIGN"); break;
  case (CASE): return ("CASE"); break;
  case (ESAC): return ("ESAC"); break;
  case (OF): return ("OF"); break;
  case (DARROW): return ("DARROW"); break;
  case (NEW): return ("NEW"); break;
  case (STR_CONST): return ("STR_CONST"); break;
  case (INT_CONST): return ("INT_CONST"); break;
  case (BOOL_CONST): return ("BOOL_CONST"); break;
  case (TYPEID): return ("TYPEID"); break;
  case (OBJECTID): return ("OBJECTID"); break;
  case (ERROR): return ("ERROR"); break;
  case (LE): return ("LE"); break;
  case (NOT): return ("NOT"); break;
  case (ISVOID): return ("ISVOID"); break;
  case '+': return ("+"); break;
  case '/': return ("/"); break;
  case '-': return ("-"); break;
  case '*': return ("*"); break;
  case '=': return ("="); break;
  case '<': return ("<"); break;
  case '.': return ("."); break;
  case '~': return ("~"); break;
  case ',': return (","); break;
  case ';': return (";"); break;
  case ':': return (":"); break;
  case '(': return ("("); break;
  case ')': return (")"); break;
  case '@': return ("@"); break;
  case '{': return ("{"); break;
  case '}': return ("}"); break;
  default: return ("<Invalid Token>");
  }
}

void print_cool_token(int tok) {
  const char *tok_string = cool_token_to_string(tok);
  if (strlen(tok_string) == 1) {
    // Single character tokens should be quoted
    cerr << std::quoted(tok_string, '\'');
  } else {
    cerr << tok_string;
  }

  switch (tok) {
  case (STR_CONST):
    cerr << " = ";
    cerr << " \"";
    cerr << (cool_yylval.symbol->get_string());
    cerr << "\"";
#ifdef CHECK_TABLES
    stringtable.lookup_string(cool_yylval.symbol->get_string());
#endif
    break;
  case (INT_CONST): cerr << " = " << cool_yylval.symbol;
#ifdef CHECK_TABLES
    inttable.lookup_string(cool_yylval.symbol->get_string());
#endif
    break;
  case (BOOL_CONST): cerr << (cool_yylval.boolean ? " = true" : " = false"); break;
  case (TYPEID):
  case (OBJECTID): cerr << " = " << cool_yylval.symbol;
#ifdef CHECK_TABLES
    idtable.lookup_string(cool_yylval.symbol->get_string());
#endif
    break;
  case (ERROR):
    cerr << " = ";
    cerr << get_escaped_string(cool_yylval.error_msg);
    break;
  }
}

// dump the token in format readable by the sceond phase token lexer
// void dump_cool_token(ostream& out, int lineno, int token, YYSTYPE yylval)
void build_yaml_tree(int lineno, int token, YYSTYPE yylval, ryml::NodeRef token_node) {
  token_node |= ryml::MAP;

  token_node["kind"] << cool_token_to_string(token);
  token_node["lineno"] << lineno;

  switch (token) {
  case (STR_CONST): token_node["symbol"] << get_escaped_string(cool_yylval.symbol->get_string());
#ifdef CHECK_TABLES
    stringtable.lookup_string(cool_yylval.symbol->get_string());
#endif
    break;
  case (INT_CONST): token_node["symbol"] << cool_yylval.symbol->get_string();
#ifdef CHECK_TABLES
    inttable.lookup_string(cool_yylval.symbol->get_string());
#endif
    break;
  case (BOOL_CONST): token_node["boolean"] << ryml::fmt::boolalpha(cool_yylval.boolean); break;
  case (TYPEID):
  case (OBJECTID): token_node["symbol"] << cool_yylval.symbol->get_string();
#ifdef CHECK_TABLES
    idtable.lookup_string(cool_yylval.symbol->get_string());
#endif
    break;
  case (ERROR):
    // sm: I've changed assignment 2 so students are supposed to
    // *not* coalesce error characters into one string; therefore,
    // if we see an "empty" string here, we can safely assume the
    // lexer is reporting an occurrance of an illegal NUL in the
    // input stream
    if (cool_yylval.error_msg[0] == 0) {
      token_node["symbol"] << "\\000";
    } else {
      token_node["symbol"] << get_escaped_string(cool_yylval.error_msg);
      break;
    }
  }
}
