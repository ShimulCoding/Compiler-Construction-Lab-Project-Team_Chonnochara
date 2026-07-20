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
