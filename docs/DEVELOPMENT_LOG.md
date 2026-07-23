# Development Log

Record meaningful milestones only. A log entry is evidence of work performed, not permission to attribute a Git commit to a team member.

## 21 July 2026 — Initial repository audit and memory initialization

- **Contributor/owner:** Unassigned. Codex-prepared at the repository user's request; not yet a team-member contribution.
- **Task:** Compare the complete official manual, inherited repository, and Team Chonnochara master instruction before implementation.
- **Files prepared:** `AGENTS.md` and the initial documents under `docs/`.
- **Repository inspected:** All 23 tracked files, all 26 reachable commits, branches/remotes, the complete 16-page manual, root documentation, all 13 test sketches, and both example sketches.
- **What was established:**
  - persistent start-of-task, workflow, testing, documentation, and honest Git-attribution rules;
  - an evidence-based project state and initial requirement traceability checklist;
  - a dependency-aware compiler architecture and contribution roadmap;
  - initial test, report, presentation, and viva tracking documents.
- **Why:** The inherited fork contained only the instructor template. No persistent team state, source code, build system, runnable test, or team-authored commit existed.
- **Important findings/decisions:**
  - the official manual remains the highest project authority;
  - all six compiler modules and integration are missing;
  - supplied Markdown tests are reference sketches, not executable evidence;
  - invalid expression/operator semantic coverage is absent;
  - declaration initializer, `print` operand, numeric conversion, and unary-minus behavior require an explicit language-contract decision;
  - out-of-scope versus never-declared diagnostics require retained declaration history or an equivalent design;
  - the fork was one instructor README commit behind during live inspection and has no configured `upstream` remote.
- **Checks performed:**
  - `git status`, branch/ref inspection, full history/author audit, file-introduction history, and remote comparison;
  - complete manual text extraction and review;
  - inventory/search for Makefiles, C/C++/Flex/Bison sources, generated files, binaries, and test automation;
  - native and common-path tool discovery for Git, Make, GCC/G++, Flex, Bison, Clang, and WSL;
  - attempted native `make` and direct MinGW Make invocation.
- **Result:** Audit completed. Native `make` was unavailable; direct MinGW Make reported that no Makefile exists. No compilation or functional test was possible. Implementation files were not changed or created.
- **Git result:** No commit created. Attribution must remain unassigned until a named member reviews, understands, and accepts a coherent milestone.

## 21 July 2026 — M1 mandatory language contract and formal grammar

- **Contributor/owner:** Shimul; reviewed and explicitly approved as the first Team Chonnochara contribution.
- **Task:** Finalize the smallest language, grammar, semantic rules, test conventions, and build-environment plan that satisfy the official manual.
- **New files prepared:** `docs/LANGUAGE_SPEC.md`, `docs/GRAMMAR.md`, `docs/TEST_CONVENTIONS.md`, `docs/TOOLCHAIN.md`.
- **Related memory files updated:** `AGENTS.md`, `docs/PROJECT_STATE.md`, `docs/ROADMAP.md`, `docs/DECISIONS.md`, `docs/DEVELOPMENT_LOG.md`, and relevant architecture/test/report/presentation/viva tracking documents.
- **What was defined:**
  - authoritative 32-token catalog and lexical boundaries;
  - complete strict BNF with standalone/empty nested block statements, optional declaration initializers, normal Bison EOF, and zero intended ambiguity;
  - precedence/associativity, type compatibility, exact assignment, scope, and six-category error rules;
  - deterministic CLI/output/exit-code and expected/actual evidence conventions using `.mc` as the primary extension, `.txt` compatibility, and no extension enforcement;
  - pinned Ubuntu 24.04 LTS on WSL2 setup commands and environment gate.
