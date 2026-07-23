# Presentation Notes

Status: M5 adds a working nested-scope symbol-table foundation to the M4 source-to-token-to-AST path. No semantic AST traversal, TAC pipeline, final driver, slide deck, or screenshot evidence exists yet.

## Core story

Team Chonnochara is implementing one fixed mini-language as a sequence of cooperating compiler phases:

```text
source -> Flex lexer/tokens -> Bison parser/AST -> symbol table + semantic analysis -> TAC
```

The presentation should emphasize why each phase has a narrow contract, how errors stop unsafe later phases, and how nested scopes and types are enforced before TAC generation.

M1 contract summary for a future language slide:

- three types and exactly the manual's statement/operator families;
- declarations may have one optional initializer, while later assignment remains a distinct statement;
- every brace pair is a lexical scope; blocks may be standalone or empty, and `if`/`else`/`while` bodies remain braced;
- `print` takes an identifier;
- mixed numeric operations may yield `float`, while assignments require exact types;
- unsupported forms are rejected rather than silently treated as extensions.

## Suggested short presentation flow

1. **Objective and scope:** compiler front-end through TAC; no hardware backend.
2. **Language:** three types, statements, operators, nested blocks.
3. **Architecture:** one diagram showing all six required modules and data flow.
4. **Lexer:** longest match, keyword/identifier distinction, comments, locations, invalid tokens.
5. **Parser and AST:** CFG, precedence, recovery, meaningful nodes instead of punctuation-heavy parse trees.
6. **Symbol table and semantics:** scope stack, declaration history, type matrix, clear diagnostics.
7. **TAC:** temporaries, labels, conditionals, loops, and print.
8. **Testing:** isolated phase cases plus one full source-to-TAC golden.
9. **Challenges/solutions:** only claim challenges actually encountered and logged.
10. **Live demo, lessons, and readiness for unseen input.**

## M2 AST demonstration material

- Show `AstNodeKind` selecting the active member of one `AstNode` union.
- Explain that program/block nodes own source-ordered dynamic statement lists; an empty block has a zero-length list and prints `<empty>`.
- Contrast a declaration's optional initializer child with a later assignment's required value child.
- Point out that one `If` node represents both `if` and `if-else` through an optional else block.
- Trace `a + b * 2`: the addition node owns an identifier on the left and a multiplication subtree on the right, preserving parser precedence without a parentheses node.
- Show that every printed node includes `line=<n>` so later semantic diagnostics retain source context.
- Run `make test` and explain the preserved M2 result: generated-header validation and 15 direct AST tests plus golden printer comparison.

## M3 lexer demonstration material

- Show that `src/parser/parser.y` declares the 32 source tokens once and Flex includes the generated `parser.tab.h` rather than copying token numbers.
- Trace `ifvalue`: the identifier rule matches seven characters while the `if` keyword matches two, so Flex longest match returns one identifier. Exact `if` is an equal-length tie won by the earlier keyword rule.
- Trace `!==`: `!=` is the longest first match (`NE`), followed by `=` (`ASSIGN`). The compact operator fixture makes this observable.
- Explain that exact floats require digits-dot-digits; `.5`, `5.`, and `1e10` produce `LEX_INVALID_TOKEN` instead of being accepted as extra literal forms.
- Show a code-before-comment line and the LF/CRLF test producing the same later token lines through `%option yylineno`.
- Demonstrate `@` on line 3 producing `lexical error at line 3 [LEX_INVALID_TOKEN]: invalid token '@'` and exit 1.
- Clarify that `tests/support/lexer_driver.c` is only a temporary token display tool; the future compiler driver and Bison semantic values are separate milestones.
- Current `make test` summary covers the token header, 15 AST tests/unchanged golden, and 10 lexer cases including the official sample.

## M4 parser demonstration material

- Show how the lexer fills Bison semantic values and line locations, then how grammar actions call the existing `ast_new_*` constructors.
- Trace `a + b * c`: the layered grammar reduces multiplication inside the right side of addition without using conflict-hiding precedence directives.
- Point out that equality and relational productions allow at most one operator at their tier, so `a < b < c` and `a == b != c` reject.
- Show `{}` and the nested-block golden: parser-owned temporary statement lists allow zero/multiple statements and transfer them into program/block AST nodes in source order.
- Explain that `%destructor` frees discarded identifier strings, AST nodes, and temporary lists during recovery; successful constructors take child ownership.
- Demonstrate `print ;` followed by later input: recovery synchronizes at `;` and reports a later independent error. A separate case synchronizes at `}`.
- Demonstrate `@`: Flex prints `LEX_INVALID_TOKEN`, Bison receives `YYUNDEF`, and the parser suppresses the duplicate generic syntax report for that same token.
- Show the official Section 5.5 source producing the parser-built AST and the manual's `bool ... = 5 + 3.2;` reaching an initializer subtree for future semantic rejection.
- Current `make test` summary is: header check, 15 AST tests, 10 lexer cases, and 32 parser cases with seven AST goldens and zero Bison conflicts.

