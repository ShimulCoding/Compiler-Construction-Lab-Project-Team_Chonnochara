# Project State

Last updated: 24 July 2026 (Asia/Dhaka)

## Executive status

- **Stage:** M1-M7 are committed and pushed. M8 control-flow TAC generation, tests, and documentation are implemented and validated in the working tree, pending Mehedi's review and commit approval.
- **Implementation:** The C11/AST foundation, mandatory lexer/parser-to-AST path, nested-scope symbol table, semantic analyzer, and complete mandatory TAC library now exist. The final production compiler driver does not exist; drivers under `tests/support/` remain test-only.
- **Integration:** The test path is source -> lexer -> parser -> AST -> semantic gate -> TAC. Semantically valid declarations, expressions, assignments, blocks, print, `if`, `if-else`, and `while` produce deterministic TAC with structural labels and jumps.
- **Build:** `make`, `make test`, and `make clean` work under the verified Ubuntu 24.04 WSL2 environment. Bison conflict warnings are treated as build errors, and generated files remain under ignored `build/`.
- **Tests:** The current suite passes the generated-header check, 15 AST tests, 30 symbol-table tests, 10 lexer cases, 32 parser cases, 26 semantic cases, 17 TAC unit tests, and 20 TAC integration cases with exact goldens/exits.
- **Deadline:** 31 July 2026, no extensions. Target release freeze: 30 July 2026.
- **Immediate next task:** Mehedi reviews M8 label/jump ownership, exact `if`/`if-else`/`while` lowering, loop-condition placement, nested-scope behavior, and exact goldens before any staging or commit.
- **Next Intended Contributor:** **Mehedi**.

The initialization and M1 documents describe audited facts and the finalized technical contract. They do not prove compiler implementation or functional test completion.

## Audit snapshot

Audit time: 21 July 2026, approximately 01:14 +06:00.

- Local branch: `main` at `7d073fa01fddf7cbc6b1fc5830ac709dfaee3cd6`.
- Cached `origin/main` matched local `main` with no local divergence during the audit.
- Live remote inspection found the team fork at the same commit and the instructor repository one later README-only commit ahead (`61e305a`). No upstream remote is configured locally.
- Reachable local history: 26 linear commits, all authored by instructor Khalid Bin Selim.
- Team-authored commits: none for Shimul, Nayem, Dipro, or Mehedi.
- Tracked baseline: 23 instructor-template files.
- Worktree before initialization edits: clean.
- Branches/tags: only local `main`, `origin/main`, and `origin/HEAD`; no tags or feature branches.

Do not count inherited commits or this Codex-assisted audit as a team-member contribution. A member must review, understand, and accept ownership before any related commit is attributed.

## What currently exists

| Area | Current state | Provenance |
| --- | --- | --- |
| Official requirements | Complete 16-page PDF manual | Instructor template |
| Root documentation | README, INSTALL, FAQ, contributing guide, changelog, license | Instructor template |
| `.gitignore` | Covers common Flex/Bison outputs, objects, executables, logs, and IDE files | Instructor template |
| Valid test sketches | Six Markdown files | Instructor template |
| Invalid test sketches | Seven Markdown files | Instructor template |
| Example sketches | One valid and one mixed-invalid Markdown file | Instructor template |
| Persistent project memory | Audit plus approved M1 language/grammar/test/toolchain documents | First Team Chonnochara commit; owned by Shimul |
| C11 build/test foundation | Makefile, ignored build tree, Bison-generated token header, and POSIX test runner | M2 completed; owned and reviewed by Nayem |
| AST subsystem | Tagged node model, source lines, constructors, statement lists, recursive cleanup, printer, and unit tests | M2 completed; owned and reviewed by Nayem |
| Flex lexer | Complete mandatory rules, line/location access, invalid-token diagnostics, generated-header integration, and focused goldens | M3 completed; owned and reviewed by Dipro |
| Bison parser | Complete approved CFG, semantic values/locations, AST actions, diagnostics/recovery, and focused parser goldens | M4 completed, validated, and accepted by Mehedi |
| Symbol table | Scope records, stable symbol records, current/active/history lookup, monotonic IDs, printer, cleanup, and focused golden tests | M5 committed and pushed by Shimul as `1f319c6` |
| Semantic analyzer | Source-order AST traversal, nested scopes, name/type/operator/context rules, deterministic diagnostics, and focused integration goldens | M6 committed and pushed by Nayem as `4f302dc` |
| TAC generator | Owned instruction list, deterministic temporaries/labels/printer, all required expressions/statements/control flow, and scope-safe bindings | M7 pushed as `3386ba1`; M8 implemented and validated, pending Mehedi review/approval |

