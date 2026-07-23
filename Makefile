CC := gcc
BISON := bison
FLEX := flex
CPPFLAGS := -Isrc -Ibuild/generated
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
BISONFLAGS := -Wall -Wcounterexamples -Werror=conflicts-sr -Werror=conflicts-rr
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
SYMBOL_TABLE_OBJECT := $(OBJECT_DIR)/symbol_table.o
SEMANTIC_OBJECT := $(OBJECT_DIR)/semantic.o
AST_TEST_OBJECT := $(OBJECT_DIR)/test_ast.o
SYMBOL_TABLE_TEST_OBJECT := $(OBJECT_DIR)/test_symbol_table.o
TOKEN_TEST_OBJECT := $(OBJECT_DIR)/test_token_interface.o
LEXER_OBJECT := $(OBJECT_DIR)/lexer.o
LEXER_DRIVER_OBJECT := $(OBJECT_DIR)/lexer_driver.o
PARSER_OBJECT := $(OBJECT_DIR)/parser.o
PARSER_DRIVER_OBJECT := $(OBJECT_DIR)/parser_driver.o
SEMANTIC_DRIVER_OBJECT := $(OBJECT_DIR)/semantic_driver.o
AST_TEST_BINARY := $(BUILD_DIR)/ast_tests
SYMBOL_TABLE_TEST_BINARY := $(BUILD_DIR)/symbol_table_tests
TOKEN_TEST_BINARY := $(BUILD_DIR)/token_interface_test
LEXER_TEST_BINARY := $(BUILD_DIR)/lexer_test
PARSER_TEST_BINARY := $(BUILD_DIR)/parser_test
SEMANTIC_TEST_BINARY := $(BUILD_DIR)/semantic_test

.PHONY: all test clean token-interface lexer-test parser-test \
	symbol-table-test semantic-test

all: $(AST_TEST_BINARY) $(SYMBOL_TABLE_TEST_BINARY) \
	$(TOKEN_TEST_BINARY) $(LEXER_TEST_BINARY) $(PARSER_TEST_BINARY) \
	$(SEMANTIC_TEST_BINARY)

token-interface: $(PARSER_HEADER)

lexer-test: $(LEXER_TEST_BINARY)

parser-test: $(PARSER_TEST_BINARY)

symbol-table-test: $(SYMBOL_TABLE_TEST_BINARY)

semantic-test: $(SEMANTIC_TEST_BINARY)

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

$(SYMBOL_TABLE_OBJECT): src/symbol_table/symbol_table.c \
                             src/symbol_table/symbol_table.h \
                             src/common/source_location.h \
                             src/common/value_type.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(SEMANTIC_OBJECT): src/semantic/semantic.c src/semantic/semantic.h \
                         src/ast/ast.h src/symbol_table/symbol_table.h \
                         src/common/source_location.h src/common/value_type.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(AST_TEST_OBJECT): tests/unit/test_ast.c src/ast/ast.h \
                         src/common/source_location.h src/common/value_type.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(SYMBOL_TABLE_TEST_OBJECT): tests/unit/test_symbol_table.c \
                                  src/symbol_table/symbol_table.h \
                                  src/common/source_location.h \
                                  src/common/value_type.h
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

$(PARSER_OBJECT): $(PARSER_SOURCE) $(PARSER_HEADER) src/parser/parser.h \
                       src/lexer/lexer.h src/ast/ast.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(PARSER_SOURCE) -o $@

$(PARSER_DRIVER_OBJECT): tests/support/parser_driver.c src/parser/parser.h \
                              src/ast/ast.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(SEMANTIC_DRIVER_OBJECT): tests/support/semantic_driver.c \
                                src/parser/parser.h src/semantic/semantic.h \
                                src/ast/ast.h
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(AST_TEST_BINARY): $(AST_OBJECTS) $(AST_TEST_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

$(SYMBOL_TABLE_TEST_BINARY): $(SYMBOL_TABLE_OBJECT) \
                                  $(SYMBOL_TABLE_TEST_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

$(TOKEN_TEST_BINARY): $(TOKEN_TEST_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

$(LEXER_TEST_BINARY): $(PARSER_OBJECT) $(LEXER_OBJECT) $(AST_OBJECTS) \
                           $(LEXER_DRIVER_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

$(PARSER_TEST_BINARY): $(PARSER_OBJECT) $(LEXER_OBJECT) $(AST_OBJECTS) \
                            $(PARSER_DRIVER_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

$(SEMANTIC_TEST_BINARY): $(PARSER_OBJECT) $(LEXER_OBJECT) $(AST_OBJECTS) \
                              $(SYMBOL_TABLE_OBJECT) $(SEMANTIC_OBJECT) \
                              $(SEMANTIC_DRIVER_OBJECT)
	$(CC) $(CFLAGS) $^ -o $@

test: all
	bash tests/run_tests.sh

clean:
	rm -rf -- $(BUILD_DIR)
