# Repository Instructions for Codex and Contributors

## Authority and mission

- Treat `Compiler Construction Lab Project Manual.pdf` as the highest project authority. If another project document conflicts with the manual, follow the manual and record the conflict.
- Build a small, understandable compiler front-end whose required pipeline is:

  ```text
  source -> Flex lexer -> Bison parser -> AST -> symbol table/semantic analysis -> TAC
  ```

- Three Address Code (TAC) is the final required compiler output.
- The submission deadline is **31 July 2026, with no extensions**. Aim for a tested release by 30 July so 31 July remains a submission buffer.
- Prefer correctness, clarity, testability, documentation, and viva understanding over extra features.

## Required start-of-task protocol

Before every task or new session:

1. Read this file completely.
2. Read `docs/PROJECT_STATE.md` completely.
3. Read the relevant entries in `docs/DEVELOPMENT_LOG.md` and the relevant design/test documents.
4. Inspect `git status`, the current branch, and recent Git history.
5. Inspect every existing file relevant to the task before editing it.
6. Identify affected module interfaces and tests.
7. Preserve unrelated working code and user changes.

Do not rely on chat memory when repository memory is available. Never claim that a feature works solely because it is planned or because an illustrative test file exists.

## Fixed language specification

Do not replace or arbitrarily extend the required language.

- Types: `int`, `float`, `bool`.
- Keywords: `int`, `float`, `bool`, `if`, `else`, `while`, `print`, `true`, `false`.
- Statements: single-variable declaration with or without an initializer, assignment, braced `if`, braced `if-else`, braced `while`, identifier-only `print`, and braced block statements.
- Arithmetic operators: `+`, `-`, `*`, `/`, `%`.
- Relational operators: `<`, `>`, `<=`, `>=`, `==`, `!=`.
- Logical operators: `&&`, `||`, `!`.
- Assignment operator: `=`.
- Syntax elements: `{`, `}`, `(`, `)`, and `;`.
- Literals: integer, floating-point, `true`, and `false`.
- Identifiers begin with a letter or underscore and continue with letters, digits, or underscores.
- Whitespace and `//` line comments are discarded by the lexer.
- The program is a nonempty top-level statement sequence without required outer braces. Every block creates a nested lexical scope, and an empty block is accepted as an implementation edge case.
- Expressions occur in declaration initializers, assignment right-hand sides, conditions, and parentheses. Bare expression statements and assignment expressions are unsupported.
- Integer literals are decimal digits; floating literals require digits on both sides of one decimal point.
- Numeric unary signs, block comments, comma declarations, and print expressions/literals are unsupported.

The complete accepted contract, type matrices, and formal CFG are `docs/LANGUAGE_SPEC.md` and `docs/GRAMMAR.md`. Do not broaden them during implementation; record any required clarification in `docs/DECISIONS.md` first.

## Mandatory modules and behavior

### Flex lexer

- Recognize every keyword, identifier, literal, operator (including assignment), delimiter, parenthesis, brace, and semicolon in the fixed language.
- Discard whitespace and supported comments while maintaining line numbers.
- Use longest-match behavior correctly. Define atomic patterns for multi-character operators; Flex chooses the longest match regardless of rule order, while rule order breaks equal-length ties such as a keyword pattern versus the identifier pattern.
- Report invalid tokens clearly with a line number where possible.

### Bison parser

- Implement and document a complete, unambiguous CFG.
- Define precedence and associativity deliberately and document conflict resolution.
- Accept the completed start symbol through Bison's normal EOF handling; do not declare a custom source-language `END` token.
- Construct the AST during parsing.
- Report line-aware syntax errors and provide basic `error`-token recovery where practical.

### AST

- Represent meaningful constructs such as program, block, declaration with an optional initializer child, assignment, identifier, literal, binary operation, unary operation, if/else, while, and print.
- Preserve source-line information needed by diagnostics.
- Provide readable indentation-based printing.
- Be the shared input to semantic analysis and TAC generation.

### Symbol table

- Store at least name, type, scope, and declaration line.
- Support insert, active-scope lookup, enter scope, and exit scope.
- Enforce nested visibility and same-scope redeclaration rules.
- Distinguish a true out-of-scope use from a name that was never declared, using retained declaration history or an equally clear design, because the manual lists both diagnostics.

### Semantic analyzer

Walk the AST and report at least:

1. undeclared variable use;
2. redeclaration in the same scope;
3. scope violation;
4. type mismatch;
5. invalid assignment; and
6. invalid expression/operator usage.

Diagnostics must be human-readable and line-aware where possible. TAC must not be emitted for a semantically invalid program.

For initialized declarations, analyze the initializer before the new binding becomes visible, insert a non-redeclared symbol into the current scope after that analysis, use exact type compatibility, and classify an incompatible initializer as `SEM_TYPE_MISMATCH`. Reserve `SEM_INVALID_ASSIGNMENT` for standalone assignment statements.

### TAC generator

- Generate temporaries and labels deterministically.
- Support initialized declarations and assignment; arithmetic, relational, and logical expressions; precedence; `if`; `if-else`; `while`; blocks; and `print`.
- Use conditional and unconditional jumps for control flow.
- Do not add target-program assembly, machine code, register allocation, linking, executable/binary generation, instruction scheduling, or a hardware backend. Building the compiler tool itself is required. Do not pursue optimization beyond an optional post-core bonus.