The README explicitly identifies the baseline as a template with no compiler solution. Its tree diagram is visibly mojibaked in the current checkout and its generic build/run commands do not describe an implemented program.

## Mandatory module status

| Module | Status | Evidence / gap |
| --- | --- | --- |
| Flex lexer | M3 completed | `src/lexer/lexer.l` recognizes exactly the 32 source tokens, discards documented whitespace/`//` comments, tracks lines, and reports `LEX_INVALID_TOKEN`; focused goldens pass |
| Bison parser | M4 completed/validated | `src/parser/parser.y` implements the complete approved CFG, typed values, locations, AST actions, stable syntax diagnostics, and `error` recovery with zero Bison conflicts |
| AST | M2 completed | `src/ast/ast.h`, `ast.c`, and `ast_print.c` provide all mandatory AST shapes, source lines, ownership, cleanup, and deterministic printing |
| Symbol table | M5 completed | `src/symbol_table/` stores name/type/location/scope metadata, manages global/nested scopes, preserves inactive history, and passes 30 direct unit tests |
| Semantic analyzer | M6 completed | `src/semantic/` walks parser-built ASTs, enforces the approved name/scope/type/operator/context rules, and passes 26 exact integration cases |
| TAC generator | M8 implemented/validated, pending review | `src/codegen/tac.*` emits all mandatory TAC forms with `.L1` labels, conditional/unconditional jumps, and 17 unit plus 20 integration cases |
| Driver/integration | Missing | No CLI, phase sequencing, exit-code policy, or executable |
| Build/test automation | M8 expansion implemented | `make`, `make test`, and `make clean` build complete TAC unit/integration tests while preserving all M2-M7 behavior |

## Build and environment status

The separately approved environment gate passed on 21 July 2026:

- WSL `2.7.10.0`, default WSL version 2;
- Ubuntu `24.04.4 LTS` using WSL2;
- GCC `13.3.0`, GNU Make `4.3`, Flex `2.6.4`, Bison `3.8.2`, and Git `2.43.0`;
- direct C11 and Flex/Bison/GCC smoke builds passed;
- the GNU Make smoke build passed; and
- the existing Windows checkout is readable through `/mnt/e`.

Windows remains the canonical editing/Git worktree. WSL performs builds and tests against the same mounted checkout. Native Windows compilation is not a supported target.

## Test status and known coverage gaps

- `make test` validates the generated token header, 15/15 AST tests with unchanged golden output, 30/30 symbol-table tests, 10 lexer cases, 32 parser cases, 26 semantic cases, 17 TAC unit tests, and 20 TAC integration cases.
- Lexer coverage includes all 32 token kinds, lowercase keyword versus identifier boundaries, compact overlapping operators, exact integer/float forms, spaces/tabs/blank lines, code followed by comments, LF and generated CRLF input, the complete Section 5.5 sample, unsupported block-comment behavior, invalid characters, and leading-dot/trailing-dot/exponent numeric rejection.
- Recursive destruction executed successfully for nested trees; memory-leak verification remains unperformed because no leak-analysis package was approved or installed.
- The inherited Markdown sketches remain reference material rather than direct fixtures, but executable phase fixtures now cover lexical, syntax, AST, symbol-table, and semantic behavior.
- Semantic coverage includes all approved diagnostic categories, name history, declaration-point visibility, shadowing/restoration, sibling isolation, initialized-declaration cascade policy, all operator families, exact storage compatibility, Boolean conditions, valid print lookup, and multiple independent errors.
- The M6 test driver intentionally produces no stdout and returns status 3 for semantic failure. It is not the final compiler driver.
- M7 goldens remain unchanged for all expression operators, direct operands, initialized/plain declarations, assignment, print, source-order blocks, nested shadowing, outer restoration, sibling isolation, initializer-before-binding resolution, and collision-safe temporaries.
- M8 goldens add simple/expression `if`, `if-else`, `while`, condition reevaluation after loop-start labels, nested control flow in both directions, sequential/empty control blocks, branch/loop shadowing and restoration, source identifier `L1` versus label `.L1`, and a full-language control-flow program.
- Remaining functional gaps are the final production CLI, end-to-end `AST:`/`TAC:` output integration, README/deliverables, expanded final evidence, and release QA.
- The M4 integration requirement is implemented: the lexer reports `LEX_INVALID_TOKEN` and returns `YYUNDEF`; the parser suppresses the matching generic syntax report but still reports a later independent syntax error after recovery.
- The mixed-invalid example crosses lexical/syntax/semantic concerns and is unsuitable as an isolated oracle; retain it only as a later recovery stress test.

