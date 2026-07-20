# Compiler Architecture

Status: **Proposed architecture; no compiler implementation exists yet.** Update this document to name actual files/functions as each milestone lands.

## Required pipeline

```text
source file
    |
    v
Flex lexer -- tokens + semantic values + locations
    |
    v
Bison parser -- syntax diagnostics/recovery
    |
    v
AST root -- readable AST printer
    |
    v
Semantic analyzer <--> scoped symbol table
    |
    | only when no semantic errors exist
    v
TAC generator -- temporaries + labels + linear instructions
```

The compiler stops later phases when an earlier phase cannot provide a valid contract. In particular, parser failure must not feed a malformed AST to semantics, and semantic failure must not produce authoritative TAC.

## Recommended repository layout

```text
project-root/
|-- docs/
|-- build/                 # generated and ephemeral; untracked
|-- examples/
|   |-- valid/
|   `-- invalid/
|-- src/
|   |-- lexer/
|   |   `-- lexer.l
|   |-- parser/
|   |   `-- parser.y
|   |-- ast/
|   |   |-- ast.c
|   |   `-- ast.h
|   |-- symbol_table/
|   |   |-- symbol_table.c
|   |   `-- symbol_table.h
|   |-- semantic/
|   |   |-- semantic.c
|   |   `-- semantic.h
|   |-- codegen/
|   |   |-- tac.c
|   |   `-- tac.h
|   `-- main.c
|-- tests/
|   |-- valid/
|   |-- invalid/
|   |-- expected/
|   |-- actual/
|   `-- run_tests.sh
|-- Makefile
|-- README.md
`-- AGENTS.md
```

Headers may remain beside their implementation to keep each module self-contained. Generated Flex/Bison C/header files, dependency files, test scratch output, and executables should go to `build/` and remain untracked. M2 must extend `.gitignore` for the actual output names, including Windows artifacts if that platform is later supported.

## Module contracts

### Driver

Responsibilities:

1. validate CLI arguments and open the source file;
2. initialize compiler context and diagnostics;
3. invoke parsing and obtain one program AST;
4. optionally print the AST in a stable readable format;
5. run semantic analysis and report all safely recoverable errors;
6. invoke TAC generation only after semantic success;
7. free all owned resources; and
8. return a documented nonzero status for lexical, syntax, semantic, usage, or I/O failure.

The M1 interface is one supplied source path per invocation, deterministic `AST:` then `TAC:` stdout on success, diagnostics on stderr, and phase-specific exit codes. `.mc` is the primary fixture/example convention and `.txt` is accepted, but the driver does not reject another readable path solely by extension. `docs/TEST_CONVENTIONS.md` is authoritative; M10 implements and verifies it without adding language syntax.

### Lexer (`src/lexer/`)

Input: characters from the active source stream.

Output: Bison token kinds plus typed semantic values and source locations.

Important integration rules:

- Bison's generated token header is the shared token contract. M2 creates a minimal `%token` parser interface before lexer implementation; M4 completes the grammar and actions without duplicating token numbers.
- Keywords must not be returned as generic identifiers.
- Multi-character operators (`<=`, `>=`, `==`, `!=`, `&&`, `||`) must be recognized atomically.
- Numeric longest-match rules must distinguish integers and floats deliberately.
- `//` comment/newline handling must keep line locations correct; block comments are unsupported by the minimal contract.
- Dynamically allocated identifier/text values need one documented owner and destructor path.
- Prefer Flex `%option noyywrap` unless the build deliberately documents and provides the `libfl` runtime dependency.
- At physical end-of-input, the lexer returns Bison's normal EOF value (`0`/`YYEOF`); it does not return a project-defined `END` token.

### Parser (`src/parser/`)

Input: lexer tokens and locations.

Output: an AST program root or a failed-parse result.

The grammar should mirror the documented precedence layers rather than relying on ad hoc actions. It accepts standalone/empty blocks, optional declaration initializers, and the completed start symbol only at Bison's normal EOF. Basic recovery should synchronize at safe boundaries such as semicolons or closing braces. Recovery must avoid double-freeing semantic values and must not turn a malformed subtree into valid semantic input.

### AST (`src/ast/`)

Recommended simple representation:

- one `AstNodeKind` enum;
- one tagged `AstNode` structure containing a source line and a union of kind-specific data;
- explicit child pointers/lists for programs, blocks, and statements;
- a `ValueType` enum shared carefully with semantics (`TYPE_INT`, `TYPE_FLOAT`, `TYPE_BOOL`, plus internal error/unknown states);
- constructors, indentation printer, and recursive destructor.

Core node kinds:

```text
Program, Block, Declaration, Assignment, Identifier,
IntLiteral, FloatLiteral, BoolLiteral,
BinaryOp, UnaryOp, If, While, Print
```

