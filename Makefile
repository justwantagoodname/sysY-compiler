CLEAR = clear
WATCHER = entr
LEX = flex
YACC = bison

CC = gcc
CFLAGS = 
LDFLAGS = -ly

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
    LDFLAGS += -lfl
endif

ifeq ($(UNAME), Darwin)
    LDFLAGS += -ll
endif

ZIP = zip
TEST_DIR=test

GEN_FILES = lex.yy.c y.* $(TEST_DIR)/lexer.* $(TEST_DIR)/lexer $(TEST_DIR)/output.txt $(TEST_DIR)/parser $(TEST_DIR)/parser.*

SRC = lex.yy.c y.tab.c
OBJ = $(SRC:.c=.o)

lexer: $(TEST_DIR)/lexer
parser: $(TEST_DIR)/parser

$(TEST_DIR)/lexer: lex.yy.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(TEST_DIR)/parser:	lex.yy.o y.tab.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lex.yy.c: y.tab.h sysY.l
	$(LEX) sysY.l

y.tab.h y.tab.c: sysY.y
	$(YACC) -vdty sysY.y

test-lex: $(TEST_DIR)/lexer
	$(CLEAR) && date && cd test && ./lexer

test-parser: $(TEST_DIR)/parser
	$(CLEAR) && date && cd test && ./parser -o -

submit.zip: y.tab.h y.tab.c lex.yy.c
	$(ZIP) submit.zip $(wildcard *.c) $(wildcard *.h)

zip: submit.zip

clean:
	$(RM) -f $(GEN_FILES) $(OBJ) submit.zip

dev:
	echo sysY.* $(TEST_DIR)/testfile.txt | tr '[:blank:]' '\n' | $(WATCHER) make test-parser

.PHONY: clean test-lex text-parser lexer parser dev zip

.DEFAULT_GOAL = test-parser
