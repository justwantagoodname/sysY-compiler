CLEAR = clear
WATCHER = entr
LEX = flex
YACC = bison

CC = gcc
CFLAGS = 
LDFLAGS = 

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
    LDFLAGS += -lfl
endif

ifeq ($(UNAME), Darwin)
    LDFLAGS += -ll -ly
endif

ZIP = zip
TEST_DIR=test

GEN_FILES = lex.yy.c y.* $(TEST_DIR)/lexer.* $(TEST_DIR)/lexer $(TEST_DIR)/output.txt $(TEST_DIR)/parser $(TEST_DIR)/parser.*

SRC = lex.yy.c y.tab.c token.c
OBJ = $(SRC:.c=.o)

lexer: $(TEST_DIR)/lexer
parser: $(TEST_DIR)/parser

$(TEST_DIR)/lexer: lex.yy.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(TEST_DIR)/parser:	lex.yy.o y.tab.o token.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lex.yy.c: y.tab.h sysY.l
	$(LEX) sysY.l

y.tab.h y.tab.c: sysY.y
	$(YACC) -vdt -b y sysY.y

test-lex: $(TEST_DIR)/lexer
	$(CLEAR) && date && cd test && ./lexer

test-parser: $(TEST_DIR)/parser
	$(CLEAR) && date && cd test && ./parser -o -

submission.zip: y.tab.h y.tab.c lex.yy.c
	$(ZIP) submission.zip -r y.tab.h y.tab.c lex.yy.c $(wildcard *.c) $(wildcard *.h) lib

zip: submission.zip

clean:
	$(RM) -f $(GEN_FILES) $(OBJ) submission.zip

dev:
	echo *.c *.h sysY.* $(TEST_DIR)/testfile.txt | tr '[:blank:]' '\n' | $(WATCHER) make test-parser

.PHONY: clean test-lex test-parser lexer parser dev zip

.DEFAULT_GOAL = test-parser
