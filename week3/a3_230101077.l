%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

int lineNo = 1;
int colNo = 1;

HashTable *st;

FILE *symbolOut,*tokenOutFile;
char lastToken[512];

// --- State Machine for Token Expectation ---
int expectingIdentifier = 0;
int inDeclaration = 0;

typedef struct {
    char *name;
    int line;
    int col;
} SymbolTableEntry;

// Function prototypes
unsigned int hash_symbol(void *key);
int compare_symbols(void *key1, void *key2);
void print_symbol(void *key, void *value);

// Helper to log if a token violates our expectation
void check_expectation_violation(const char* foundToken) {
    if (expectingIdentifier) {
        fprintf(stderr, "Syntax Error at L:%d C:%d: Expected IDENTIFIER, but found '%s'\n", 
                lineNo, colNo, yytext);
        expectingIdentifier = 0; // Reset to avoid cascading error spam
    }
}

// Helper to manage symbol table logic
void handle_identifier(char* name) {
    SymbolTableEntry *found = (SymbolTableEntry *)lookup(st, name);
    
    if (found) {
        fprintf(stderr, "%s already declared at L:%d C:%d\n",
                name, found->line, found->col);
    } else {
        SymbolTableEntry *entry = malloc(sizeof(SymbolTableEntry));
        entry->name = strdup(name);
        entry->line = lineNo;
        entry->col = colNo;
        
        insert(st, entry->name, entry);

        fprintf(symbolOut, "%-15s %-10d %-10d\n",
                name, entry->line, entry->col);
    }
}
void IncrementCol(){
    colNo += yyleng;
}
void SetLastToken(){
    strncpy(lastToken, yytext, sizeof(lastToken) - 1);
    lastToken[sizeof(lastToken) - 1] = '\0';
}
int isTypeKeyword(const char *tok) {
    return strcmp(tok, "int") == 0 ||
           strcmp(tok, "float") == 0 ||
           strcmp(tok, "char") == 0 ||
           strcmp(tok, "double") == 0 ||
           strcmp(tok, "short") == 0 ||
           strcmp(tok, "long") == 0 ||
           strcmp(tok, "signed") == 0 ||
           strcmp(tok, "unsigned") == 0 ||
           strcmp(tok, "void") == 0;
}

void free_symbol_entry(void *key, void *value) {
    SymbolTableEntry *e = (SymbolTableEntry *)value;
    if (e) {
        free(e->name);
        free(e);
    }
}

%}

/* ---------- DEFINITIONS ---------- */

DIGIT        [0-9]
NONZERO      [1-9]
LETTER       [a-zA-Z]
ID           [_a-zA-Z][_a-zA-Z0-9]*

INT_CONST    {NONZERO}{DIGIT}*|0
FLOAT_CONST  {DIGIT}*"."{DIGIT}+|{DIGIT}+"."

