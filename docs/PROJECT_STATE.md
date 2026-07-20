# Project State

Last updated: 21 July 2026 (Asia/Dhaka)

## Executive status

- **Stage:** M1 language contract and formal grammar completed in the first Team Chonnochara commit owned by Shimul.
- **Implementation:** Not started. None of the six mandatory compiler modules exists.
- **Integration:** No compiler pipeline exists.
- **Build:** Blocked because there is no `Makefile`, source tree, or installed/approved WSL2 Flex-Bison-GCC-Make environment.
- **Tests:** Compiler tests are not runnable. M1 documentary/static validation passed 32-token and 23-nonterminal accounting, a 12-case grammar recognizer, zero-conflict canonical LR(1)/merged-LALR analysis, initializer semantic review, and Markdown integrity checks.
- **Deadline:** 31 July 2026, no extensions. Target release freeze: 30 July 2026.
- **Immediate next task:** Wait for user authorization before pushing or starting M2. Nayem's proposed M2 is the C/Make/AST foundation, but the separately approved toolchain gate must pass first.
- **Next Intended Contributor:** **Nayem**.

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

The README explicitly identifies the baseline as a template with no compiler solution. Its tree diagram is visibly mojibaked in the current checkout and its generic build/run commands do not describe an implemented program.

## Mandatory module status

| Module | Status | Evidence / gap |
| --- | --- | --- |
| Flex lexer | Missing | No `.l`/`.lex` file, token interface, or lexer tests |
| Bison parser | Missing | No `.y`/`.yy` file, formal CFG implementation, or recovery rules |
| AST | Missing | No node definitions, constructors, ownership rules, or printer |
| Symbol table | Missing | No symbol entries or nested-scope operations |
| Semantic analyzer | Missing | No AST walk, type rules, or diagnostics |
| TAC generator | Missing | No temporaries, labels, instruction representation, or output |
| Driver/integration | Missing | No CLI, phase sequencing, exit-code policy, or executable |
| Build/test automation | Missing | No Makefile, test runner, or CI |

## Build and environment status

Audit commands were read-only; no software was installed.

- Native Windows `PATH`: Git is available (`2.53.0.windows.1`); `make`, `flex`, `bison`, `gcc`, `g++`, and Clang were not found.
- Off-path MinGW: `C:\MinGW\bin` contains GCC/G++ `6.3.0` and `mingw32-make` `3.82.90`, but no Flex/Bison.
- `wsl.exe` reports that Windows Subsystem for Linux itself is not installed. Enabling it and installing a distribution may require administrator approval, network access, and a restart.
- `make` failed because the command is unavailable.
- Direct `C:\MinGW\bin\mingw32-make.exe` ran but failed with `No targets specified and no makefile found.`
- No compile, compiler execution, or automated test was possible.

Primary environment plan: pinned Ubuntu 24.04 LTS on WSL2 with C11, Flex, Bison, GCC, GNU Make, Git, and Bash. Exact setup commands are in `docs/TOOLCHAIN.md`; no installation was approved or performed. Before M2, separate approval is required, then observed tool versions and the environment go/no-go result must be recorded. Native Windows is unsupported unless separately validated.

## Test status and known coverage gaps

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
- Environment: pinned Ubuntu 24.04 LTS on WSL2 is planned; no installation was authorized or attempted.

Authoritative details: `docs/LANGUAGE_SPEC.md`, `docs/GRAMMAR.md`, `docs/TEST_CONVENTIONS.md`, and `docs/TOOLCHAIN.md`.

## Known risks

- The audit began ten days before the 31 July deadline, leaving no schedule slack beyond the planned 30 July freeze.
- No complete development toolchain is active.
- Installing/enabling the proposed WSL toolchain may require external approval, network access, administrator rights, and a restart; this is a go/no-go dependency for implementation.
- One meaningful Shimul contribution now exists; Nayem, Dipro, and Mehedi still require genuine reviewed contributions before submission.
- Semantic analysis has the highest indicative implementation weight (20%) and currently has no design/code/tests.
- The inherited README still describes the instructor template rather than Team Chonnochara's implementation.
- The fork is one README commit behind the instructor repository; review that upstream change before deciding whether to merge it.
- Presentation, report, screenshots, and individual viva evidence must develop alongside code, not after it.

## Current development stage and next action

M1 is complete in the commit containing this state file. The handoff is:

1. Do not push until the user explicitly approves the verified local commit.
2. Do not start M2 until the user explicitly authorizes the next milestone.
3. Obtain separate explicit approval before any WSL/Ubuntu/package installation and record observed versions when it occurs.
4. After the environment gate passes, Nayem owns M2: establish the C/Make/test foundation and implement the AST model, source locations, cleanup, printer, and focused tests.
5. Advance to Dipro only after Nayem's genuine reviewed M2 commit.

No compiler implementation is claimed by M1. The current branch contains one approved Team Chonnochara documentation/foundation contribution and remains unpushed pending user direction.