- **Why:** Implementation must not begin against an ambiguous or accidentally extended language. Section 5 is authoritative and the user explicitly required minimal interpretation.
- **Manual-derived corrections:** Blocks are statements and may be empty; declarations may have one initializer so the manual's semantic example reaches type checking; the parser uses normal Bison EOF; `.mc` is primary while `.txt` and other readable supplied paths are not rejected by extension.
- **Key exclusions:** No print expression/literal, comma declaration, bare expression statement, unbraced control body, numeric unary sign, block comment, exponent float, implicit storage conversion, or optional bonus syntax.
- **Validation performed:** Full manual re-read; 23 defined/traced nonterminals with all 22 non-start symbols referenced on a RHS; 32/32 source-token BNF accounting with no custom `END`; official sample plus standalone/empty-block and initialized-declaration walkthroughs; a 12-case in-memory recognizer; independent canonical LR(1)/merged-LALR construction with zero conflicts; initializer type/scope/diagnostic/TAC review; inherited-test impact review; official Microsoft/Canonical/Ubuntu toolchain-source review; document consistency/format checks.
- **Functional test result:** Not run—no compiler, Makefile, Flex/Bison toolchain, or user-approved system setup exists. M1 validations are documentary/static only.
- **System changes:** None. Only read-only WSL status/list probes ran; no WSL installation/update, Ubuntu-shell, or package-install command was executed.
- **Git result:** Recorded by the commit containing this entry with message `Shimul: Defined the mandatory language contract and formal grammar`, using Shimul's approved per-command identity. No push was performed.

## 21 July 2026 — M2 C11 build and AST foundation

- **Contributor/owner:** Nayem; reviewed and explicitly approved as the M2 owner.
- **Task:** Establish only the initial C11/Make foundation, shared source locations/value types, Bison token-header boundary, complete mandatory AST representation, constructors, cleanup, printer, and focused tests.
- **Files implemented:** `Makefile`; `src/common/source_location.h`; `src/common/value_type.h`; `src/ast/ast.h`; `src/ast/ast.c`; `src/ast/ast_print.c`; `src/parser/parser.y`; `tests/unit/test_ast.c`; `tests/unit/test_token_interface.c`; `tests/run_tests.sh`; and `tests/expected/ast_unit.stdout`.
- **Supporting changes:** `/build/` ignore rule and concise updates to project state, architecture, decisions, testing, toolchain, report, presentation, and viva documents.
- **Implemented:**
  - a line-bearing `AstNodeKind` tagged union for every required program/statement/expression/literal shape;
  - growable source-ordered statement lists supporting empty and nested blocks;
  - optional declaration initializer children without semantic type checking;
  - copied identifier ownership, failure-aware constructors/list append, and recursive destruction;
  - deterministic indentation printing with line numbers and labeled child roles;
  - one 32-token Bison source that generates the future Flex-shared header without implementing the complete parser;
  - truthful `make`, `make test`, and `make clean` targets using ignored `build/` output.
- **Why:** The parser, semantic analyzer, and TAC generator need one stable, understandable AST/lifetime contract before their own milestones can integrate safely.
- **Technical decisions:** One `If` node uses an optional else block; parentheses/semicolons have no AST node; source locations store only the required line; successful constructors/append transfer child ownership, while failure does not; internal allocation/argument failures return `NULL`/`false` rather than emitting future compiler-phase diagnostics.
- **Tests/commands:** WSL `make clean`, `make`, direct M2 executables, and `make test`; Windows token accounting, ignore checks, Git status/diff, and whitespace review.
- **Result:** C11 compilation and Bison generation completed with no warnings. The generated-header test and 15 AST tests passed, including official-sample-shaped golden printer comparison and recursive cleanup execution. No leak detector was installed/run, so no memory-leak claim is made.
- **Git handling:** Approved for one local Nayem-owned commit with message `Nayem: Established the C11 build and AST foundation`; pushing remains prohibited until separately approved.

## 21 July 2026 — M3 mandatory lexical analyzer

