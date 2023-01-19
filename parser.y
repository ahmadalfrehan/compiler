%require "2.4.1"
%defines


%define api.token.constructor

%define parse.assert

%code requires

{
#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include "command.h"
using namespace std;

namespace EzAquarii {
    class Scanner;
    class Interpreter;
    }
extern int yylex(void);
void yyerror(char *);
int sym[26];
#include "lex.yy.c"
}
%code top
{
    #include <iostream>
    #include "scanner.h"
    #include "parser.hpp"
    #include "interpreter.h"
    #include "location.hh"
    
    // yylex() arguments are defined in parser.y
    static EzAquarii::Parser::symbol_type yylex(EzAquarii::Scanner &scanner, EzAquarii::Interpreter &driver) {
        return scanner.get_next_token();
    }
    
    // you can accomplish the same thing by inlining the code using preprocessor
    // x and y are same as in above static function
    // #define yylex(x, y) scanner.get_next_token()
    
    using namespace EzAquarii;
}


%union {
    int num;
    char sym;
    string std;
    string uint64_t;
}

%token EOL
%token <sym> TOK_EMPTY_LINE 
%token <num> NUMBER2 VARIABLE
%left '+' '-'
%left '*' '/'
%type<num> exp
%token PLUS
%lex-param { EzAquarii::Scanner &scanner }
%lex-param { EzAquarii::Interpreter &driver }
%parse-param { EzAquarii::Scanner &scanner }
%parse-param { EzAquarii::Interpreter &driver }
%locations
%define parse.trace


%token END 0 "end of file"
%token <std> STRING  "string";
%token <uint64_t> NUMBER "number";
%token LEFTPAR "leftpar";
%token RIGHTPAR "rightpar";
%token SEMICOLON "semicolon";
%token COMMA "comma";

%type <EzAquarii::Command> command;


%start program

%%


program :   {
start : program     {  
    printf("start -> program\n");                    
                    }
      ;

program : program unit {
                    
                     printf("program -> program unit\n");                  
                    
                        }
        | unit      
            {
                            printf("program -> unit\n");
        
                        }
        ;
unit : var_dec  {
                    printf("unit -> var_dec\n");
                }
        |statement{
                    printf("unit -> statement\n");
                }   

        |func_declaration {

                    printf("unit -> func_dec\n");
                }
        |func_definition {

                    printf("unit -> func_def\n",yytext);

                }
                ;
     ;

                cout << "*** RUN ***" << endl;
                cout << "Type function with list of parmeters. Parameter list can be empty" << endl
                     << "or contain positive integers only. Examples: " << endl
                     << " * function()" << endl
                     << " * function(1,2,3)" << endl
                     << "Terminate listing with ; to see parsed AST" << endl
                     << "Terminate parser with Ctrl-D" << endl;
                
                cout << endl << "prompt> ";
                
                driver.clear();
            }
        | program command
            {
                const Command &cmd = $2;
                cout << "command parsed, updating AST" << endl;
                driver.addCommand(cmd);
                cout << endl << "prompt> ";
            }
        | program SEMICOLON
            {
                cout << "*** STOP RUN ***" << endl;
                cout << driver.str() << endl;
            }
        ;


statement:
    exp      command : STRING LEFTPAR RIGHTPAR
        {
            string &id = $1;
            cout << "ID: " << id << endl;
            $$ = Command(id);
        }
    | STRING LEFTPAR arguments RIGHTPAR
        {
            string &id = $1;
            const std::vector<uint64_t> &args = $3;
            cout << "function: " << id << ", " << args.size() << endl;
            $$ = Command(id, args);
        }
    ;


arguments : NUMBER
        {
            uint64_t number = $1;
            $$ = std::vector<uint64_t>();
            $$.push_back(number);
            cout << "first argument: " << number << endl;
        }
    | arguments COMMA NUMBER
        {
            uint64_t number = $3;
            std::vector<uint64_t> &args = $1;
            args.push_back(number);
            $$ = args;
            cout << "next argument: " << number << ", arg list size = " << args.size() << endl;
        }
    ;              

    
    | VARIABLE '=' exp       { sym[$1] = $3; }
    ;

