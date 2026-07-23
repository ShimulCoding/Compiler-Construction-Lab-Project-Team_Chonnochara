# Test, Input, and Output Conventions

Status: **M1 conventions approved. M7 adds exact non-control-flow TAC fixtures/goldens to the M2-M6 runner; control-flow TAC and the final compiler driver do not yet exist.**

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

M3 also provides this temporary phase-test command:

```text
./build/lexer_test <source-file>
```

It is not the final compiler driver. On valid input it prints one deterministic token line at a time, including `line=<n>` and identifier/numeric lexemes. On invalid input the production scanner writes one `LEX_INVALID_TOKEN` diagnostic; the test driver stops at that first error and exits 1. Usage/I/O failures exit 2. Physical EOF is not printed as a source token.

M4 adds a second temporary phase-test command:

```text
./build/parser_test <source-file>
```

On syntactically valid input it prints the deterministic parser-built AST and exits 0. A lexical failure exits 1, a syntax failure exits 2, and usage/I/O/internal phase-test failure exits 4. Any lexical or syntax failure suppresses AST stdout and destroys the partial tree. This remains test infrastructure, not the final semantic/TAC compiler driver.

M6 adds a third temporary phase-test command:

```text
./build/semantic_test <source-file>
```

It parses one file and runs semantic analysis only after parser success. A semantically valid input exits 0 with empty stdout/stderr. Semantic diagnostics go to stderr in source order, keep stdout empty, and produce exit 3. Lexical/syntax failures retain their earlier phase statuses. Usage, I/O, or internal failures exit 4. This is integration-test infrastructure, not the final TAC compiler.

M7 adds a fourth temporary phase-test command:

```text
./build/tac_test <source-file>
```

It runs parser -> semantic analysis -> non-control-flow TAC. Success prints only one TAC instruction per line and exits 0. Semantic failure preserves the M6 diagnostics/status and prints no TAC. An `if` or `while` reaches the explicit M7 `TAC_UNSUPPORTED_NODE` path, prints no partial TAC, and the test-only driver exits 4; M8 replaces this deferral with labels/jumps. This is not the final compiler driver.

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

M7 locks assignment/unary/binary/print TAC spelling. M8 still locks label and jump spelling; M9 integrates the `AST:`/`TAC:` headings in the final driver.

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

If a file contains errors from multiple phases, the earliest failed phase determines the exit status and later phases do not produce authoritative diagnostics. Parser recovery after a lexical marker does not add a misleading generic syntax error for the same lexeme. A genuinely independent later syntax error may still be reported, while the earlier lexical phase keeps exit status 1.

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

M2 established the runner, M3/M4 expanded it through parsing, M5 added symbol-table tests, M6 added semantics, and M7 now validates:

```text
make
make test
make clean
```

- `make test` builds first, validates the generated Bison token header, runs 15 direct-construction AST tests, 30 direct symbol-table tests, 10 lexer cases, 32 parser cases, 26 semantic cases, 14 TAC unit tests twice for determinism, and 12 TAC integration cases.
- Symbol-table cases cover global/nested/sibling scopes, monotonic IDs/depths, declaration metadata/order, current/active/history lookup, duplicates, shadowing/restoration, inactive-versus-never-declared evidence, ownership, cleanup execution, and deterministic address-free printing. They do not claim AST semantic traversal or `SEM_...` diagnostics.
- The lexer cases cover all 32 tokens, identifier/keyword boundaries, compact longest-match operators, whitespace, blank lines, code-before-comment and full-line comments, LF and generated CRLF input, the official sample, unsupported block-comment behavior, invalid characters, and malformed numeric spellings.
- Parser cases cover every required statement, all 14 operators, precedence/non-associativity, the manual initialized-declaration example, standalone/empty/nested blocks, the official sample, seven parser-built AST goldens, LF/generated-CRLF lines, required unsupported forms, semicolon/closing-brace recovery, and lexical/syntax diagnostic separation.
- Semantic cases cover valid declarations/assignments/print/control flow; numeric promotion; shadowing, outer restoration, sibling isolation, and inactive history; initializer visibility/insertion/cascade behavior; all approved operator/context/storage rules; all six diagnostic families; exact line/code/message/status goldens; and deterministic multiple-error order.
- TAC cases cover all expression operators, direct literal/identifier operands, readable numeric/Boolean formatting, initialized/plain declarations, assignment, print, precedence and left-to-right lowering, empty/standalone/nested blocks, scope-qualified shadows, restoration/sibling isolation, initializer-before-binding, global/temporary collisions before and after use, initialized `t1`, per-run counter reset, repeated output, semantic gating, and explicit `if`/`while` deferral.
- The POSIX test runner quotes paths because the Windows-mounted repository path contains spaces, normalizes tracked CRLF-sensitive goldens, and creates temporary CRLF input only under ignored `build/test-results/`.
- `make clean` removes only generated content under `build/`; it never deletes tracked expected or curated actual evidence.

Verified M7 commands: `make clean`, `make`, `make test`, and final `make clean` under Ubuntu 24.04.4 LTS on WSL2. The full compiler command remains unavailable because control-flow TAC and the final driver do not yet exist.
