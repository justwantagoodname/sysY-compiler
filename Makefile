WATCHER = entr
LEX = flex
YACC = bison
ZIP = zip

DEFINES = -DXML_PP -DDEBUG -DUNI_OPTIMIZTION -DASM_GEN
CXXFLAGS = -std=c++17 
LDFLAGS = -lm
JOBS := 4

UNAME := $(shell uname)

# 从 git 获取版本信息
GIT_VERSION := $(shell git describe --always --dirty --tags)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
DEFINES += -DVERSION_TEXT=\"$(GIT_BRANCH)-$(GIT_VERSION)\"

INCLUDE_DIR = include lib frontend/parser package
BUILD_DIR = build
DEV_BUILD_DIR = build-dev
TEST_DIR=test

ifeq ($(UNAME), Linux)
    LDFLAGS += 
	JOBS := $(shell nproc)
endif

ifeq ($(UNAME), Darwin)
    LDFLAGS += -ll -ly
	JOBS := $(shell sysctl -n hw.ncpu)
endif

# Disable default suffixes
%.c: %.y
%.c: %.l

Y_FILES = $(shell find . -type f -name "*.y" | sed 's/^\.\///')
L_FILES = $(shell find . -type f -name "*.l" | sed 's/^\.\///')
H_FILES = $(shell find . -type f -name "*.h" | sed 's/^\.\///')
HPP_FILES = $(shell find . -type f -name "*.hpp" ! -path "./testcase/*" | sed 's/^\.\///')
CC_FILES = $(shell find . -type f -name "*.cc" ! -path "./testcase/*" | sed 's/^\.\///')

BISON_C_FILES = $(Y_FILES:.y=.tab.cc)
BISON_H_FILES = $(Y_FILES:.y=.tab.hh)
FLEX_C_FILES = $(L_FILES:.l=.lex.cc)

# TEST_CASES := $(shell find testcase -type f -name "*.cc" | sed 's/^\.\///')

SRC = $(CC_FILES) # 运行的时候展开 因为必须确保 paser-files 生成之后才能编译

OBJ := 	$(patsubst %.cc, $(DEV_BUILD_DIR)/%.o, $(CC_FILES)) \
		$(patsubst %.tab.cc, $(DEV_BUILD_DIR)/%.tab.o, $(BISON_C_FILES)) \
		$(patsubst %.lex.cc, $(DEV_BUILD_DIR)/%.lex.o, $(FLEX_C_FILES))

OBJ := $(sort $(OBJ)) # 去重防止重复链接

TEST_SRC = $(filter-out main.cc,$(SRC)) $(TEST_CASES)
TEST_OBJ := $(patsubst %.cc, $(DEV_BUILD_DIR)/%.o, $(TEST_CASES))

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DEV_BUILD_DIR):
	mkdir -p $(DEV_BUILD_DIR)

release-compiler: CXXFLAGS+=-O2
release-compiler: $(BUILD_DIR)/compiler # cg 实际的编译方式, cg 上面的linker path 加了一堆 antlr4 的东西，我们就不加了