input: exp EOL {printf("%d\n",$1);} 
|   EOL;
exp: 
NUMBER2 {$$ = $1;} 
| 
exp  PLUS exp {$$=$1 + $3 ;};
    | VARIABLE                { $$ = sym[$1]; }
    | exp '+' exp           { $$ = $1 + $3; }
    | exp '-' exp           { $$ = $1 - $3; }
    | exp '*' exp           { $$ = $1 * $3; }
    | exp '/' exp           { $$ = $1 / $3; }
    | '(' exp ')'            { $$ = $2; }
    ;
if_statement: IF LPAREN expression RPAREN statement %prec IF_BREC {$$ = newflow('I', $3, $5, NULL, NULL);}
        |IF LPAREN expression RPAREN statement else_if ELSE statement //{$$ = newflow('I', $3, $5, $6,$7);}
        |IF LPAREN expression RPAREN statement ELSE statement {$$ = newflow('I', $3, $5, NULL, $7);}
        ;
declaration_list : declaration_list COMMA ID {
                    printf("declaration_list -> declaration_list COMMA ID\n");  
                       }
                 | declaration_list COMMA ID LTHIRD CONST_INT RTHIRD {
                    printf("declaration_list -> declaration_list COMMA ID LTHIRD CONST_INT RTHIRD\n"); 

                }     
                 ;  
func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON     {
                  
               $$ = newfndef('D',$2,$4,NULL);
                printf("func_declaration -> type_specifier id LPAREN parameter_list RPAREN SEMICOLON\n");
        }
        | type_specifier ID LPAREN RPAREN SEMICOLON {
            $$ = newfndef('D',$2,NULL,NULL);
                printf("func_declaration -> type_specifier id LPAREN RPAREN SEMICOLON\n");
        }
        ;

func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement {
            $$ = newfndef('D',$2,$4,$6);
                printf("func_definition -> type_specifier ID LPAREN parameter_list RPAREN compound_statement\n");
        }
        | type_specifier ID LPAREN RPAREN compound_statement {
               $$ = newfndef('D',$2,NULL,$5);
                printf("func_definition -> type_specifier id LPAREN RPAREN compound_statement\n");
        
        }
        ;               
parameter_list  : parameter_list COMMA type_specifier ID {
                printf("parameter_list -> parameter_list COMMA type_specifier ID\n");
            
        }
        | parameter_list COMMA type_specifier {
                printf("parameter_list -> parameter_list COMMA type_specifier\n");
                
        }
        | type_specifier ID {
                printf("parameter_list -> type_specifier ID\n");
        }
        | type_specifier {
                printf("parameter_list -> type_specifier\n");
        }
        ;




else_if: ELSEIF LPAREN expression RPAREN statement else_if
        |
        ;
        statement : var_dec {
                printf("statement -> var_dec\n");       

        }
      | expression_statement{
                printf("statement -> expression_statement\n");       

        }

      | compound_statement{
                printf("statement -> compound_statement\n");       

        }
      | FOR LPAREN expression_statement expression_statement expression RPAREN statement{
            $$ = newforloop('P', $3, $4, $5,$7);
            printf("For_statement -> LPAREN expression_statement expression_statement expression RPAREN statement\n");
      }
      | if_statement{
            printf("if_statement -> IF LPAREN expression RPAREN statement else_if ELSE statement\n");
      }
      | WHILE LPAREN expression RPAREN statement{
        $$ = newflow('W', $3, $5, NULL, NULL);
            printf("While_statement -> LPAREN expression RPAREN statement\n");
    
      }
      | RETURN expression SEMICOLON  {

            $$ = newreturn('R',$2);
             printf("Return_statement -> RETURN expression SEMICOLON\n");
      }
      ;
%%
 
int main2(){
    yyparse();
    return 0;
}
yyerror(char* s) {
    printf("ERROR: %s\n", s);
    return 0;
}