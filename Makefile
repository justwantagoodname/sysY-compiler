CLEAR = clear
WATCHER = entr
LEX = flex
YACC = bison

CFLAGS = -g 
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

GEN_FILES = lex.yy.c y.* $(TEST_DIR)/lexer.* $(TEST_DIR)/lexer $(TEST_DIR)/output.txt $(TEST_DIR)/parser $(TEST_DIR)/parser.* $(TEST_DIR)/compiler $(TEST_DIR)/compiler.* submission.zip

SRC = main.c sysY.yy.c sysY.tab.c token.c sym.c ast.c action.c
OBJ = $(SRC:.c=.o)

compiler: $(TEST_DIR)/compiler

$(TEST_DIR)/compiler:	$(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

sysY.yy.c: sysY.tab.h sysY.l
	$(LEX) -o $@ sysY.l

sysY.tab.h sysY.tab.c: sysY.y
	$(YACC) -vdt -b sysY sysY.y

test-compiler: $(TEST_DIR)/compiler
	$(CLEAR) && date && cd test && ./compiler -o -

submission.zip: y.tab.h y.tab.c lex.yy.c
	$(ZIP) submission.zip -r y.tab.h y.tab.c lex.yy.c $(wildcard *.c) $(wildcard *.h) lib

zip: submission.zip

clean:
	$(RM) $(GEN_FILES) $(OBJ)

dev:
	echo *.c *.h sysY.* $(TEST_DIR)/testfile.txt | tr '[:blank:]' '\n' | $(WATCHER) make test-compiler

all: compiler

.PHONY: clean dev zip test-compiler compiler

.DEFAULT_GOAL = test-compiler
