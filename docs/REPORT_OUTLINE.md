# Project Report Outline

Status: M1-M5 language/lexer/parser/AST/symbol-table evidence and uncommitted M6 semantic evidence are prepared; TAC/end-to-end result chapters remain outlines. The official manual's report structure is mandatory; section numbers below are normalized for the team's final report.

## Front matter

- Title: *Design and Implement a Mini Programming Language Compiler using Flex and Bison*
- Course/department/university
- Team Chonnochara member names and IDs if required
- Submission date
- Abstract: problem, six-phase approach, language scope, and verified result
- Table of contents, figures, and tables as appropriate

## 1. Introduction

- Compiler-front-end context and motivation
- Project replaces the lab final and integrates previously separate phases
- Fixed-language constraint and educational focus
- Scope ends at TAC

Evidence needed: final repository state and concise pipeline figure.

## 2. Objectives

- Tokenize valid input and diagnose invalid input
- Parse the complete fixed CFG and construct an AST
- Enforce declarations, scope, and type/operator rules
- Generate TAC for expressions, control flow, and print
- Provide reproducible build/tests and understandable documentation

## 3. Language Specification

- Types, keywords, identifiers, literals, delimiters, and comments
- Statements and nested blocks
- Arithmetic, relational, and logical operators
- Complete formal CFG
- Precedence and associativity table
- Manual-derived support for standalone/empty nested blocks and optional declaration initializers, plus explicit resolutions for print operand, numeric conversion, `%`, unary operations, condition types, empty-program/bare-expression forms, exact floating-literal spellings, equality compatibility, normal Bison EOF, source-path conventions, and the semantic-error taxonomy
- One valid and one invalid example

Evidence prepared: `docs/LANGUAGE_SPEC.md`, `docs/GRAMMAR.md`, the implemented lexer/parser sources, zero-conflict Bison generation, and executable lexical/syntax/AST tests. Semantic/TAC evidence remains later.

## 4. Compiler Architecture

- Pipeline/data-flow diagram
- Phase gates and diagnostic flow
- Directory/module structure
- Ownership/lifecycle overview
- Driver behavior and output contracts

Evidence needed: implemented `ARCHITECTURE.md`, exact filenames/functions, final diagram.

## 5. Lexer Design

- Implemented files: `src/lexer/lexer.l` and `lexer.h`; test-only `tests/support/lexer_driver.c`
- One 32-token authority generated from `src/parser/parser.y`; normal EOF and built-in `YYUNDEF` are not source tokens
- Identifier `[A-Za-z_][A-Za-z0-9_]*`, integer `[0-9]+`, and float `[0-9]+\.[0-9]+` rules
- Keyword-before-identifier equal-length ties and Flex longest match for prefixes, floats, comments, and multi-character operators
- `%option noyywrap yylineno noinput nounput never-interactive nodefault`; no `libfl` dependency
- spaces/tabs/CR/newline handling, `//` comments only, and line-only `SourceLocation` access for LF/CRLF
- deterministic first-error `LEX_INVALID_TOKEN` behavior for unmatched input and unsupported numeric forms
- borrowed `yytext` lifetime for immediate lexer display; M4 copies identifier values and converts numeric values before the next token
- 10 golden lexer cases covering every source token, compact operator overlap, the official sample, layout/comments/locations, malformed numeric forms, invalid input, and block-comment non-support

M3 evidence: Bison-header-before-Flex build under C11 warning flags, automated 32-token set equality, exact stdout/stderr/exit goldens, and full M2 regression success.

## 6. Parser Design

- Implemented files: complete `src/parser/parser.y`, public `parser.h`, and test-only `tests/support/parser_driver.c`
- `%union` strategy: AST nodes, temporary statement lists, copied identifier text, numeric values, source types, and binary-operator tags only
- Flex-to-Bison location flow using `yylineno`/`YYLTYPE`, then line-only `SourceLocation` on every constructed AST node
- structural precedence/associativity through the documented nonterminal tiers; braced control bodies eliminate dangling `else`
- normal Bison EOF acceptance without a custom source token; exactly 32 source tokens remain
- AST actions for every statement/expression/literal form, including optional declaration initializers and empty/nested blocks
- explicit ownership transfer plus `%destructor` cleanup for discarded strings, nodes, and temporary lists
- stable `SYN_UNEXPECTED_TOKEN` diagnostics, recovery at semicolon/closing brace, and suppression of a duplicate syntax callback for lexer-reported `YYUNDEF`
- zero shift/reduce and reduce/reduce conflicts under Bison 3.8.2 with conflict warnings promoted to errors
- 32 parser cases: all operators/forms, seven AST goldens, official sample, LF/CRLF, required syntax rejections, both recovery boundaries, and lexical/syntax integration

M4 limitation: these tests prove syntax and AST construction, not semantic correctness, TAC, a final driver, or leak freedom.

## 7. Abstract Syntax Tree