## M5 symbol-table demonstration material

- Start with the automatic global scope: ID 0 and depth 0. Explain that a child increments depth, while every newly created child or sibling receives the next monotonic ID.
- Show the golden snapshot with scopes `0,1,2,3` at depths `0,1,2,1`; exited scopes remain visible as `active=false`, while the later sibling is independently active.
- Trace shadowing: global `int x`, inner `float x`, and active lookup returning the inner record. After exit, lookup naturally follows the restored parent and returns the outer record again.
- Contrast the three lookups: current scope for redeclaration, active inner-to-outer for normal resolution, and inactive history for later scope-violation evidence. History never makes an exited declaration usable.
- Explain initializer order without claiming semantics: M6 will look up the outer `x`, analyze the initializer, and only then call insertion for the inner `x`.
- Point out ownership: the table copies names; scope frames and symbols remain allocated until destruction; lookup pointers are borrowed and read-only. This keeps pointers stable and preserves history.
- Run `make test` and identify the new summary: header check, 15 AST tests, 30 symbol-table tests with repeatable golden output, 10 lexer cases, and 32 parser cases.

Indicative grading emphasis: semantics 20%; parser and TAC 15% each; lexer, AST, symbol table, documentation, and presentation 10% each. Do not omit lower-weight mandatory modules.

## Required live-demo sequence

Replace placeholders with exact commands after M10:

1. Show a clean build (`make clean && make`) in the supported environment.
2. Run one concise non-trivial valid `.mc` program containing an initialized declaration and a standalone nested block.
3. Show its readable AST and explain one precedence subtree.
4. Show semantic success and TAC; point out a temporary, conditional jump, loop back edge, and print.
5. Run an invalid-token input and identify the lexical line.
6. Run an isolated malformed statement and show parser recovery/diagnostic.
7. Run semantic failures: undeclared/out-of-scope and invalid operator or assignment.
8. Run one unseen small input supplied during rehearsal/instructor demo.
9. Show the automated test summary and repository history only if time allows.

Keep the demo inputs short enough that output can be explained live. Do not rely only on the inherited samples or screenshots.

## Architecture explanation prompts

- The lexer recognizes forms; it does not decide variable types or scope.
- Bison confirms grammar and creates the AST; punctuation is omitted from the AST.
- The symbol table answers name/scope questions while semantics enforces type/operator rules.
- TAC generation consumes only a semantically valid AST, so it can rely on resolved constructs.
- Deterministic temporary/label naming makes behavior easy to test and demonstrate.

## Initial challenge log

These are verified project-start challenges and may be used if still relevant:

- The fork began as an instructor-only template with zero implementation and zero team commits.
- At project start, native Windows lacked an active supported Flex/Bison/GNU build toolchain.
- Inherited tests are fenced Markdown sketches without actual outputs.
- The manual's nested-block requirement and initialized type-mismatch example required explicit grammar compatibility. M1 made blocks true statements, allowed empty block contents as a documented edge case, and let declaration initializers reach semantic type checking without adding bonus syntax.
- The audit began only ten days before the strict 31 July deadline.

Resolved implementation challenge: the supported WSL2/Ubuntu toolchain was installed and validated, including direct C11/Flex/Bison/GCC and GNU Make smoke builds. M2 then kept all generated artifacts under ignored `build/` while building from the Windows-mounted path containing spaces.

Resolved M4 challenge: once Flex began writing Bison's `yylval` and `yylloc`, even the lexer-only phase test needed the generated parser object at link time. The Makefile now shares that generated definition while keeping token numbers in one header. Recovery ownership was kept safe with explicit transfers and Bison destructors rather than a second AST representation.

Resolved M5 design challenge: an exited declaration must stop resolving actively but remain available to distinguish scope violation from a never-declared name. Permanent scope frames with an active flag preserve that history, while parent links keep active lookup and shadow restoration direct and explainable.

Add implementation challenges only after they occur in `DEVELOPMENT_LOG.md`; do not invent conflicts or bugs for presentation value.

## Evidence still required

- final pipeline diagram;
- slide deck;
- exact live commands;
- valid/invalid executable inputs;
- AST and TAC samples;
- build, successful run, and error screenshots;
- actual test summary;
- final challenges, solutions, and lessons learned;
- speaking assignments understood by every member.
