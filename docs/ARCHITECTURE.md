# Compiler Architecture

Status: **M5 adds the tested nested-scope symbol-table foundation to the M4 source-to-AST path. AST semantic traversal, type checking, TAC, and the final compiler driver remain unimplemented.**

## Implemented M2 foundation

- `src/common/source_location.h` defines the shared line-only `SourceLocation` value.
- `src/common/value_type.h` defines the three source-language value types.
- `src/ast/ast.h` exposes the node/operator tags, tagged `AstNode` union, `AstNodeList`, constructors, append operation, printer, and destructor.
- `src/ast/ast.c` implements allocation, owned string copies, list growth, constructor validation, and recursive destruction.
- `src/ast/ast_print.c` implements deterministic two-space-indented tree output with a line on every node.
- `src/parser/parser.y` began as M2's 32-token interface; M4 now implements the complete CFG while preserving the generated header as Flex's sole token authority.
- `Makefile` incrementally builds the AST, lexer, complete parser, and phase-test executables while confining generated output to ignored `build/`.

## Implemented M3 lexer

- `src/lexer/lexer.l` recognizes exactly the 32 Bison-defined source tokens and returns normal Bison EOF at physical end-of-input.
- `src/lexer/lexer.h` exposes scanner input reset, current line as `SourceLocation`, the current temporary lexeme, lexical-error count, and internal scanner-failure state.
- Keywords precede the identifier rule for equal-length ties; Flex longest match preserves keyword-prefixed identifiers, floats, comments, and multi-character operators.
- Spaces, tabs, carriage returns, newlines, and `//` comments are discarded; `%option yylineno` counts LF once for both LF and CRLF files.
- Invalid characters and explicitly unsupported leading-dot, trailing-dot, and exponent numeric forms produce one deterministic `LEX_INVALID_TOKEN` diagnostic and Bison's built-in `YYUNDEF` marker.
- `tests/support/lexer_driver.c` is a test-only token display program. It uses the generated constants, prints stable line/lexeme information, and stops after the first lexical error.

## Implemented M4 parser integration

- `src/parser/parser.y` implements the exact approved statement and layered-expression grammar with normal Bison EOF and no custom source token.
- Its `%union` contains only parser-stage values: AST pointers, temporary statement-list pointers, copied identifier strings, converted numeric values, source `ValueType`, and binary-operator tags.
- Flex sets Bison's line locations from `yylineno`. Parser actions convert each first line to the existing `SourceLocation` and call the M2 `ast_new_*` constructors.
- Program/block statement sequences use parser-owned temporary `AstNodeList` values. Successful attachment transfers nodes into an AST container; `%destructor` rules clean strings, nodes, and lists discarded during recovery.
- The layered grammar encodes precedence/associativity and generated with zero shift/reduce or reduce/reduce conflicts. Braced bodies avoid dangling `else` without a precedence workaround.
- Syntax diagnostics use `SYN_UNEXPECTED_TOKEN`. Recovery synchronizes at `;` or `}`; any syntax error invalidates and destroys the partial AST.
- An already-reported lexical `YYUNDEF` suppresses only its matching generic syntax callback. A later independent syntax error is still emitted.
- `src/parser/parser.h` exposes `parser_parse`; `tests/support/parser_driver.c` is a phase-test executable that prints parser-built ASTs. It is not the final compiler CLI.

## Implemented M5 symbol table

- `src/symbol_table/symbol_table.h` exposes an opaque table/symbol API with explicit scope and insertion result enums, three lookup modes, read-only symbol views, current-scope information, printing, and cleanup.
- `src/symbol_table/symbol_table.c` creates global scope ID 0/depth 0, retains scope frames in monotonic creation order, links each child to its active parent, and marks exited frames inactive instead of deleting their declarations.
- Each scope owns a declaration-ordered linked list of individually allocated symbols. A symbol stores its copied name, `ValueType`, `SourceLocation`, scope ID, and scope depth; its address remains stable while the table exists.
- Current lookup supports same-scope duplicate checks. Active lookup walks parent frames from inner to outer. History lookup searches only inactive frames from newest to oldest, providing evidence for M6's scope-violation versus undeclared distinction without resolving an inactive symbol.
- `symbol_table_print` produces address-free creation-order snapshots with active state and declaration order. `tests/unit/test_symbol_table.c` exercises 30 behaviors, and the runner compares two executions with `tests/expected/symbol_table_unit.stdout`.
- The subsystem is deliberately independent of the AST. M6 will enter/exit once per block and enforce initializer-before-insertion order; M5 emits no semantic diagnostics.

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

Headers may remain beside their implementation to keep each module self-contained. Generated Flex/Bison C/header files, dependency files, test scratch output, and executables go to ignored `build/` and remain untracked. Windows artifacts must also remain ignored if that platform is later supported.

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

