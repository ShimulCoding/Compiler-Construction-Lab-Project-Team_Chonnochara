# Presentation Notes

Status: M2 provides buildable AST/token-foundation evidence and deterministic AST output. No full compiler, TAC demo, slide deck, or screenshot evidence exists yet.

## Core story

Team Chonnochara plans to implement one fixed mini-language as a sequence of cooperating compiler phases:

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
- Run `make test` and explain the two results: generated Bison token-header validation and 15 direct AST tests plus golden printer comparison.

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
