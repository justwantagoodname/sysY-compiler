#pragma once

#ifndef SYSY_H
#define SYSY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

extern char *yytext;
extern FILE *yyin;


int yylex(void);

#endif
