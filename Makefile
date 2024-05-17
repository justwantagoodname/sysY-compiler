LEX = flex
YACC = bison
CC = gcc
ZIP = zip
TEST_DIR=test

objects = lex.yy.c $(TEST_DIR)/lexer.* $(TEST_DIR)/lexer $(TEST_DIR)/output.txt $(TEST_DIR)/parser $(TEST_DIR)/parser.* y.*

lexer: lex.yy.c
	$(CC) lex.yy.c -o $(TEST_DIR)/lexer

parser:	lex.yy.c y.tab.h y.tab.c
	$(CC) lex.yy.c y.tab.h y.tab.c -o $(TEST_DIR)/$@ -lfl -ly


lex.yy.c: sysY.l
	$(LEX) sysY.l

clean:
	$(RM) $(objects)

test-lex: lexer
	cd test && ./lexer

test-parser: parser
	cd test && ./parser

y.tab.h y.tab.c y.output: sysY.y
	$(YACC) -vdty sysY.y

submit.zip: lex.yy.c
	$(ZIP) submit.zip lex.yy.c

.PHONY: clean test

.DEFAULT_GOAL = test-parser
