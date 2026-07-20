# Project State

Last updated: 21 July 2026 (Asia/Dhaka)

## Executive status

- **Stage:** M2 C11/AST foundation is completed, validated, and approved as Nayem's milestone. M3 has not started.
- **Implementation:** The source-location, shared value-type, AST construction/list/cleanup/printing, minimal Bison token interface, Makefile, and AST unit-test infrastructure exist. The lexer, complete parser, symbol table, semantic analyzer, TAC generator, and driver do not yet exist.
- **Integration:** Bison generates the shared 32-token header and the AST is ready for later parser actions, semantic analysis, and TAC generation. No source-to-TAC compiler pipeline exists yet.
- **Build:** `make`, `make test`, and `make clean` work under the verified Ubuntu 24.04 WSL2 environment. Generated files remain under ignored `build/`.
- **Tests:** M2 clean compilation passed with C11 and `-Wall -Wextra -Wpedantic`; the generated token header test and 15 AST unit tests passed, including golden printer determinism and recursive cleanup execution. This is not a memory-leak claim because no leak detector was installed or run.
- **Deadline:** 31 July 2026, no extensions. Target release freeze: 30 July 2026.
- **Immediate next task:** Preserve and verify the approved local M2 commit, then wait for explicit authorization before pushing or beginning M3.
- **Next Intended Contributor:** **Dipro**.

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

The README explicitly identifies the baseline as a template with no compiler solution. Its tree diagram is visibly mojibaked in the current checkout and its generic build/run commands do not describe an implemented program.

## Mandatory module status

| Module | Status | Evidence / gap |
| --- | --- | --- |
| Flex lexer | Missing | The generated Bison token-header boundary exists, but there is no `.l`/`.lex` scanner or lexer test yet |
| Bison parser | Token interface only | `src/parser/parser.y` declares the 32 tokens and generates the shared header; it deliberately contains no complete CFG, actions, or recovery rules |
| AST | M2 completed | `src/ast/ast.h`, `ast.c`, and `ast_print.c` provide all mandatory AST shapes, source lines, ownership, cleanup, and deterministic printing |
| Symbol table | Missing | No symbol entries or nested-scope operations |
| Semantic analyzer | Missing | No AST walk, type rules, or diagnostics |
| TAC generator | Missing | No temporaries, labels, instruction representation, or output |
| Driver/integration | Missing | No CLI, phase sequencing, exit-code policy, or executable |
| Build/test automation | M2 foundation completed | `make`, `make test`, and `make clean` build and validate only the modules that exist |

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

- `make test` currently validates the M2 generated token header and AST subsystem only: 15/15 named AST tests pass and the official-sample-shaped printer output matches `tests/expected/ast_unit.stdout` byte-for-byte after CRLF normalization.
- Recursive destruction executed successfully for nested trees; memory-leak verification remains unperformed because no leak-analysis package was approved or installed.
- All 13 test files and both example files are Markdown containing fenced C-like snippets. No extraction runner exists.
- Only the seven invalid tests contain prose `Expected` sections; no test has recorded actual output.
- Valid tests have no expected AST, TAC, stdout, or exit status.
- The mandatory invalid-expression/operator semantic case is absent.
- Type-mismatch and invalid-assignment sketches currently exercise essentially the same incompatible assignment behavior.
- Missing coverage includes float literals/operations, `&&`, `||`, `!`, `/`, `%`, `<`, `<=`, `!=`, comments, identifier boundaries, longest-match operators, associativity, initialized declarations, standalone/empty nested blocks, nested shadowing/sibling scopes, non-boolean conditions, AST output, TAC labels/jumps, and error recovery continuation.
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
- The toolchain is ready, but all post-AST compiler modules remain on the critical path with little schedule slack.
- Shimul's M1 and Nayem's M2 are genuine reviewed milestones; Dipro and Mehedi still require genuine reviewed contributions before submission.
- Semantic analysis has the highest indicative implementation weight (20%) and currently has no design/code/tests.
- The inherited README still describes the instructor template rather than Team Chonnochara's implementation.
- The fork is one README commit behind the instructor repository; review that upstream change before deciding whether to merge it.
- Presentation, report, screenshots, and individual viva evidence must develop alongside code, not after it.

## Current development stage and next action

M1 is committed and pushed. The environment gate passed. M2 is completed, validated, and approved as Nayem's milestone. The handoff is:

1. Verify the approved local M2 commit contains only its C11/AST foundation, focused tests, and related documentation.
2. Do not push the M2 commit or begin M3 until the user explicitly approves the next action.
3. Keep **Dipro** as the next intended contributor for M3 lexical analysis against the generated Bison header.

No lexer, complete parser, symbol table, semantic analysis, TAC, driver, or end-to-end compiler behavior is claimed by M2.