See `docs/TEST_MATRIX.md` for per-test status.

## M1 language-contract result

- Token catalog: 32 source token kinds; decimal integers, digits-dot-digits floats, ASCII identifiers, and `//` comments.
- Syntax: nonempty top-level list; declarations with an optional initializer; statement-only assignment; braced control bodies; standalone nested blocks including empty blocks; identifier-only print; no bare expression statements.
- Expressions: conventional precedence; left-associative arithmetic/logical binary operators; right-associative `!`; non-associative equality/ordering.
- Types: local mixed-numeric promotion, integer-only `%`, exact storage compatibility, Boolean conditions/logical operands; incompatible declaration initializers are type mismatches while incompatible standalone assignments are invalid assignments.
- Scope: every block establishes a scope; initializer analysis uses pre-declaration bindings; declaration-point visibility, nested shadowing, same-scope duplicate rejection, active lookup plus prior inactive history.
- Diagnostics: six non-overlapping semantic triggers with dependent-cascade suppression.
- Parser boundary: the completed start symbol is accepted through normal Bison EOF; no custom `END` token is part of the 32-token catalog.
- Tests/output: `.mc` is the primary fixture/example extension, `.txt` is accepted, and the future driver will not enforce an extension; deterministic AST/TAC stdout, line-aware stderr, phase exit codes, and expected/actual evidence rules remain fixed.
- Environment: the pinned Ubuntu 24.04 LTS on WSL2 environment was subsequently installed and passed the toolchain gate recorded above.

Authoritative details: `docs/LANGUAGE_SPEC.md`, `docs/GRAMMAR.md`, `docs/TEST_CONVENTIONS.md`, and `docs/TOOLCHAIN.md`.

## Known risks

- The audit began ten days before the 31 July deadline, leaving no schedule slack beyond the planned 30 July freeze.
- The mandatory compiler libraries now reach complete TAC, but the final driver and deliverables remain on the critical path with little schedule slack.
- M1-M7 are genuine reviewed and pushed milestones; M8 remains uncommitted until Mehedi reviews and accepts responsibility.
- Temporary/global-name collisions are covered: the generator pre-reserves every unqualified global storage name, while nested `name@scope-id` storage remains distinct.
- The inherited README still describes the instructor template rather than Team Chonnochara's implementation.
- The fork is one README commit behind the instructor repository; review that upstream change before deciding whether to merge it.
- Presentation, report, screenshots, and individual viva evidence must develop alongside code, not after it.

## Current development stage and next action

M1-M7 are committed and pushed. M8 implementation and validation are complete but uncommitted. The handoff is:

1. Mehedi reviews the three structural control instruction kinds, `.L1` namespace, label reset, exact branch/loop ordering, and one-scope-per-AST-block behavior.
2. Do not stage, commit, push, or begin M9 until the user explicitly approves the next action.
3. Keep **Mehedi** as the next intended contributor until the genuine M8 contribution is reviewed, approved, and committed.
4. After an approved M8 commit, advance the cycle to Shimul for the final production driver.

M8 completes the manual's mandatory TAC forms at the library/test-driver level. The overall project is not complete until M9 integrates the production compiler CLI and later milestones finish end-to-end evidence, documentation, demonstration material, and release validation.