ESCAPE       \\[\'\"\?\\abfnrtv]
CCHAR        [^\'\\\n]|{ESCAPE}
CHAR_CONST   \'({CCHAR})+\'

SCHAR        [^\"\\\n]|{ESCAPE}
STRING       \"({SCHAR})*\"

/* Keywords */
KEYWORD      break|case|char|else|default|float|for|return|void|short|unsigned|long|double|signed|while|Bool|continue|if|do|int|static

/* Punctuators (longest first) */
PUNCT        "..."|"->"|"++"|"--"|"<<"|">>"|"<="|">="|"=="|"!="|"&&"|"||"|"*="|"/="|"%="|"+="|"-="|"<<="|">>="|"&="|"^="|"|="|"+"|"-"|"*"|"/"|"%"|"&"|"|"|"^"|"~"|"!"|"<"|">"|"="|"?"|":"|";"|","|"#"|"."|"("|")"|"["|"]"|"{"|"}"

/* Comments */
SINGLE_COMMENT   "//"[^\n]*
MULTI_COMMENT    "/*"([^*]|\*+[^*/])*\*+"/"

NEWLINE     \n
/* Whitespace */
WS           [ \t\r]+

%%

{SINGLE_COMMENT}    { /* ignore */IncrementCol(); }
{MULTI_COMMENT}     {
                      int lastNewline = -1;
                      for (int i = 0; i < yyleng; i++) {
                          if (yytext[i] == '\n') {
                              lineNo++;
                              fprintf(tokenOutFile,"----------<LINE %d>----------\n", lineNo);
                              lastNewline = i;
                          }
                      }
                      if (lastNewline == -1) {
                          colNo += yyleng;
                      } else {
                          colNo = yyleng - lastNewline;
                      }
                    }

{KEYWORD}           { 
                        if (isTypeKeyword(yytext)) {
                            inDeclaration = 1;
                            expectingIdentifier = 1;
                        } else {
                            check_expectation_violation(yytext);
                            inDeclaration = 0;
                        }
                        fprintf(tokenOutFile,"<KEYWORD, %s >\n", yytext);
                        IncrementCol(); 
                    }

{ID}                { 
                        if (expectingIdentifier) {
                            handle_identifier(yytext);
                            expectingIdentifier = 0; // Expectation met
                        }
                        fprintf(tokenOutFile,"<IDENTIFIER, %s >\n", yytext);
                        IncrementCol();
                    }

{FLOAT_CONST}       { check_expectation_violation("FLOAT");fprintf(tokenOutFile,"<FLOAT, %s >\n", yytext); IncrementCol();SetLastToken(); }
{INT_CONST}         { check_expectation_violation("INT");fprintf(tokenOutFile,"<INTEGER, %s >\n", yytext); IncrementCol();SetLastToken(); }
{CHAR_CONST}        { check_expectation_violation("CHAR");fprintf(tokenOutFile,"<CHAR, %s >\n", yytext); IncrementCol();SetLastToken(); }
{STRING}            { check_expectation_violation("STRING");fprintf(tokenOutFile,"<STRING, %s >\n", yytext); IncrementCol();SetLastToken(); }

{PUNCT}             { 
                        if (strcmp(yytext, ";") == 0) {
                            check_expectation_violation(";");
                            inDeclaration = 0;
                            expectingIdentifier = 0;
                        } else if (strcmp(yytext, ",") == 0) {
                            if (inDeclaration) {
                                check_expectation_violation(","); // Error if user did 'int a,,'
                                expectingIdentifier = 1; // Propagate expectation
                            }
                        } else if (strcmp(yytext, "*") == 0 && inDeclaration) {
                            // It's a pointer in a declaration (e.g. int *a). Still expecting identifier, so do nothing.
                        } else {
                            // Any other punctuator (like =, [, ])
                            check_expectation_violation(yytext);
                        }
                        
                        fprintf(tokenOutFile,"<PUNCTUATOR, %s >\n", yytext); 
                        IncrementCol(); 
                    }

{NEWLINE}           { fprintf(tokenOutFile,"----------<LINE %d>----------\n",++lineNo);colNo=1;lastToken[0]='\0';}
{WS}                { IncrementCol();  }

.                   {   
                        check_expectation_violation(yytext);
                        fprintf(tokenOutFile,"<UNKNOWN, %s >\n", yytext); 
                        fprintf(stderr,"Line: %d, Col: %d UNKNOWN symbol '%s'\n", lineNo, colNo, yytext);  
                        IncrementCol(); 
                    }

%%

int main(int argc,char ** argv) {
    memset(lastToken,0,sizeof(lastToken)); 
    char *outFileName = "a3_230101077_token.txt";
    char *symbolOutName = "a3_230101077_st.txt";
    if(argc == 2){
       FILE*fp = fopen(argv[1],"r"); 
       if(fp == NULL){
            fprintf(stderr,"Unable to open file %s for lexing.\n",argv[1]);
            return 2;
       }
       // snprintf(outFileName, sizeof(outFileName), "%s_symbol.out", argv[1]);
       yyin = fp;
    }else if(argc > 2){
        fprintf(stderr,"USAGE: nanoLex [inFile]\n");
        return 1;
    }else if(argc ==1){
       yyin = stdin;
       // snprintf(outFileName, sizeof(outFileName), "stdin_symbol.out");
    }
    if((symbolOut=fopen(symbolOutName,"w")) ==NULL){
        fprintf(stderr,"Unable to open %s for symbol output.\n",outFileName);
        return 2;
    }
    if((tokenOutFile=fopen(outFileName,"w")) ==NULL){
        fprintf(stderr,"Unable to open %s for token output.\n",outFileName);
        return 2;
    }
    if(yyin != stdin){
        int c = fgetc(yyin);   // read first char
        if (c == EOF) {
            return 0;
        }
        ungetc(c, yyin);
    }

    fprintf(tokenOutFile,"----------<LINE %d>----------\n",1);
    lineNo=1;
    st = create_table(50, hash_symbol, compare_symbols, print_symbol);

    yylex();

    fprintf(stdin,"\n--- FINAL SYMBOL TABLE ---\n");
    print_table(st);

    free_table_with_cb(st, free_symbol_entry);
    return 0;
}

unsigned int hash_symbol(void *key) {
    char *str = (char *)key; 
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

int compare_symbols(void *key1, void *key2) {
    return strcmp((char *)key1, (char *)key2);
}

void print_symbol(void *key, void *value) {
    SymbolTableEntry *e = (SymbolTableEntry *)value;
    fprintf(stdout,"[%s | L:%d C:%d]", e->name, e->line, e->col);
}
