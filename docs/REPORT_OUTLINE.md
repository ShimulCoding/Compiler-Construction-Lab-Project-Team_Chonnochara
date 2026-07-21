# Project Report Outline

Status: M1 language/grammar, M2 AST/build, and M3 lexer evidence are prepared; later compiler-phase and end-to-end result chapters remain outlines. The official manual's report structure is mandatory; section numbers below are normalized for the team's final report.

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

M1 evidence prepared: `docs/LANGUAGE_SPEC.md` and `docs/GRAMMAR.md`. Still needed: implemented lexer/parser evidence, generated parser conflict result, and executable tests.

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
- borrowed `yytext` lifetime at M3; copied/converted Bison semantic values deferred explicitly to M4
- 10 golden lexer cases covering every source token, compact operator overlap, the official sample, layout/comments/locations, malformed numeric forms, invalid input, and block-comment non-support

M3 evidence: Bison-header-before-Flex build under C11 warning flags, automated 32-token set equality, exact stdout/stderr/exit goldens, and full M2 regression success.

## 6. Parser Design

- CFG implementation in Bison
- semantic values and source locations
- precedence/associativity and dangling-else strategy
- normal Bison EOF acceptance without a custom source token
- AST-building actions
- basic `error`-token recovery and synchronization
- parser conflicts/warnings and how resolved
- focused tests

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

- AST traversal and inferred types
- all six required error categories
- operator/type compatibility matrix
- statement-order visibility and condition rules
- initialized-declaration traversal, exact compatibility, type-mismatch classification, and cascade suppression
- diagnostic suppression/recovery policy
- isolated semantic tests and results

Give this chapter appropriate depth because semantic correctness has the highest indicative implementation weight.

## 9. Symbol Table

- Entry fields: name, type, scope, declaration line
- scope representation and enter/exit operations
- active lookup, same-scope insertion, and declaration history
- shadowing decision
- complexity and why the simple design suits this language
- scope/redeclaration tests

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
