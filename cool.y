/*
 *  cool.y
 *              Parser definition for the COOL language.
 *
 */
%{
#include <iostream>
#include "cool-tree.h"
#include "stringtab.h"
#include "utilities.h"

/* memory */
#define YYINITDEPTH 10000
#define YYMAXDEPTH 10000

/* Locations */
#define YYLTYPE int              /* the type of locations */
#define cool_yylloc curr_lineno  /* use the curr_lineno from the lexer
                                    for the location of tokens */
extern int node_lineno;          /* set before constructing a tree node
                                    to whatever you want the line number
                                    for the tree node to be */

/* The default action for locations.  Use the location of the first
   terminal/non-terminal and set the node_lineno to that value. */
#define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current = Rhs[1];                             \
  node_lineno = Current;

#define SET_NODELOC(Current)  \
  node_lineno = Current;

extern char *curr_filename;

void yyerror(const char *s);        /*  defined below; called for each parse error */
extern int yylex();           /*  the entry point to the lexer  */
Program *ast_root;	      /* the result of the parse  */
Classes parse_results;        /* for use in semantic analysis */
int omerrs = 0;               /* number of erros in lexing and parsing */

%}

/* A union of all the types that can be the result of parsing actions. */
%union {
  Boolean boolean;
  Symbol symbol;
  Program *program;
  Class_ *class_;
  Classes classes;
  Feature *feature;
  Features features;
  Formal *formal;
  Formals formals;
  Case *case_;
  Cases cases;
  Expression *expression;
  Expressions expressions;
  const char *error_msg;
}

/* 
   Declare the terminals; a few have types for associated lexemes.
   The token ERROR is never used in the parser; thus, it is a parse
   error when the lexer returns it.

   The integer following token declaration is the numeric constant used
   to represent that token internally.  Typically, Bison generates these
   on its own, but we give explicit numbers to prevent version parity
   problems (bison 1.25 and earlier start at 258, later versions -- at
   257)
 */

%token CLASS 258 ELSE 259 FI 260 IF 261 IN 262 
%token INHERITS 263 LET 264 LOOP 265 POOL 266 THEN 267 WHILE 268
%token CASE 269 ESAC 270 OF 271 DARROW 272 NEW 273 ISVOID 274
%token <symbol>  STR_CONST 275 INT_CONST 276 
%token <boolean> BOOL_CONST 277
%token <symbol>  TYPEID 278 OBJECTID 279 
%token ASSIGN 280 NOT 281 LE 282 ERROR 283


/*  DON'T CHANGE ANYTHING ABOVE THIS LINE, OR YOUR PARSER WONT WORK       */
/**************************************************************************/
 
   /* Complete the nonterminal list below, giving a type for the semantic
      value of each non terminal. (See section 3.6 in the bison 
      documentation for details). */

/* Declare types for the grammar's non-terminals. */
%type <program> program
%type <classes> class_list
%type <class_> class

/* You will want to change the following line. */
/* %type <features> dummy_feature_list */
%type <features> feature_list
%type <features> features
%type <feature> feature
%type <expressions> expression_list
%type <expressions> expression_block
%type <expression> expression 
%type <expression> let_exp
%type <formals> formal_list
%type <formal> formal
%type <case_> case /* moved branch from expression to case_ */
%type <cases> case_list


/* Precedence declarations go here. */
%nonassoc IN
%right ASSIGN
%left NOT
%nonassoc '<' LE '='
%left '+' '-'
%left '*' '/'
%left ISVOID
%left '~'
%left '@'
%left '.'






%%
/* 
   Save the root of the abstract syntax tree in a global variable.
*/
program     : class_list  { /* make sure bison computes location information */
                @$ = @1;
                ast_root = program($1); };

class_list  : class                 /* single class */
                  { $$ = single_Classes($1);
                  parse_results = $$; }
            | class_list class      /* several classes */
                { $$ = append_Classes($1,single_Classes($2)); 
                  parse_results = $$; };

/* If no parent is specified, the class inherits from the Object class. */
class:  CLASS TYPEID '{' feature_list '}' ';' 
          { $$ = class_($2, idtable.add_string("Object"), $4, stringtable.add_string(curr_filename)); }
      | CLASS TYPEID INHERITS TYPEID '{' feature_list '}' ';' 
          { $$ = class_($2, $4, $6, stringtable.add_string(curr_filename)); }
      | CLASS error '{' feature_list '}' ';' 
          { yyerrok; yyclearin; $$ = NULL; }
      | CLASS error '{' error '}' ';' 
          { yyerrok; yyclearin; $$ = NULL; }
      ;

/* Collection of expressions */
expression_list : expression 
                  { $$ = single_Expressions($1); }
                | expression_list ',' expression 
                  { $$ = append_Expressions($1, single_Expressions($3)); }
                ;

/* Any multiline expression */
expression_block : expression ';'
                { $$ = single_Expressions($1); }
            |   expression ';' expression_block 
                { $$ = append_Expressions(single_Expressions($1), $3); }
            |   error ';'
                { yyclearin; $$ = nil_Expressions(); }
            ;