- **Contributor/owner:** Dipro; reviewed and explicitly approved as the M3 owner.
- **Task:** Implement only the complete mandatory Flex scanner, generated Bison-token integration, deterministic invalid-token behavior, focused lexer fixtures/goldens, and M2 regression integration.
- **Files implemented:** `src/lexer/lexer.l`, `src/lexer/lexer.h`, `tests/support/lexer_driver.c`, lexer sources under `tests/lexer/`, and token/diagnostic goldens under `tests/expected/lexer/`.
- **Supporting changes:** Incremental `Makefile`/`tests/run_tests.sh` rules plus concise project-state, architecture, decisions, testing, report, presentation, viva, and requirement-traceability updates.
- **Implemented:**
  - all 32 token kinds from the generated Bison header, with no custom `END` or duplicate production enum;
  - lowercase keywords, ASCII identifiers, exact integer/float forms, every operator/delimiter, and longest-match boundaries;
  - LF/CRLF-aware line tracking, spaces/tabs/newlines, and `//` comment removal without block-comment support;
  - line-aware `LEX_INVALID_TOKEN` diagnostics for unmatched characters and unsupported leading-dot, trailing-dot, and exponent numbers;
  - a borrowed current-lexeme/current-location API and a separate test-only token display driver that stops on the first lexical error;
  - Bison-header-before-Flex generation and warning-clean C11 compilation under existing targets.
- **Why:** M4 requires a tested character-to-token boundary before implementing the complete CFG and AST-building parser actions.
- **Technical decisions:** Return Bison's built-in `YYUNDEF` after lexical diagnostics; leave copied/converted semantic values and Bison locations to M4; use Flex rule order only for equal-length keyword/identifier ties and longest match for longer identifiers/comments/operators/floats.
- **Tests/commands:** WSL `make clean`, `make`, `make test`, direct static token-set comparison, Bash syntax validation, and Windows Git whitespace/scope inspection.
- **Result:** Bison, Flex, and GCC completed without actionable warnings. The generated-header check, all 15 AST tests, unchanged AST golden, and 10 lexer cases passed, including all token families, keyword prefixes, compact overlapping operators, LF/CRLF/comments, the official sample, block-comment non-support, invalid characters, and malformed numeric diagnostics.
- **Git handling:** Committed and pushed as `084a3df7a77071bd36fff31963d513be1221b0dd` with message `Dipro: Implemented lexical analysis for the required language tokens` after separate approvals.

## 21 July 2026 — M4 syntax analyzer and AST construction

- **Contributor/owner:** Mehedi; reviewed and explicitly approved as the M4 owner.
- **Task:** Implement only the complete mandatory Bison grammar, typed lexer/parser values and locations, AST-building actions, basic recovery, deterministic syntax diagnostics, focused parser fixtures/goldens, and M2/M3 regressions.
- **Files implemented:** Complete `src/parser/parser.y`; public `src/parser/parser.h`; test-only `tests/support/parser_driver.c`; parser fixtures under `tests/parser/`; and AST/diagnostic/exit goldens under `tests/expected/parser/`.
- **Supporting changes:** Typed/location-aware updates to `src/lexer/lexer.l` and `lexer.h`, identifier-value cleanup in `tests/support/lexer_driver.c`, incremental `Makefile`/`tests/run_tests.sh` integration, and relevant project-memory updates.
- **Implemented:**
  - the approved nonempty-program CFG with both declaration forms, assignment, identifier print, standalone/empty/nested blocks, braced `if`/`if-else`/`while`, and the full layered expression grammar;
  - copied identifier semantic values, converted integer/float values, default Bison line locations, and direct construction of the existing AST;
  - parser-owned temporary statement lists plus `%destructor` cleanup for discarded strings, nodes, and lists;
  - line-aware `SYN_UNEXPECTED_TOKEN` diagnostics and `error` recovery at semicolons and closing braces;
  - suppression of the generic syntax diagnostic caused by an already-reported lexical `YYUNDEF`, without suppressing a later independent syntax error; and
  - a test-only parser interface that prints deterministic AST output for valid syntax and uses status 1/2 for lexical/syntax failures.
- **Why:** Semantic analysis and TAC require a grammar-validated AST produced from real source text rather than direct AST construction alone.
- **Technical decisions:** Precedence/non-associativity remain structural rather than hidden by precedence directives; one Bison `%union` carries only syntax-stage values; AST constructors copy identifier text, after which parser token storage is freed; no custom `END`, symbol table, type checking, TAC, or final driver was introduced.
- **Tests/commands:** WSL `make clean`, `make`, `make test`, `make clean`; Windows Git status/diff/static token and artifact inspection.
- **Result:** Bison generated the grammar with zero shift/reduce and zero reduce/reduce conflicts; Flex/GCC emitted no actionable warnings. The header check, 15 AST tests/unchanged direct golden, 10 lexer cases, and 32 parser cases passed. Parser cases cover all 14 operators, every required statement form, the official sample, seven AST goldens, LF/generated-CRLF locations, required rejected forms, two recovery boundaries, and lexical/syntax diagnostic separation.
- **Git handling:** Committed and pushed as `6d6fc307a97d9edca6cc4f8ec47c114b3f695e25` with message `Mehedi: Implemented syntax analysis and AST construction` after separate approvals.

