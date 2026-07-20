# Compiler Architecture

Status: **M2 AST/build foundation implemented and tested; lexer, complete parser, symbol table, semantic analyzer, TAC generator, and driver remain unimplemented.**

## Implemented M2 foundation

- `src/common/source_location.h` defines the shared line-only `SourceLocation` value.
- `src/common/value_type.h` defines the three source-language value types.
- `src/ast/ast.h` exposes the node/operator tags, tagged `AstNode` union, `AstNodeList`, constructors, append operation, printer, and destructor.
- `src/ast/ast.c` implements allocation, owned string copies, list growth, constructor validation, and recursive destruction.
- `src/ast/ast_print.c` implements deterministic two-space-indented tree output with a line on every node.
- `src/parser/parser.y` is deliberately only the M2 32-token interface. Bison generates `build/generated/parser.tab.h` for future Flex use; no complete parser is claimed.
- `Makefile` builds the two M2 test executables, generates the token header, runs `tests/run_tests.sh`, and confines generated output to ignored `build/`.

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

The implemented representation uses one `AstNodeKind` tag and one `AstNode` union. Every node stores `SourceLocation`; program and block variants own dynamically grown `AstNodeList` arrays. The source-language `ValueType` contains only `int`, `float`, and `bool`; a future semantic error/unknown state must remain an analyzer-internal result rather than become a source type.

Implemented node kinds:

```text
Program, Block, Declaration, Assignment, Identifier,
IntLiteral, FloatLiteral, BoolLiteral,
BinaryOp, UnaryOp, If, While, Print
```

One `AST_NODE_IF` represents both forms through an optional `else_block`. A declaration owns its copied name, type, and optional initializer expression. Assignment and print nodes own copied names; identifier-expression nodes own their own copied occurrence names. Parentheses and semicolons have no AST nodes.

`ast_new_*` functions return `NULL` for invalid required arguments or allocation failure. A successful constructor takes ownership of child nodes; a failed constructor leaves child ownership with its caller. `ast_add_statement` accepts only program/block containers and statement-kind children, taking ownership only when append succeeds. `ast_destroy` accepts `NULL` and recursively frees lists, names, and children.

`ast_print(FILE *, const AstNode *)` prints deterministic lines such as `Declaration(line=1, type=int, name=x)` and labeled `Condition`, `Then`, `Else`, `Body`, `Value`, `Left`, `Right`, and `Operand` edges. Empty program/block lists print `<empty>`. Parentheses are already represented by expression hierarchy and therefore do not print.

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

The verified target is Ubuntu 24.04.4 LTS on WSL2. Windows owns the canonical Git worktree while WSL compiles/tests the same checkout through `/mnt/e`; native Windows compilation remains unsupported. The implemented Makefile provides:

```text
make          build the AST tests and Bison token-header validation executable
make test     run the M2 automated tests and golden AST comparison
make clean    remove only the generated build/ directory
```

The current generated dependency is `src/parser/parser.y -> build/generated/parser.tab.c + parser.tab.h -> token interface test`. M3 adds `parser.tab.h -> lexer C`; M4 completes `parser.y`, and later milestones add compiler objects/executable without duplicating token definitions. The quoted-path-safe POSIX runner propagates failures to Make and normalizes CRLF in the tracked golden before byte comparison.

Routine comparisons should write ephemeral output under `build/test-results/` so ordinary test runs do not dirty Git. To satisfy grading evidence, deliberately promote stable release/milestone results to paired curated actual-output files and record their environment/command in `TEST_MATRIX.md`.
