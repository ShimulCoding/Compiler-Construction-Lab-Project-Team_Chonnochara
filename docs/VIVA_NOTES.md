# Viva Notes

Status: Foundation notes. Replace proposed details with exact implemented file/function names after each milestone. Every member must understand the complete project, not only their attributed commits.

## One-minute project explanation

The project is a compiler front-end for an instructor-defined mini-language. Flex converts source characters into tokens. Bison checks the token sequence against a CFG and builds an AST. The semantic analyzer walks the AST while a scoped symbol table resolves identifiers and enforces type rules. If the program is valid, the code generator lowers the AST into Three Address Code using temporaries, labels, and jumps. TAC is the final output; no assembly or executable backend is required.

## Phase questions

### Why use Flex?

Flex expresses token patterns as regular expressions and generates the scanner. Its longest-match rule helps distinguish, for example, `>=` from `>` followed by `=`, while rule order or keyword lookup distinguishes `while` from a general identifier.

### Why use Bison?

Bison generates a parser from a context-free grammar. Grammar actions construct AST nodes, precedence/associativity resolve expression grouping, and the special `error` token can recover at safe boundaries such as `;` or `}`.

### Token stream versus AST?

Tokens are a flat sequence such as `IDENTIFIER ASSIGN INTEGER PLUS ...`. The AST is hierarchical and preserves semantic structure: `a + b * 2` has addition at the root and multiplication as the right child. Parentheses and semicolons usually disappear after influencing that structure.

### Why attach line numbers to AST nodes?

Semantic errors are discovered after parsing. The analyzer still needs the original source location to report where an undeclared use, bad assignment, or invalid operator occurred.

### What does the symbol table store?

At minimum: identifier name, declared type, scope identifier/depth, and declaration line. Operations insert in the current scope, look up from inner to outer scope, and enter/exit scopes.

### How are nested scopes handled?

Entering any block—including a standalone or empty block—pushes or activates a new scope. Lookup searches that scope then its parents. Leaving the block makes its declarations inactive. The proposed design retains declaration history so later use can be called out-of-scope rather than never declared. Inner shadowing is allowed; duplicate declarations in the same scope are rejected.

### Syntax error versus semantic error?

A syntax error violates the CFG, such as a missing semicolon. A semantic error has valid structure but violates meaning, such as using an undeclared variable or adding booleans.

### How does type checking work?

Each expression visit returns an inferred type. Numeric `int`/`float` pairs promote to `float` only within an operation; `int,int` stays `int`, `%` is integer-only, comparisons return `bool`, and logical operators require booleans. Assignment statements and declaration initializers require exact compatible types. An incompatible initializer is `SEM_TYPE_MISMATCH`; an incompatible later assignment is `SEM_INVALID_ASSIGNMENT`. An internal error type prevents one mistake from causing many misleading follow-up errors. The complete matrix is in `docs/LANGUAGE_SPEC.md`.

### Why does `print` accept only an identifier?

The project uses the smallest syntax demonstrated by the official manual: every manual print form is `print` followed by a variable name. Literal or compound-expression printing would broaden the grammar and therefore remains unsupported.

### Why are declaration initializers supported?

The manual's semantic-analysis section explicitly classifies `bool b = 5 + 3.2;` as a type mismatch. The parser must therefore accept `type name = expression;` so that input reaches semantics. This is a manual-derived compatibility requirement, not a bonus extension.

Semantic analysis first checks same-scope redeclaration, then analyzes the initializer using bindings visible before the declaration. A fresh symbol is inserted afterward even if the initializer failed, preventing later undeclared cascades; a rejected redeclaration never replaces the first binding. Exact type compatibility applies, and a valid initializer lowers to expression TAC followed by a store. A plain declaration emits no TAC.

### How does the parser recognize end-of-input?

The lexer returns Bison's normal EOF value (`0`/`YYEOF`) at physical end-of-file. Bison accepts the completed start symbol through its internal end marker; the project does not declare an `END` source token, so the authoritative catalog stays at 32.

### Which source extension is required?

`.mc` is the primary project test/example convention and `.txt` is also documented by the official pipeline. The driver receives a source path and does not reject an otherwise readable file solely by extension; extensions are not grammar tokens.

### Why generate TAC only after semantics?

TAC assumes names, operators, and types are meaningful. Generating code for an invalid AST can hide errors and produce misleading output, so semantic success is a phase gate.

### What is Three Address Code?

It is a linear intermediate representation where a complex expression is decomposed into simple instructions, commonly one operator and up to two operands. Temporaries hold intermediate results; labels and jumps express control flow.

### How are `if` and `while` lowered?

An `if` evaluates a condition and conditionally jumps to an else/end label. A `while` places a start label before its condition, jumps to an exit label when false, emits the body, then jumps back to the start.

### Parse tree versus AST?

A parse tree contains every grammar symbol and punctuation. An AST keeps only meaningful constructs and is smaller/easier for semantic analysis and TAC generation.

## Important implementation topics to fill in

For every module, add:

- exact files and main public functions;
- input/output contract;
- important structures and ownership;
- normal algorithm;
- error behavior and exit status;
- one representative test;
- one design tradeoff;
- interaction with adjacent phases.

## Likely detailed viva questions

1. How does Flex longest match distinguish a float from an integer prefix, and when does rule order matter only as an equal-length tie-breaker?
2. What happens for `abc123` and for `ifvalue`?
3. How are dangling `else`, precedence, and associativity resolved?
4. Why does a declaration become visible in statement order rather than throughout a block retroactively?
5. Why is an initialized declaration analyzed before its new binding is inserted, and what does a shadowing initializer resolve?
6. How does same-scope redeclaration differ from legal shadowing?
7. How can the analyzer distinguish out-of-scope from never declared?
8. Which operands are legal for `%`, equality, relational, and logical operators?
9. Why is mixed numeric promotion allowed inside expressions but `float f = 1;` is a type mismatch?
10. Why does `bool b = 5 + 3.2;` reach semantics rather than fail parsing?
11. What prevents cascaded diagnostics after an invalid initializer or subexpression?
12. Who owns identifier strings created by the lexer/parser, and when are they freed?
13. How does error recovery avoid an infinite parser loop?
14. How are temporary and label names made unique/deterministic?
15. Does logical TAC short-circuit? Explain the chosen strategy.
16. What output/exit code does each failed compiler phase produce?
17. Which test proves the entire required pipeline rather than one isolated phase?

Do not memorize generic answers that disagree with the final code. Update these notes continuously and rehearse by tracing an unseen program through every phase.