## 21 July 2026 — M5 nested-scope symbol table

- **Contributor/owner:** Shimul; reviewed and explicitly approved as the M5 owner.
- **Task:** Implement only the reusable nested-scope symbol-table subsystem, focused unit/golden evidence, build integration, and related documentation without AST semantic traversal or diagnostics.
- **Files implemented:** `src/symbol_table/symbol_table.h`, `src/symbol_table/symbol_table.c`, `tests/unit/test_symbol_table.c`, and `tests/expected/symbol_table_unit.stdout`.
- **Supporting changes:** Incremental `Makefile`/`tests/run_tests.sh` rules plus concise project-state, architecture, decisions, testing, report, presentation, viva, roadmap, and traceability updates.
- **Implemented:**
  - symbols with copied name, `ValueType`, declaration `SourceLocation`, scope ID, and scope depth;
  - automatic global scope ID 0/depth 0, monotonic child IDs, parent-linked active nesting, safe global-exit rejection, and retained inactive scope frames;
  - explicit current-scope, innermost-first active, and newest-inactive history lookup;
  - same-scope duplicate rejection, legal shadowing, outer restoration, sibling isolation, declaration-order retention, and initializer-before-insertion API support;
  - opaque records, stable borrowed lookup pointers, read-only symbol views, recursive ownership cleanup, and deterministic creation-order printing.
- **Why:** M6 requires one tested name/scope foundation that can distinguish active resolution, same-scope redeclaration, out-of-scope history, and never-declared names while keeping semantic policy outside the data structure.
- **Technical decisions:** Scope frames and individually allocated symbols remain alive until table destruction; exit marks a frame inactive rather than deleting it. Linear lists favor clarity at this project scale. Internal result enums report duplicate/invalid/allocation/global-exit outcomes without emitting future `SEM_...` text.
- **Tests/commands:** Baseline, focused, and final WSL validation with `make clean`, `make`, `make symbol-table-test`, direct unit execution, `make test`, and final `make clean`; Bash syntax plus Windows Git whitespace, scope, staging, and artifact checks.
- **Result:** C11 compilation emitted no actionable warnings. All 30 symbol-table tests passed, two complete outputs matched each other and the tracked golden, and the generated-header, 15 AST, 10 lexer, and 32 parser regressions remained passing. Cleanup paths executed without crashes; no leak-analysis tool was installed or run, so no leak-free claim is made.
- **Git handling:** Committed and pushed as `1f319c6b44e64fceba968a0bfbab09f55dd1a059` with message `Shimul: Implemented nested-scope symbol table management` after separate approvals.

## 24 July 2026 — M6 semantic analysis

- **Intended contributor/owner:** Nayem. Implementation and validation are prepared for review; ownership and a Git commit remain pending explicit approval.
- **Task:** Implement the complete semantic pass over parser-built ASTs without adding TAC, a final compiler driver, language extensions, or unrelated refactors.
- **Files implemented:** `src/semantic/semantic.h`, `src/semantic/semantic.c`, `tests/support/semantic_driver.c`, semantic fixtures under `tests/semantic/`, and exact diagnostic/exit goldens under `tests/expected/semantic/`.
- **Supporting changes:** Incremental `Makefile`/`tests/run_tests.sh` integration and concise updates to project state, architecture, decisions, testing, roadmap, report, presentation, viva, language-status, and requirement traceability.
- **Implemented:**
  - source-order AST traversal using a private M5 symbol table and exactly one enter/exit pair for each AST block;
  - current/active/history lookup for redeclaration, normal resolution, scope violation, and undeclared classification;
  - initializer-before-binding analysis, legal shadowing, outer restoration, sibling isolation, and post-error insertion of every non-redeclared declaration;
  - transient expression-type inference with local numeric promotion, integer-only remainder, numeric ordered comparison, compatible equality, Boolean logical operators, exact storage compatibility, and Boolean control conditions;
  - stable `SEM_UNDECLARED`, `SEM_REDECLARATION`, `SEM_SCOPE_VIOLATION`, `SEM_TYPE_MISMATCH`, `SEM_INVALID_ASSIGNMENT`, and `SEM_INVALID_OPERATOR` diagnostics with source lines and dependent-cascade suppression; and
  - a test-only parser-to-semantic driver that is silent on success, emits diagnostics on stderr, and returns semantic status 3 when errors exist.
