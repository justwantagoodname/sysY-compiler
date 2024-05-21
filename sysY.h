#pragma once

#ifndef SYSY_H
#define SYSY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *yytext;
extern FILE *yyin;
extern int yylval;

int yylex(void);
void yyerror(const char *s);

#endif