$(BUILD_DIR)/compiler: $(SRC) $(H_FILES) $(HPP_FILES) $(BISON_C_FILES) $(BISON_H_FILES) $(FLEX_C_FILES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(addprefix -I, $(INCLUDE_DIR)) $(DEFINES) $(SRC) -o $@

dev-compiler: CXXFLAGS+=-g
dev-compiler: $(DEV_BUILD_DIR)/compiler # 开发时使用，分别编译模式

$(DEV_BUILD_DIR)/compiler: parser-files $(OBJ) | $(DEV_BUILD_DIR)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(DEV_BUILD_DIR)/%.o: %.cc | $(DEV_BUILD_DIR) 
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(addprefix -I, $(INCLUDE_DIR)) $(DEFINES) -c $< -o $@

# flex 文件必须在生成 bison 文件之后才能生成
$(FLEX_C_FILES): $(BISON_C_FILES) $(BISON_H_FILES)

# 生成解析器文件
parser-files: $(FLEX_C_FILES) $(BISON_C_FILES) $(BISON_H_FILES)

%.tab.cc %.tab.hh: %.y
	$(YACC) -v -b $* -d -o $*.tab.cc $<

%.lex.cc: %.l
	$(LEX) -o $*.lex.cc $<

INPUT_FILE ?= "testfile.sysy"

test-compiler: dev-compiler # 测试的时候使用开发模式，快速编译
	@clear
	@date
	$(DEV_BUILD_DIR)/compiler $(TEST_DIR)/$(INPUT_FILE) -S -o $(TEST_DIR)/output.s

run-arm: $(TEST_DIR)/output.s $(TEST_DIR)/libsysy.a
	arm-linux-gnueabihf-gcc $< $(TEST_DIR)/libsysy.a -static -o $(TEST_DIR)/$(basename $(notdir $<)).arm
	qemu-arm-static $(TEST_DIR)/$(basename $(notdir $<)).arm

debug-arm: $(TEST_DIR)/output.s $(TEST_DIR)/libsysy.a
	arm-linux-gnueabihf-gcc -g $< $(TEST_DIR)/libsysy.a -static -o $(TEST_DIR)/$(basename $(notdir $<)).arm
	qemu-arm-static -g 1234 $(TEST_DIR)/$(basename $(notdir $<)).arm

start-gdb:
	gdb-multiarch $(TEST_DIR)/$(basename $(notdir $(TEST_DIR)/output.s)).arm -ex "target remote localhost:1234" -ex "b main" -ex "c"


run-riscv: $(TEST_DIR)/output.s $(TEST_DIR)/libsysy.riscv.a
	riscv64-linux-gnu-gcc-12 $< $(TEST_DIR)/libsysy.riscv.a -static -o $(TEST_DIR)/$(basename $(notdir $<)).rv
	qemu-riscv64-static $(TEST_DIR)/$(basename $(notdir $<)).rv

debug-riscv: $(TEST_DIR)/output.s $(TEST_DIR)/libsysy.riscv.a
	riscv64-linux-gnu-gcc-12 -g $< $(TEST_DIR)/libsysy.riscv.a -static -o $(TEST_DIR)/$(basename $(notdir $<)).rv
	qemu-riscv64-static -g 1234 $(TEST_DIR)/$(basename $(notdir $<)).rv

test-submit-compiler: release-compiler # 提交测试模式的时候使用 release 模式，确保没有问题
	@clear
	@date
	$(BUILD_DIR)/compiler $(TEST_DIR)/testfile.sysy -S -o -

submission.zip: gen-files $(C_FILES) $(H_FILES) $(HPP_FILES)
	$(ZIP) submission.zip -r $(SRC) $(BISON_H_FILES) $(H_FILES) $(HPP_FILES)

zip: submission.zip

clean:
	$(RM) -r $(BUILD_DIR)/* $(DEV_BUILD_DIR)/*
	find . -type f -name "*.lex.*" -delete
	find . -type f -name "*.tab.*" -delete
	find . -type f -name "*.output" -delete

requirements:
ifeq ($(UNAME), Linux) 
		sudo apt-get -y install build-essential flex bison entr libxml2-utils \
						gcc-arm-linux-gnueabihf libc6-dev-armhf-cross qemu-user-static gdb-multiarch \
						gcc-12-riscv64-linux-gnu libc6-dev-riscv64-cross qemu-system-misc \
endif

ifeq ($(UNAME), Darwin)
		xcode-select --install
		brew install flex bison entr
endif

dev:
	echo $(L_FILES) $(Y_FILES) $(C_FILES) $(CC_FILES) $(H_FILES) $(HPP_FILES) $(TEST_DIR)/testfile.sysy Makefile \
		 | tr '[:blank:]' '\n' | $(WATCHER) -cs 'make -j$(JOBS) test-compiler'

all: release-compiler

.PHONY: clean dev zip test-compiler release-compiler dev-compiler parser-files requirements start-gdb run-arm debug-arm test-submit-compiler

.DEFAULT_GOAL := all