- **Why:** TAC must consume only an AST whose names, scopes, operands, assignments, and control conditions satisfy the approved language contract. The user-approved M6 scope consolidates the roadmap's former separate declaration/use and type-checking milestones without changing the language.
- **Technical decisions:** The public analyzer returns only status plus diagnostic count and does not annotate or own the AST. An internal `{valid, ValueType}` result represents expression success/error without adding a source type. Boolean/numeric equality is `SEM_TYPE_MISMATCH`; invalid operator signatures are `SEM_INVALID_OPERATOR`; non-Boolean conditions are contextual `SEM_TYPE_MISMATCH`. Independent children/statements continue after recoverable semantic errors.
- **Tests/commands:** Focused `make semantic-test`; integrated `make test`; final required sequence is `make clean`, `make`, `make test`, `make clean`, followed by Windows Git diff/artifact checks.
- **Result:** The final clean/build/test/clean sequence passed 26 semantic cases (6 valid, 20 invalid) plus the generated-header check, 15 AST tests, 30 symbol-table tests, 10 lexer cases, and 32 parser cases. Bison reported zero conflicts, GCC/Flex/Bison emitted no actionable warnings, and final cleanup removed `build/`.
- **Git handling:** Committed and pushed as `4f302dc0b3a3ebf47c341b2cc3eb4b5415b67c2c` with message `Nayem: Implemented semantic validation for the language rules` after separate approvals.

## 24 July 2026 — M7 expression and statement TAC

- **Intended contributor/owner:** Dipro. Implementation and validation are prepared for review; ownership and a Git commit remain pending explicit approval.
- **Task:** Implement deterministic TAC for expressions and non-control-flow statements without adding labels, jumps, control-flow lowering, a final compiler driver, or language extensions.
- **Files implemented:** `src/codegen/tac.h`, `src/codegen/tac.c`, `tests/unit/test_tac.c`, `tests/support/tac_driver.c`, focused sources under `tests/tac/`, and exact output/error/exit goldens under `tests/expected/tac/`.
- **Supporting changes:** Incremental `Makefile`/`tests/run_tests.sh` integration and concise updates to project state, architecture, decisions, testing, roadmap, report, presentation, viva, and requirement traceability.
- **Implemented:**
  - a tagged, owned TAC instruction list for assignment, unary, binary, and print instructions, with recursive string cleanup and deterministic printing;
  - left-to-right AST expression lowering for literals, identifiers, `!`, and all 13 binary AST operators, with one new `t1`, `t2`, ... temporary per unary/binary result;
  - plain/initialized declarations, assignments, print, sequential statements, empty blocks, standalone/nested blocks, and no optimization or short-circuit lowering;
  - per-generation symbol-table resolution plus binding-owned storage names: globals retain source names while non-global declarations use `name@scope-id`;
  - a pre-emission reservation pass over all direct global declarations, so `t1`, `t2`, ... allocation skips both earlier and later global storage-name collisions;
  - initializer lowering before inner declaration insertion, legal shadowing, outer restoration, sibling isolation, and collision-aware counter reset for every generation; and
  - a test-only parser -> semantic gate -> TAC driver; semantic failure emits existing diagnostics and no TAC, while `if`/`while` return `TAC_UNSUPPORTED_NODE` with no partial program.
