CLEAR = clear
WATCHER = entr
LEX = flex
YACC = bison

CFLAGS = -I$(BUILD_DIR) -g -Wall 
LDFLAGS = 
JOBS := 4

UNAME := $(shell uname)

BUILD_DIR = build

ifeq ($(UNAME), Linux)
    LDFLAGS += -lfl
	JOBS := $(shell nproc)
endif

ifeq ($(UNAME), Darwin)
    LDFLAGS += -ll -ly
	JOBS := $(shell sysctl -n hw.ncpu)
endif

ZIP = zip
TEST_DIR=test

GEN_FILES = *.lex.* *.yy.* *.tab.* *.output $(TEST_DIR)/lexer.* $(TEST_DIR)/lexer $(TEST_DIR)/output.txt $(TEST_DIR)/parser $(TEST_DIR)/parser.* $(TEST_DIR)/compiler $(TEST_DIR)/compiler.* submission.zip

Y_FILES = $(shell find . -type f -name "*.y")
L_FILES = $(shell find . -type f -name "*.l")
H_FILES = $(shell find . -type f -name "*.h")
C_FILES = $(shell find . -type f -name "*.c")
CXX_FILES = $(shell find . -type f -name "*.cc")

BISON_C_FILES = $(Y_FILES:.y=.tab.c)
BISON_H_FILES = $(Y_FILES:.y=.tab.h)
FLEX_C_FILES = $(L_FILES:.l=.lex.c)

ALL_C_FILES = $(C_FILES) $(BISON_C_FILES) $(FLEX_C_FILES)

O_FILES = $(addprefix $(BUILD_DIR)/, $(ALL_C_FILES:.c=.o) $(CC_FILES:.cc=.o))

# SRC = main.c sysY.yy.c sysY.tab.c query.yy.c query.tab.c token.c sym.c ast.c action.c ast_query.c
# OBJ = $(SRC:.c=.o)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

compiler: $(BUILD_DIR)/compiler

$(BUILD_DIR)/compiler: $(O_FILES)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile .c files into .o files
$(BUILD_DIR)/%.o: %.c gen-files | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile .cc files into .o files
$(BUILD_DIR)/%.o: %.cc gen-files | $(BUILD_DIR)
	$(CXX) $(CFLAGS) -c -o $@ $<

bison-files: $(BISON_C_FILES) $(BISON_H_FILES)

flex-files: $(FLEX_C_FILES) bison-files

gen-files: bison-files flex-files

%.tab.c %.tab.h: %.y | $(BUILD_DIR)
	$(YACC) -b $* -d -o $*.tab.c $<

%.lex.c: %.l | $(BUILD_DIR)
	$(LEX) -o $*.lex.c $<

test-compiler: compiler
	$(CLEAR) && date && cd $(BUILD_DIR) && ./compiler -i ../$(TEST_DIR)/testfile.txt -o -

submission.zip: gen-files $(C_FILES) $(H_FILES) $(FLEX_C_FILES) $(L_FILES)
	$(ZIP) submission.zip -r y.tab.h y.tab.c lex.yy.c $(wildcard *.c) $(wildcard *.h) lib

zip: submission.zip

clean:
	$(RM) $(GEN_FILES) $(O_FILES) $(BUILD_DIR)/compiler

requirements:
	ifeq ($(UNAME), Linux)
		sudo apt-get -y install build-essential flex bison entr
	endif

dev:
	echo *.c *.h *.y *.l $(TEST_DIR)/testfile.txt | tr '[:blank:]' '\n' | $(WATCHER) make test-compiler

all: compiler

.PHONY: clean dev zip test-compiler compiler bison-files flex-files gen-files requirements

.DEFAULT_GOAL := all