An `If` node can represent both forms with an optional else branch. A `Declaration` node owns its name/type and an optional initializer expression child. Parentheses and semicolons do not need AST nodes.

### Symbol table (`src/symbol_table/`)

Recommended educational design: a stack/tree of scope records, with a linked list of symbols in each scope. Each symbol records:

```text
name, declared type, scope id/depth, declaration line
```

Operations:

```text
enter_scope
exit_scope
insert_current_scope
lookup_active_from_inner_to_outer
lookup_any_declaration_history
destroy
```

Inner shadowing is allowed; redeclaration is rejected only in the same active scope. Scope records should remain queryable during the semantic pass (or a separate history should be kept) so a post-block use can be diagnosed as out-of-scope rather than never-declared.

### Semantic analyzer (`src/semantic/`)

Input: AST root.

Output: annotations/inferred expression types, populated symbol information, diagnostic count, and success/failure.

Traversal is statement-order sensitive. Every block—including a standalone or empty block—enters a scope before its statements and exits afterward. The block visitor is the sole owner of that enter/exit pair; `if`/`while` visitors delegate to their block children and must not create a duplicate scope. For a declaration, check same-scope redeclaration and analyze any initializer against the pre-declaration environment; only then insert a fresh binding into the current scope. The binding is inserted even if initializer analysis failed, preventing later undeclared cascades, but a rejected redeclaration never replaces the first binding. Expressions return an inferred type or an internal error type, allowing one root diagnostic without a cascade of misleading follow-ups.

The accepted type matrix is in `docs/LANGUAGE_SPEC.md` and requires:

- operation-local numeric arithmetic promotion;
- integer-only `%`;
- relational/equality compatibility and boolean results;
- boolean-only `&&`, `||`, and `!`;
- exact storage compatibility for assignment statements and declaration initializers, with construct-specific diagnostics;
- boolean conditions for `if`/`while`; and
- exact-type assignment, Boolean conditions, and an identifier-only `print` operand.

### TAC generator (`src/codegen/`)

Input: a semantically valid AST.

Output: deterministic linear TAC.

Expression generation returns an operand name/literal. Compound expressions emit temporaries in evaluation order. Statements append instructions. A valid initialized declaration evaluates its optional initializer and emits a store to the declared name; an uninitialized declaration and an empty block emit no TAC. A standalone populated block emits its contained statements in source order. Control flow uses monotonically numbered labels and explicit jumps.

Illustrative conventions (final spelling must be locked before golden tests):

```text
t1 = b * 2
t2 = a + t1
c = t2
ifFalse t3 goto L1
goto L2
L1:
print c
```

Logical expressions are materialized into Boolean temporaries. The M1 language contract does not promise short-circuit evaluation because its expressions contain no calls or assignment expressions. Correct deterministic logical TAC remains mandatory.

## Ownership and cleanup

- The parser transfers constructed nodes into the AST root.
- AST nodes own duplicated identifier strings and their child/list storage.
- The symbol table owns its copied symbol names and scope records.
- TAC storage owns formatted instruction strings or structured instruction operands.
- The driver owns the phase contexts and performs cleanup on both success and error paths.

These rules must be reflected in actual constructors/destructors and Bison destructors to prevent leaks or double frees.

## Diagnostics and phase gates

Recommended diagnostic form:

```text
<phase> error at line <n> [<stable-code>]: <specific message>
```

Diagnostic policy should distinguish:

- lexical invalid input;
- parser expectation/recovery errors;
- never-declared identifiers;
- declarations that exist only in inactive scopes;
- same-scope redeclarations; and
- operator errors, declaration-initializer/context type mismatches, and standalone-assignment incompatibilities.

Tests should verify diagnostic phase, line, essential wording, and exit status without depending on unstable Bison implementation text.

## Build and test flow

The primary planned path is pinned Ubuntu 24.04 LTS on WSL2, with installation requiring separate user approval and exact observed tool versions recorded afterward. Native Windows remains unsupported until independently validated. The Makefile should provide at least:

```text
make          build generated parser/lexer sources and compiler
make test     build and run the automated suite
make clean    remove only generated build artifacts
```

Generated dependencies should follow `parser.y -> parser header/C -> lexer C -> object files -> compiler`. A minimal `make test` target begins in M2 and grows with every module. The primary runner should be a quoted-path-safe POSIX shell script for the declared Linux target and must propagate failures to Make; normalize line endings where cross-platform checkouts affect comparisons.

Routine comparisons should write ephemeral output under `build/test-results/` so ordinary test runs do not dirty Git. To satisfy grading evidence, deliberately promote stable release/milestone results to paired curated actual-output files and record their environment/command in `TEST_MATRIX.md`.
