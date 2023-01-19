%{
    #include "parser.tab.h"
    #include <iostream>
	#include <cstdlib>
	// #include "scanner.h"
	// #include "interpreter.h"
	// #include "parser.hpp"
	// #include "location.hh"
	using namespace std;
	#define yyterminate() EzAquarii::Parser::make_END(EzAquarii::location());
	#define YY_USER_ACTION m_driver.increaseLocation(yyleng);
    int count = 1;
    void yyerror(char *);
%}
%option nodefault
%option noyywrap
%option c++
%option yyclass="Scanner"
%option prefix="EzAquarii_"

letter      [a-zA-Z]

digit       [0-9]

id          ({letter}|"_")("_"|{letter}|{digit})*

num         [+-]?{digit}+

double      [+-]?({digit}+\.?{digit}+)

keywd       "main"|"while"|"float"|"int"|"if"|"printf"|"else"|"for"|"else if"|"return"|"switch"|"auto"|"double"|"char"|"string"|"asm"|"new"|"enum"|"this"|"operator"|"privet"|"case"|"template"|"extern"|"break"|"public"|"protected"|"try"|"catch"|"throw"|"register"|"class"|"typedef"|"friend"|"union"|"const"|"goto"|"short"|"unsigned"|"continue"|"signed"|"virtual"|"default"|"inline"|"sizeof"|"void"|"delete"|"static"|"volatile"|"do"|"long"|"struct"|"alignof"|"alignas"|"bool"|"char16_t"|"char32_t"|"default"|"dynamic_cast"|"goto"|"explicit"|"FALSE"|"inline"|"mutable"|"namespace"|"noexcept"|"nullptr"|"reinterpret_cast"|"static_assert"|"static_cast"|"thread_local"|"TRUE"|"typedef"|"typeid"|"typename"|"using"|"virtual"|"wchar_t" 

arry        ({id}"["({num}|{id})"]")

ws          [ \t]

commt       ("/*"({id}|"\n"|{num}|{ws})*"*/")

sp          [;,.]

literal     ({digit}|{letter}|{sp}|"="|"+"|"-"|"*"|"&"|"|"|">"|"<"|"!"|"%"|"$"|"#"|"~"|"/"|"_")

char        "'"{literal}"'"

%%
\(          {
                cout << "Scanner: '('" << endl;
                return EzAquarii::Parser::make_LEFTPAR(EzAquarii::location());
            }
            
\)          { 
                cout << "Scanner: ')'" << endl;
                return EzAquarii::Parser::make_RIGHTPAR(EzAquarii::location());
            }
            
;           {
                cout << "Scanner: ';'" << endl;
                return EzAquarii::Parser::make_SEMICOLON(EzAquarii::location());
            }
            
,           {
                cout << "Scanner: ','" << endl;
                return EzAquarii::Parser::make_COMMA(EzAquarii::location());
            }


[\n]                            {count++;return EOL;}
{ws}                            {}
{sp}                            {printf("special symbols = %s in line = %d\n",yytext,count);}

">"|"<"|"<="|">="|"!="|"=="    {printf("relation operator = %s in line = %d\n",yytext,count);}
"&&"|"||"|"!"                   {printf("logical operator = %s in line = %d\n",yytext,count);}
"++"|"--"                       {printf("unary operator = %s in line = %d\n",yytext,count);}
"&"|"|"|"<<"|">>"|"~"|"^"       {printf("bitwise operator = %s in line = %d\n",yytext,count);}
{commt}                         {printf("comment = %s in line = %d\n",yytext,count);}
"?:"                            {printf("conditional operator = %s in line = %d\n",yytext,count);}
"="|"+="|"-="|"*="|"/="|"%="     {printf("assignment oper = %s in line = %d\n",yytext,count);}
"+"|"-"|"*"|"/"|"%"             {printf("arithmetic oper= %s in line = %d\n",yytext,count);}
"["|"]"|"{"|"}"|"("|")"         {printf("parenthesis = %s in line = %d\n",yytext,count);}
{keywd}                         {printf("keyword = %s in line = %d\n",yytext,count);}
{arry}                          {printf("array = %s in line = %d\n",yytext,count);}
{id}                            {printf("identifier = %s in line = %d\n",yytext,count);}
{num}                           { yylval.num = atoi(yytext); return NUMBER;}
{double}                        {printf("decimal number constant= %s in line = %d\n",yytext,count);}
{char}                          {printf("character constant= %s in line = %d\n",yytext,count);}
.                               {printf("error in %d is %s",count,yytext);} 

<<EOF>>     { return yyterminate(); }
%%

void main(int argc, char **argv)
{
    if (argc > 1){
        for(int i=0;i<argc;i++)
            printf("value of argv[%d] = %s\n\n",i,argv[i]);
            yyin=fopen(argv[1],"r");
    }
    else
    yyin=stdin;
    yylex();
}
/*yywrap is called whenever lex reaches an end-of-file we must define this function because yylex() need it*/
int yywrap(){
    exit(0);
}