- Implemented node-kind diagram/table from `src/ast/ast.h`
- one tagged union, explicit expression/control children, and dynamic source-ordered lists for programs/blocks
- optional initializer child on declaration nodes and block nodes as statements/scopes
- `ast_new_*`, `ast_add_statement`, `ast_print`, and `ast_destroy` APIs
- line-only `SourceLocation`; copied-name and child-transfer ownership rules
- deterministic official-sample-shaped printed AST in `tests/expected/ast_unit.stdout`
- interaction with semantics and TAC

M2 evidence: clean C11 build, 15 direct AST unit tests, generated Bison token-header test, repeated-printer equality, and golden stdout comparison. Recursive destruction ran without a crash; do not claim leak freedom without a real memory-analysis run.

## 8. Semantic Analysis

- Implemented files: `src/semantic/semantic.h` and `semantic.c`; test-only integration in `tests/support/semantic_driver.c`
- Public `semantic_analyze` contract, caller-owned AST, private analyzer-owned symbol table, diagnostic count, and phase statuses
- Source-order AST traversal; program/global relationship; exactly one scope per block; no extra control-statement scopes
- Active/current/history lookup mapping to declaration resolution, redeclaration, scope violation, and undeclared diagnostics
- Internal valid/type expression result without extending the three source-language types
- Operator/type compatibility matrix, operation-local numeric promotion, exact storage compatibility, and Boolean condition rules
- Initialized-declaration traversal, pre-binding initializer visibility, post-analysis insertion, redeclaration preservation, and cascade suppression
- Exact six-code taxonomy and deterministic line-aware diagnostic format
- Six valid plus twenty invalid semantic cases with exact stderr/exit goldens; full M2-M5 regression results
- Limitation: semantic state is transient and no TAC/final compiler output is produced yet

Give this chapter appropriate depth because semantic correctness has the highest indicative implementation weight.

## 9. Symbol Table

- Implemented files: `src/symbol_table/symbol_table.h` and `symbol_table.c`; direct tests in `tests/unit/test_symbol_table.c`
- Opaque `SymbolTable`/`Symbol`, read-only `SymbolInfo`, copied name, `ValueType`, `SourceLocation`, scope ID, and scope depth
- Permanent creation-ordered scope-frame array; parent-linked active stack; declaration-ordered linked symbols with stable addresses
- automatic global ID 0/depth 0, monotonic non-reused child IDs, active/inactive state, and safe global-exit rejection
- distinct current, active innermost-to-outer, and newest-inactive history lookup operations
- same-scope duplicate rejection, legal shadowing, outer restoration, sibling isolation, and declaration-point/initializer-order support
- table-owned frames/symbols/names, borrowed lookup pointers, `symbol_get_info`, and destruction of active plus exited scopes
- deterministic printer format with active state and address-free metadata
- linear lookup complexity and why it is clearer than a hash framework at this language/project scale
- 30 direct tests, identical repeated output, and `tests/expected/symbol_table_unit.stdout`

M5/M6 boundary: M5 proves reusable symbol storage and scope mechanics; M6 consumes that API during AST traversal and owns source-facing name/type/operator/context diagnostics. Neither generates TAC.

## 10. Intermediate Code (TAC)

- Instruction format and at-most-three-address principle
- expression lowering and evaluation order
- temporary/label generation
- assignment and print
- initialized-declaration expression lowering and final store; no TAC for plain declarations or empty blocks
- relational/logical strategy
- `if`, `if-else`, and `while` control-flow patterns
- sample source-to-TAC walkthrough
- golden tests

## 11. Challenges and Solutions

Use only verified entries from `DEVELOPMENT_LOG.md`, such as:

- starting from an implementation-free template;
- establishing a supported toolchain;
- grammar ambiguities/conflict resolution;
- source-location propagation;
- scope-history distinction;
- parser recovery without cascades;
- deterministic TAC/testing.

For each, state the symptom/evidence, root cause, chosen solution, and validation.

## 12. Testing and Results

- Test strategy: unit/phase/integration/end-to-end
- Environment and exact commands
- Test-matrix summary by valid, lexical, syntax, and six semantic categories
- all language/operator/control-flow coverage
- expected versus actual output
- clean-build result and limitations/known bugs
- screenshots or references to appendices

Do not report planned or unexecuted tests as passes.

## 13. Conclusion

- What the completed compiler demonstrates
- Lessons about phase interfaces, grammar, scope, types, and IR
- Honest limitations
- Optional future work only after the mandatory result

## 14. References

At minimum consider and cite the resources actually used:

- official course project manual;
- GNU Flex manual;
- GNU Bison manual;
- relevant C/GCC/Make documentation;
- textbooks or external snippets/algorithms;
- AI assistance disclosure if required by instructor/report convention.

Never list a source that was not consulted, and cite any borrowed code or diagram directly.

## Appendices

- Complete CFG and type matrix
- Representative source, AST, and TAC
- Full test result summary
- Build/run instructions
- Screenshots
- Contribution summary based on truthful Git history