- Bison's generated token header is the shared token contract. M2 created the minimal `%token` interface, and M4 completed the grammar/actions without duplicating token numbers.
- Keywords must not be returned as generic identifiers.
- Multi-character operators (`<=`, `>=`, `==`, `!=`, `&&`, `||`) must be recognized atomically.
- Numeric longest-match rules must distinguish integers and floats deliberately.
- `//` comment/newline handling must keep line locations correct; block comments are unsupported by the minimal contract.
- Dynamically allocated identifier/text values need one documented owner and destructor path.
- Prefer Flex `%option noyywrap` unless the build deliberately documents and provides the `libfl` runtime dependency.
- At physical end-of-input, the lexer returns Bison's normal EOF value (`0`/`YYEOF`); it does not return a project-defined `END` token.

Implemented M4 integration keeps the scanner non-reentrant for the one-file workflow. Flex copies identifier lexemes into token-owned storage, converts integer/float text while current, and supplies Bison locations. Successful AST actions free copied token text after AST constructors make their own name copy; discarded token text is handled by Bison destructors. The borrowed `lexer_current_lexeme()` remains available only for immediate lexer-test display.

### Parser (`src/parser/`)

Input: lexer tokens and locations.

Output: an AST program root or a failed-parse result.

The implemented grammar mirrors the documented precedence layers rather than relying on ad hoc precedence repair. It accepts standalone/empty blocks, optional declaration initializers, and the completed start symbol only at Bison's normal EOF. `error SEMICOLON` and `LBRACE error RBRACE` provide basic recovery. Recovered items are omitted from temporary statement lists, and `parser_parse` never returns a partial AST when a syntax error was recorded.

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

The implemented design uses permanent scope frames in a creation-ordered pointer array and a linked list of symbols inside each frame. Each symbol records:

```text
name, declared type, scope id/depth, declaration line
```

Operations:

```text
symbol_table_enter_scope
symbol_table_exit_scope
symbol_table_insert
symbol_table_lookup_current
symbol_table_lookup_active
symbol_table_lookup_history
symbol_table_current_scope_info
symbol_get_info
symbol_table_print
symbol_table_destroy
```

Global scope is created as ID 0/depth 0 and cannot be exited normally. Child IDs increase monotonically, while depth follows parent nesting. Exiting marks only the current child inactive and restores its parent. Inner shadowing is allowed; insertion rejects a duplicate only in the current scope and preserves the first binding. A post-block active lookup fails, while inactive-history lookup can still find the prior declaration for later `SEM_SCOPE_VIOLATION` classification.

Lookup is intentionally linear: symbols are scanned in declaration order within a scope, active resolution walks the nesting depth, and history scans exited scopes newest first. The language and coursework scale do not justify a more complex hash-table framework. The table owns all frames, symbols, and copied names; returned `const Symbol *` pointers and `SymbolInfo.name` are borrowed until table destruction.

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

- The lexer owns each copied identifier token until a successful parser action frees it after the AST constructor copies the name; Bison destructors free discarded token text.
- Parser semantic nodes/lists own their children while on the parse stack. Successful actions transfer them upward and finally into the AST root; Bison destructors clean abandoned values on error paths.
- AST nodes own duplicated identifier strings and their child/list storage.
- The symbol table owns its copied symbol names and scope records.
- TAC storage owns formatted instruction strings or structured instruction operands.
- The driver owns the phase contexts and performs cleanup on both success and error paths.

These rules must be reflected in actual constructors/destructors and Bison destructors to prevent leaks or double frees.

## Diagnostics and phase gates

Implemented diagnostic form:

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
make          build AST, symbol-table, token, lexer, and parser phase tests
make test     run M2-M4 regressions plus M5 symbol-table/golden tests
make clean    remove only the generated build/ directory
```

The dependency is `src/parser/parser.y -> build/generated/parser.tab.c + parser.tab.h -> generated lex.yy.c -> parser/lexer/AST objects -> phase-test executables`. Later milestones add semantic/TAC/compiler objects without duplicating token definitions. The quoted-path-safe runner propagates failures, normalizes tracked CRLF-sensitive goldens, and generates temporary CRLF sources under `build/test-results/` without tracking generated input.

Current `make test` evidence is one generated-header check, 15 direct AST tests with unchanged golden output, 30 symbol-table tests with identical repeated golden output, 10 lexer cases, and 32 parser cases including seven parser-built AST goldens, all required grammar boundaries, both recovery points, and lexical/syntax diagnostic integration.

Routine comparisons should write ephemeral output under `build/test-results/` so ordinary test runs do not dirty Git. To satisfy grading evidence, deliberately promote stable release/milestone results to paired curated actual-output files and record their environment/command in `TEST_MATRIX.md`.