## Architecture and implementation constraints

- Use C11 with Flex, Bison, GCC, and GNU Make. The primary planned target is pinned Ubuntu 24.04 LTS on WSL 2 as documented in `docs/TOOLCHAIN.md`; do not install it without explicit user approval or claim native Windows support until that path is separately validated.
- Keep modules small and educational. Avoid unnecessary frameworks and dependencies.
- Use meaningful names, consistent formatting, and concise comments where intent or invariants are not obvious; do not add comments that merely restate code.
- Preserve the professional structure documented in `docs/ARCHITECTURE.md`, including `src/codegen/` even though it is omitted from one suggested tree in the manual.
- Keep generated Flex/Bison sources, objects, executables, logs, credentials, and editor files out of Git unless a documented technical reason requires otherwise.
- Never add bonus features until every mandatory phase passes end-to-end tests.
- Never store passwords, tokens, private keys, or other credentials in the repository.

## Testing rules

- Use compiler-consumable source fixtures rather than relying on fenced snippets inside Markdown.
- Use `.mc` as the primary fixture/example extension and include `.txt` compatibility coverage. The compiler driver must accept a supplied readable source path without rejecting it solely by extension.
- Test a changed module first, then its integrations, then the full suite when appropriate.
- Maintain valid and invalid tests plus machine-comparable expected output.
- Record commands and actual results in `docs/TEST_MATRIX.md`; use `Not run` or `Blocked` honestly when applicable.
- At minimum, cover one non-trivial source-to-TAC success, one lexical error, one syntax error, and a separate test for each of the six mandatory semantic error classes.
- Also cover all types, literals, operators, precedence/associativity, comments, longest-match cases, nested scopes, and control flow.
- Do not hide failures, weaken assertions merely to pass, or claim an unexecuted test succeeded.
- Establish a minimal automated test target with the project foundation; expand it continuously rather than postponing automation until final integration.

## Documentation rules

Keep these files current after meaningful milestones:

- `docs/PROJECT_STATE.md`: concise handoff state and next intended contributor.
- `docs/DEVELOPMENT_LOG.md`: meaningful milestones, ownership, files, decisions, commands, and results.
- `docs/ARCHITECTURE.md`: implemented architecture and integration contracts.
- `docs/DECISIONS.md`: accepted and pending design decisions with rationale.
- `docs/TEST_MATRIX.md`: expected/actual test evidence.
- `docs/PRESENTATION_NOTES.md`: demo and presentation material.
- `docs/VIVA_NOTES.md`: explainable concepts, functions, structures, and questions.
- `docs/REPORT_OUTLINE.md`: report content and evidence.
- `docs/REQUIREMENTS_CHECKLIST.md`: manual/prompt traceability.
- `docs/ROADMAP.md`: dated, dependency-aware contribution plan.
- `docs/LANGUAGE_SPEC.md`: accepted mandatory language and semantic contract.
- `docs/GRAMMAR.md`: complete formal CFG, precedence, and manual traceability.
- `docs/TEST_CONVENTIONS.md`: stable input/output/exit/test evidence rules.
- `docs/TOOLCHAIN.md`: approved environment plan and observed tool versions.

Update the final README with project identity, features, team, structure, dependencies, exact build/run/test commands, and examples once those commands exist. Cite external resources or reused snippets in the report.

## Team contribution and Git policy

Intended contribution cycle:

```text
Shimul -> Nayem -> Dipro -> Mehedi -> repeat
```

Always read the current `Next Intended Contributor` in `docs/PROJECT_STATE.md`. Advance it only after a genuine, meaningful, tested contribution. A legitimate contribution can be implementation, testing, design, review, integration, or substantial documentation, but it must be work the named member owns, reviews, understands, or is responsible for.

Never fabricate authorship, create empty or meaningless commits, split trivial changes to inflate activity, or attribute AI-produced work to a member who has not reviewed and accepted it. If ownership is unknown, leave work uncommitted and request confirmation.

Contributor identities:

| Contributor | GitHub | Commit name | Commit email |
| --- | --- | --- | --- |
| Shimul | `ShimulCoding` | `Shimul` | `shimulc17@gmail.com` |
| Nayem | `nayem-117` | `Nayem` | `nayem692003@gmail.com` |
| Dipro | `dipro20debnath` | `Dipro` | `diprodebnath200@gmail.com` |
| Mehedi | `MdMehediHasan081` | `Mehedi` | `muhmmadmehedihasan3115@gmail.com` |

- Prefer per-command or repository-scoped Git identity; do not change global Git configuration unnecessarily.
- Before committing, review the diff, staged files, generated artifacts, secrets, author identity, and test evidence.
- Use one coherent commit per meaningful milestone, including its directly related tests and documentation.
- Commit messages use `<Contributor>: <past-tense description>`, for example `Dipro: Implemented lexical rules and diagnostics`.
- Verify commit metadata after committing. Push only when appropriate and authorized; do not rewrite published history casually.

## Required final evidence

The final repository must include buildable source, the shared GitHub history, README, report, slides, executable valid/invalid samples, expected and actual outputs, screenshots of successful build/run/error handling, and exact compilation/execution instructions. A live demonstration must show valid compilation through TAC plus lexical, syntax, and semantic failures, and the team must be ready for unseen inputs and individual viva questions.
