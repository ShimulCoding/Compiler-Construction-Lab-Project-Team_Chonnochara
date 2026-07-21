# Project State

Last updated: 21 July 2026 (Asia/Dhaka)

## Executive status

- **Stage:** M3 mandatory Flex lexer is completed, validated, and approved as Dipro's milestone. M4 has not started.
- **Implementation:** The M2 C11/AST foundation and the complete mandatory lexer now exist. The complete parser, symbol table, semantic analyzer, TAC generator, and compiler driver do not yet exist; `tests/support/lexer_driver.c` is test-only.
- **Integration:** Bison generates the sole 32-token header before Flex generation and lexer compilation. The scanner returns those token constants, exposes the current line/temporary lexeme, and uses Bison's built-in `YYUNDEF` only for invalid input. No source-to-TAC pipeline exists yet.
- **Build:** `make`, `make test`, and `make clean` work under the verified Ubuntu 24.04 WSL2 environment. Generated files remain under ignored `build/`.
- **Tests:** Clean M3 compilation passed with C11 and `-Wall -Wextra -Wpedantic`; Bison/Flex emitted no actionable warnings. The generated token-header check, all 15 AST tests, unchanged AST golden, and 10 lexer cases passed, including all 32 tokens, longest match, comments, LF/CRLF locations, the official sample, malformed numeric forms, and invalid-token diagnostics.
- **Deadline:** 31 July 2026, no extensions. Target release freeze: 30 July 2026.
- **Immediate next task:** Preserve and verify the approved local M3 commit, then wait for explicit authorization before pushing or beginning M4.
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

The README explicitly identifies the baseline as a template with no compiler solution. Its tree diagram is visibly mojibaked in the current checkout and its generic build/run commands do not describe an implemented program.

## Mandatory module status

| Module | Status | Evidence / gap |
| --- | --- | --- |
| Flex lexer | M3 completed | `src/lexer/lexer.l` recognizes exactly the 32 source tokens, discards documented whitespace/`//` comments, tracks lines, and reports `LEX_INVALID_TOKEN`; focused goldens pass |
| Bison parser | Token interface only | `src/parser/parser.y` declares the 32 tokens and generates the shared header; it deliberately contains no complete CFG, actions, or recovery rules |
| AST | M2 completed | `src/ast/ast.h`, `ast.c`, and `ast_print.c` provide all mandatory AST shapes, source lines, ownership, cleanup, and deterministic printing |
| Symbol table | Missing | No symbol entries or nested-scope operations |
| Semantic analyzer | Missing | No AST walk, type rules, or diagnostics |
| TAC generator | Missing | No temporaries, labels, instruction representation, or output |
| Driver/integration | Missing | No CLI, phase sequencing, exit-code policy, or executable |
| Build/test automation | M3 expansion completed | `make`, `make test`, and `make clean` build and validate the AST, token interface, generated Flex scanner, and lexer fixtures |

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

- `make test` validates the generated token header, 15/15 AST tests with unchanged golden output, and 10 lexer cases with byte-comparable token/diagnostic goldens.
- Lexer coverage includes all 32 token kinds, lowercase keyword versus identifier boundaries, compact overlapping operators, exact integer/float forms, spaces/tabs/blank lines, code followed by comments, LF and generated CRLF input, the complete Section 5.5 sample, unsupported block-comment behavior, invalid characters, and leading-dot/trailing-dot/exponent numeric rejection.
- Recursive destruction executed successfully for nested trees; memory-leak verification remains unperformed because no leak-analysis package was approved or installed.
- All 13 test files and both example files are Markdown containing fenced C-like snippets. No extraction runner exists.
- Only the seven invalid tests contain prose `Expected` sections; no test has recorded actual output.
- Valid tests have no expected AST, TAC, stdout, or exit status.
- The mandatory invalid-expression/operator semantic case is absent.
- Type-mismatch and invalid-assignment sketches currently exercise essentially the same incompatible assignment behavior.
- Remaining functional gaps begin at parsing: no parser-produced AST, syntax recovery, semantic/scope behavior, TAC, or end-to-end compiler result exists. Lexer semantic-value transfer into Bison's future `%union`/location interface remains an explicit M4 integration task; M3 deliberately exposes only the current temporary lexeme and `SourceLocation`.
- M4 integration requirement: the lexer already reports `LEX_INVALID_TOKEN` and returns `YYUNDEF`. Parser/compiler integration must recognize that prior lexical failure and suppress a duplicate syntax diagnostic caused by the same invalid token, while still reporting independent syntax errors.
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
- The toolchain and lexer/AST foundations are ready, but the parser, semantic/scope modules, TAC, and driver remain on the critical path with little schedule slack.
- Shimul's M1, Nayem's M2, and Dipro's M3 are genuine reviewed milestones; Mehedi still requires a genuine reviewed contribution.
- Semantic analysis has the highest indicative implementation weight (20%) and currently has no design/code/tests.
- The inherited README still describes the instructor template rather than Team Chonnochara's implementation.
- The fork is one README commit behind the instructor repository; review that upstream change before deciding whether to merge it.
- Presentation, report, screenshots, and individual viva evidence must develop alongside code, not after it.

## Current development stage and next action

M1 and M2 are committed and pushed. The environment gate passed. M3 is completed, validated, and approved as Dipro's milestone. The handoff is:

1. Verify the approved local M3 commit contains only the lexer, focused tests, build integration, and related documentation.
2. Do not push the M3 commit or begin M4 until the user explicitly approves the next action.
3. Keep **Mehedi** as the next intended contributor for the complete Bison grammar and AST-building actions in M4.
4. During M4, suppress a syntax diagnostic caused solely by an already-reported lexical `YYUNDEF`, without suppressing independent syntax errors.

No complete parser, symbol table, semantic analysis, TAC, compiler driver, parser-built AST, or end-to-end compiler behavior is claimed by M3.
