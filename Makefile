CC := gcc
BISON := bison
FLEX := flex
CPPFLAGS := -Isrc -Ibuild/generated
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
BISONFLAGS := -Wall -Wcounterexamples
FLEXFLAGS := --warn

BUILD_DIR := build
OBJECT_DIR := $(BUILD_DIR)/objects
GENERATED_DIR := $(BUILD_DIR)/generated

PARSER_SPEC := src/parser/parser.y
PARSER_SOURCE := $(GENERATED_DIR)/parser.tab.c
PARSER_HEADER := $(GENERATED_DIR)/parser.tab.h
LEXER_SPEC := src/lexer/lexer.l
LEXER_SOURCE := $(GENERATED_DIR)/lex.yy.c

AST_OBJECTS := \
	$(OBJECT_DIR)/ast.o \
	$(OBJECT_DIR)/ast_print.o
AST_TEST_OBJECT := $(OBJECT_DIR)/test_ast.o
TOKEN_TEST_OBJECT := $(OBJECT_DIR)/test_token_interface.o
LEXER_OBJECT := $(OBJECT_DIR)/lexer.o
LEXER_DRIVER_OBJECT := $(OBJECT_DIR)/lexer_driver.o
AST_TEST_BINARY := $(BUILD_DIR)/ast_tests
TOKEN_TEST_BINARY := $(BUILD_DIR)/token_interface_test
LEXER_TEST_BINARY := $(BUILD_DIR)/lexer_test

.PHONY: all test clean token-interface lexer-test

all: $(AST_TEST_BINARY) $(TOKEN_TEST_BINARY) $(LEXER_TEST_BINARY)

token-interface: $(PARSER_HEADER)

lexer-test: $(LEXER_TEST_BINARY)

$(PARSER_SOURCE) $(PARSER_HEADER) &: $(PARSER_SPEC)
	@mkdir -p $(GENERATED_DIR)
	$(BISON) $(BISONFLAGS) -d -o $(PARSER_SOURCE) $(PARSER_SPEC)

$(LEXER_SOURCE): $(LEXER_SPEC) $(PARSER_HEADER) src/lexer/lexer.h
	@mkdir -p $(GENERATED_DIR)
	$(FLEX) $(FLEXFLAGS) -o $@ $(LEXER_SPEC)

$(OBJECT_DIR)/ast.o: src/ast/ast.c src/ast/ast.h \
                         src/common/source_location.h src/common/value_type.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJECT_DIR)/ast_print.o: src/ast/ast_print.c src/ast/ast.h \
                               src/common/source_location.h src/common/value_type.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(AST_TEST_OBJECT): tests/unit/test_ast.c src/ast/ast.h \
                         src/common/source_location.h src/common/value_type.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TOKEN_TEST_OBJECT): tests/unit/test_token_interface.c $(PARSER_HEADER) \
                           src/common/source_location.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LEXER_OBJECT): $(LEXER_SOURCE) $(PARSER_HEADER) src/lexer/lexer.h \
                      src/common/source_location.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LEXER_DRIVER_OBJECT): tests/support/lexer_driver.c $(PARSER_HEADER) \
                             src/lexer/lexer.h src/common/source_location.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(AST_TEST_BINARY): $(AST_OBJECTS) $(AST_TEST_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

$(TOKEN_TEST_BINARY): $(TOKEN_TEST_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

$(LEXER_TEST_BINARY): $(LEXER_OBJECT) $(LEXER_DRIVER_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

test: all
	bash tests/run_tests.sh

clean:
	rm -rf -- $(BUILD_DIR)
