# Test, Input, and Output Conventions

Status: **M1 conventions approved. M2 provides a unit-test runner and ignored routine results for the AST/token foundation; executable language fixtures and the compiler driver do not yet exist.**

These conventions make later phase tests deterministic while satisfying the manual's expected/actual-output requirement.

## 1. Compiler interface

The minimal planned command is:

```text
./build/compiler <source-file>
```

- Exactly one source file is compiled per invocation.
- `.mc` is the primary extension for project tests and examples, matching the official pipeline diagram. `.txt` source files are also accepted.
- The driver accepts the supplied readable source path and does not reject a file solely because its extension differs. Extensions organize project fixtures; they are not part of the source-language grammar.
- Source text uses UTF-8.
- Identifiers remain ASCII as defined in `docs/LANGUAGE_SPEC.md`.
- Both LF and CRLF input line endings must be accepted; golden output uses LF.
- Standard input, multiple source files, interactive mode, and extra CLI flags are not part of the M1 interface.

## 2. Successful output

For a valid program, standard output contains the readable AST followed by TAC:

```text
AST:
<deterministic indentation-based AST>
TAC:
<one deterministic TAC instruction per line>
```

- Standard error is empty.
- Exit status is `0`.
- Temporary names start at `t1` and labels at `L1` for each compiler invocation.
- Node/statement ordering follows source order.
- No memory addresses, platform paths, timestamps, or nondeterministic values appear in golden output.

Exact AST node spelling and TAC instruction spelling will be locked with their implementation milestones, but the headings and deterministic requirement are fixed now.

## 3. Failure output and exit status

Diagnostics go to standard error in source order within the active phase:

```text
<phase> error at line <n> [<stable-code>]: <human-readable description>
```

Examples:

```text
lexical error at line 4 [LEX_INVALID_TOKEN]: invalid token '@'
syntax error at line 3 [SYN_UNEXPECTED_TOKEN]: expected ';' before identifier 'x'
semantic error at line 6 [SEM_UNDECLARED]: identifier 'y' is not declared
```

| Exit status | Meaning | Later phases/output |
| ---: | --- | --- |
| `0` | Successful compilation | AST and TAC on stdout |
| `1` | One or more lexical errors | No authoritative AST, semantics, or TAC |
| `2` | One or more syntax errors and no lexical error | No semantic analysis or TAC |
| `3` | One or more semantic errors | No TAC; stdout remains empty |
| `4` | Usage or source-file I/O error | No compiler phase runs |

If a file contains errors from multiple phases, the earliest failed phase determines the exit status and later phases do not produce authoritative diagnostics. Parser recovery after a lexical marker is internal and must not add a misleading generic syntax error for the same lexeme.

Semantic analysis reports each independent occurrence while suppressing dependent cascades according to `docs/LANGUAGE_SPEC.md`.

## 4. Planned test layout

The current instructor Markdown sketches remain reference material until converted. New runnable evidence will follow this layout:

```text
tests/
|-- valid/
|   `-- <case>.mc
|-- invalid/
|   |-- lexical/
|   |   `-- <case>.mc
|   |-- syntax/
|   |   `-- <case>.mc
|   `-- semantic/
|       `-- <case>.mc
|-- expected/
|   |-- <case>.stdout
|   |-- <case>.stderr
|   `-- <case>.exit
|-- actual/                 # curated milestone/release evidence only
|   |-- <case>.stdout
|   |-- <case>.stderr
|   `-- <case>.exit
`-- run_tests.sh

build/test-results/         # routine generated comparisons; untracked
```

Case basenames must be unique across the suite. Suggested form is `<phase>_<feature>_<nn>`, for example `semantic_scope_exit_01`.

## 5. Expected and actual evidence

- `.stdout` contains exact standard output; it is an empty file when none is expected.
- `.stderr` contains exact diagnostics; it is an empty file for successful cases.
- `.exit` contains the decimal exit status followed by newline.
- Expected files are reviewed specifications.
- Routine actual files are generated under `build/test-results/` and compared byte-for-byte after LF normalization.
- Passing milestone/release results are deliberately copied to `tests/actual/` with the environment, date, command, and related commit recorded in `docs/TEST_MATRIX.md`.
- Actual output is never invented or copied from expected output without running the compiler.

## 6. Test isolation policy

- Each invalid fixture has one primary expected error category.
- Combined multi-error files are separate recovery/stress cases and do not replace isolated tests.
- A semantic fixture must be syntactically and lexically valid so it reaches semantic analysis.
- A syntax fixture uses only valid tokens unless it explicitly tests a lexer/parser boundary.
- Every mandatory semantic category has a distinct trigger:
  - never-declared/use-before-declaration;
  - same-scope redeclaration;
  - use after scope exit;
  - incompatible declaration initializer, non-Boolean condition, or incompatible equality domains;
  - exact-type assignment failure;
  - invalid operator operand types.
- The non-trivial end-to-end case must reach TAC and demonstrate initialized and uninitialized declarations, assignments, arithmetic precedence, relational/logical expressions, `while`, `if-else`, standalone nested scopes, and print.

## 7. Contract coverage required before completion

- All 32 tokens, comment/whitespace behavior, keyword boundaries, and longest-match pairs.
- Accepted and rejected numeric spellings.
- Every statement production, both declaration forms, standalone/empty nested blocks, and every explicitly unsupported grammar form.
- Precedence, left/right/non-associativity, and parentheses.
- Every valid operator type signature and representative invalid combinations.
- All nine exact storage-compatibility matrix cells, with assignment-statement and initializer diagnostics checked separately.
- Boolean-only `if` and `while` conditions.
- Global, standalone, control-body, empty, nested, and sibling block scopes; declaration-point visibility, shadowing, restoration, redeclaration, and history-based scope violation.
- Initializer visibility, exact compatibility, type-mismatch classification, cascade suppression, and TAC lowering.
- Driver acceptance of the primary `.mc` form, the documented `.txt` form, and another readable supplied path without extension-based rejection.
- AST structure and deterministic TAC for initialized declarations, assignment, standalone/empty blocks, all expression families, `if`, `if-else`, `while`, and print.
- Basic syntax recovery at a semicolon or closing brace.
- Correct phase gate, diagnostic format, and exit status.

## 8. Automated command contract

M2 establishes:

```text
make
make test
make clean
```

- At M2, `make test` builds first, validates the generated Bison token header, runs 15 direct-construction AST unit tests, compares deterministic AST stdout with a tracked golden file, prints a concise pass/fail summary, and returns nonzero on failure. Language fixture/exit-code checks are added as the compiler phases become executable.
- The POSIX test runner quotes all paths because the current Windows-mounted repository path contains spaces and normalizes a tracked CRLF golden before comparison.
- `make clean` removes only generated content under `build/`; it never deletes tracked expected or curated actual evidence.

Verified M2 command: `make clean`, `make`, and `make test` under Ubuntu 24.04.4 LTS on WSL2. The full compiler command remains unavailable because only the AST/token/build foundation exists.
