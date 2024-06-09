CLEAR = clear
WATCHER = entr
LEX = flex
YACC = bison
ZIP = zip

DEFINES = -DXML_PP -DDEBUG
CXXFLAGS = -g -std=c++17 -lm
LDFLAGS = 
JOBS := 4

UNAME := $(shell uname)
GIT_VERSION := $(shell git describe --always --dirty --tags)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)


DEFINES += -DVERSION_TEXT=\"$(GIT_BRANCH)-$(GIT_VERSION)\"

INCLUDE_DIR = include lib frontend/parser package
BUILD_DIR = build
TEST_DIR=test

ifeq ($(UNAME), Linux)
    LDFLAGS += 
	JOBS := $(shell nproc)
endif

ifeq ($(UNAME), Darwin)
    LDFLAGS += -ll -ly
	JOBS := $(shell sysctl -n hw.ncpu)
endif

Y_FILES := $(shell find . -type f -name "*.y" | sed 's/^\.\///')
L_FILES := $(shell find . -type f -name "*.l" | sed 's/^\.\///')
H_FILES := $(shell find . -type f -name "*.h" | sed 's/^\.\///')
C_FILES := $(shell find . -type f -name "*.c" | sed 's/^\.\///')
CC_FILES := $(shell find . -type f -name "*.cc" | sed 's/^\.\///')
HPP_FILES := $(shell find . -type f -name "*.hpp" | sed 's/^\.\///')

BISON_C_FILES := $(Y_FILES:.y=.tab.c)
BISON_H_FILES := $(Y_FILES:.y=.tab.h)
FLEX_C_FILES := $(L_FILES:.l=.lex.c)

SRC = $(C_FILES) $(CC_FILES) $(BISON_C_FILES) $(FLEX_C_FILES)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

compiler: $(BUILD_DIR)/compiler

$(BUILD_DIR)/compiler: $(SRC)
	$(CXX) $(CXXFLAGS) $(addprefix -I, $(INCLUDE_DIR)) $(DEFINES) $(SRC) -o $@

bison-files: $(BISON_C_FILES) $(BISON_H_FILES)

flex-files: $(FLEX_C_FILES) bison-files

gen-files: bison-files flex-files

%.tab.c %.tab.h: %.y | $(BUILD_DIR)
	$(YACC) -v -b $* -d -o $*.tab.c $<

%.lex.c: %.l | $(BUILD_DIR)
	$(LEX) -o $*.lex.c $<

test-compiler: compiler
	$(CLEAR) && date && cd $(BUILD_DIR) && ./compiler ../$(TEST_DIR)/testfile.sysy -S -o -

submission.zip: gen-files $(C_FILES) $(H_FILES) $(HPP_FILES)
	$(ZIP) submission.zip -r $(SRC) $(BISON_H_FILES) $(H_FILES) $(HPP_FILES)

zip: submission.zip

clean:
	find $(BUILD_DIR) -type f -name "*.o" -delete
	find . -type f -name "*.lex.*" -delete
	find . -type f -name "*.tab.*" -delete
	find . -type f -name "*.output" -delete

requirements:
ifeq ($(UNAME), Linux) 
		sudo apt-get -y install build-essential flex bison entr libxml2-utils
endif

ifeq ($(UNAME), Darwin)
		xcode-select --install
		brew install flex bison entr
endif

dev:
	echo $(L_FILES) $(Y_FILES) $(C_FILES) $(CC_FILES) $(TEST_DIR)/testfile.sysy \
		 | tr '[:blank:]' '\n' | $(WATCHER) -cs 'make -j$(JOBS) test-compiler'

debug: compiler
	$(GDB) $(BUILD_DIR)/compiler -ex "run -i $(TEST_DIR)/testfile.sysy -o -"

all: compiler

.PHONY: clean dev zip test-compiler compiler bison-files flex-files gen-files requirements debug

.DEFAULT_GOAL := all