/* cases are STIL IN PROGRESS */
case : OBJECTID ':' TYPEID DARROW expression ';'
              { $$ = branch($1, $3, $5);}
            ;

case_list  :  case_list case { $$ = append_Cases($1, single_Cases($2)); }
       | case { $$ = single_Cases($1); }
       ;


/* different types of let expression declarations */
let_exp : OBJECTID ':' TYPEID ASSIGN expression IN expression 
            %prec IN
            { $$ = let($1, $3, $5, $7); }
          | OBJECTID ':' TYPEID IN expression 
            { $$ = let($1, $3, no_expr(), $5); }
          | OBJECTID ':' TYPEID ASSIGN expression ',' let_exp
            { $$ = let($1, $3, $5, $7); }
          | OBJECTID ':' TYPEID ',' let_exp
            { $$ = let($1, $3, no_expr(), $5); }
          ;

/* Collection of formals, seperated by commas  */
formal_list : formal
              { $$ = single_Formals($1); }
            | formal ','  formal_list 
              { $$ = append_Formals(single_Formals($1), $3); }
            |
              { $$ = nil_Formals(); }
            ;

/* These are like the params in methods */
formal : OBJECTID ':' TYPEID 
          { $$ = formal($1, $3); }
        ;

/* Feature list may be empty, but no empty features in list. */
feature_list:  features
                { $$ = $1; }
              |
                {  yyerrok; $$ = nil_Features(); } 
              ;

/* Features that are seperated by semicolons like attr and methods */
features : feature ';' 
              { $$ = single_Features($1); }
          | feature ';' features
              { $$ = append_Features(single_Features($1), $3); }
          | error ';'
              { $$ = nil_Features();}
          ;

/* Each individual feature (the individual attr and methods in a class) */
feature : OBJECTID ':' TYPEID ASSIGN expression
            { $$ = attr($1, $3, $5); }
          | OBJECTID ':' TYPEID 
            { $$ = attr($1, $3, no_expr()); }
          | OBJECTID '(' formal_list ')' ':' TYPEID '{' expression '}'
              { $$ = method($1, $3, $6, $8); }
          ; 

/* Types of individual expressions */
expression : OBJECTID ASSIGN expression
              { $$ = assign($1, $3); }

              /* implied self dispatch */
            | OBJECTID '(' expression_list ')'
              { $$ = dispatch(object(idtable.add_string("self")), $1, $3); }
            | OBJECTID '(' ')'
              { $$ = dispatch(object(idtable.add_string("self")), $1, nil_Expressions()); }


            | expression '.' OBJECTID '(' expression_list ')'
              { $$ = dispatch($1, $3, $5); }
            | expression '.' OBJECTID '(' ')'
              { $$ = dispatch($1, $3, nil_Expressions()); }

            | expression '@' TYPEID '.' OBJECTID '(' expression_list ')'
              { $$ = static_dispatch($1, $3, $5, $7); }
            | expression '@' TYPEID '.' OBJECTID '(' ')'
              { $$ = static_dispatch($1, $3, $5, nil_Expressions()); }


            | '{' expression_block '}'
              { $$ = block($2); }

            | IF expression THEN expression ELSE expression FI
              { $$ = cond($2, $4, $6); }
            | CASE expression OF case_list ESAC /* winston case expression */
              { $$ = typcase($2, $4); }
            | WHILE expression LOOP expression POOL
              { $$ = loop($2, $4); }
            
            | LET let_exp
              { $$ = $2; }
            | NEW TYPEID
              { $$ = new_($2); }

            | ISVOID expression
              { $$ = isvoid($2); }
            | NOT expression
              { $$ = comp($2); }

            /*  arithmetic */
            | expression '+' expression
              { $$ = plus($1, $3); }
            | expression '-' expression
              { $$ = sub($1, $3); }
            | expression '*' expression
              { $$ = mul($1, $3); }
            | expression '/' expression
              { $$ = divide($1, $3); }

            /* comp operations */
            | expression LE expression
              { $$ = leq($1, $3); }
            | expression '=' expression
              { $$ = eq($1, $3); }
            | expression '<' expression
              { $$ = lt($1, $3); }


            | '~' expression
              { $$ = neg($2); }


            | '(' expression ')' 
              { $$ = $2; }
              
            | OBJECTID
             { $$ = object($1); }
            | INT_CONST
              { $$ = int_const($1); }
            | BOOL_CONST
              { $$ = bool_const($1); }
            | STR_CONST
              { $$ = string_const($1); }

            ;

/* end of grammar */
%%

/* This function is called automatically when Bison detects a parse error. Don't change this. */
void yyerror(const char *s)
{
  extern int curr_lineno;

  cerr << "\"" << curr_filename << "\", line " << curr_lineno << ": " \
    << s << " at or near ";
  print_cool_token(yychar);
  cerr << endl;
  omerrs++;

  if(omerrs>50) {fprintf(stdout, "More than 50 errors\n"); exit(1);}
}