- **Why:** The manual requires linear TAC for expressions and statements, and M8 needs a deterministic owned instruction foundation before adding labels and jumps.
- **Technical decisions:** Direct literal/identifier operands emit no instruction; unary/binary operations materialize the lowest available `tN` after reserving every unqualified global name; plain declarations and empty blocks emit nothing; float operands use readable `%.15g` formatting with `.0` retained for integral floating values; logical values are materialized rather than short-circuited.
- **Tests/commands:** WSL Bash syntax validation followed by `make clean`, `make`, `make test`, `make clean`, and explicit absence check for `build/`; Windows Git diff/whitespace/artifact checks.
- **Result:** Final validation passed 14 TAC unit tests and 12 TAC integration cases plus every prior suite: generated header, 15 AST, 30 symbol-table, 10 lexer, 32 parser, and 26 semantic. Collision goldens cover global `t1` before use, after use, and as an initialized declaration. Bison reported zero conflicts, GCC/Flex/Bison emitted no actionable warnings, and final cleanup removed `build/`.
- **Git handling:** Committed and pushed as `3386ba1676f33dec81ee5a45b99fa96fdc7cfa2d` with message `Dipro: Generated TAC for expressions and statements` after separate approvals.

## 24 July 2026 — M8 control-flow TAC generation

- **Intended contributor/owner:** Mehedi. Implementation and validation are prepared for review; ownership and a Git commit remain pending explicit approval.
- **Task:** Complete mandatory TAC generation for `if`, `if-else`, `while`, and nested control flow by extending—not replacing—the M7 instruction model.
- **Files implemented:** Extended `src/codegen/tac.h`, `src/codegen/tac.c`, `tests/unit/test_tac.c`, `tests/support/tac_driver.c`, control-flow sources under `tests/tac/valid/`, and exact TAC goldens under `tests/expected/tac/`.
- **Supporting changes:** Expanded `tests/run_tests.sh`, converted the former unsupported `if`/`while` fixtures to passing cases, removed their obsolete error expectations, and updated only the approved M8 project-memory documents.
- **Implemented:**
  - structural label, unconditional-jump, and conditional-false-jump instruction kinds with copied label ownership and deterministic printing;
  - per-generation `.L1`, `.L2`, ... allocation, using a dot-prefixed namespace that cannot collide with legal source identifiers and resetting alongside temporary/scope state;
  - canonical `if` and `if-else` lowering with false/end labels, plus `while` lowering whose start label precedes all condition-expression TAC;
  - nested control flow with one code-generation scope per AST block, no extra scope for the control node, sibling branch isolation, outer restoration, and unchanged `name@scope-id` storage;
  - unchanged materialized logical-expression TAC, semantic-success gating, temporary/global collision avoidance, and no optimization or short-circuit rewrite; and
  - explicit internal failure with no returned partial program for corrupt/unknown statement kinds, while all valid current control nodes now generate successfully.
- **Why:** The manual requires labels plus conditional/unconditional jumps for `if`, `if-else`, and `while`; M8 completes that mandatory TAC library surface before the final production CLI is integrated.
- **Technical decisions:** Labels are structural instructions stored in a dedicated owned field and print as `.L<n>:`. Jumps print `goto .L<n>` or `ifFalse operand goto .L<n>`. Label allocation follows AST traversal, and the loop start is emitted before condition lowering so runtime back edges reevaluate every condition temporary.
- **Tests/commands:** Bash syntax validation; focused clean TAC build; integrated `make test`; final required clean/build/test/clean sequence and Windows Git integrity review.
- **Result:** The latest integrated run passed 17 TAC unit tests and 20 TAC integration cases plus the generated-header check, 15 AST, 30 symbol-table, 10 lexer, 32 parser, and 26 semantic cases. GCC/Flex/Bison emitted no actionable warning and Bison reported zero conflicts.
- **Git handling:** No files staged, no commit created, and nothing pushed. Proposed message after Mehedi review: `Mehedi: Integrated control-flow TAC generation`.

## Entry template

### YYYY-MM-DD — Milestone title

- **Contributor/owner:**
- **Task:**
- **Files changed:**
- **Implemented:**
- **Why:**
- **Technical decisions:**
- **Tests/commands:**
- **Result:**
- **Commit:**
