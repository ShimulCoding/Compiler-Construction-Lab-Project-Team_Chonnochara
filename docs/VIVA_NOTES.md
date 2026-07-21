# Viva Notes

Status: M4 parser/AST integration plus the M2/M3 foundations are implemented and documented. Every member must understand the complete project, not only their attributed commits.

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

### How is the M3 lexer organized?

`src/lexer/lexer.l` is the production Flex specification. It includes Bison's generated `parser.tab.h`, returns those token constants, discards documented layout/comments, and reports invalid input. `src/lexer/lexer.h` exposes input reset, current `SourceLocation`, the current borrowed lexeme, and error count. `tests/support/lexer_driver.c` is test-only and converts returned constants to deterministic readable lines.

### Token, lexeme, and pattern—what is the difference?

A pattern is the Flex regular expression, such as `[A-Za-z_][A-Za-z0-9_]*`. A lexeme is the actual matched text, such as `value2`. A token is the category returned to Bison, such as `IDENTIFIER`.

### How does Flex choose a rule?

Flex first chooses the rule matching the most input characters. Rule order matters only when the longest lengths tie. Thus `ifvalue` is one identifier because seven characters beat the two-character `if` match, while exact `if` is a tie won by the earlier keyword rule. Similarly `!=` beats `!`, and `//...` beats `/`.

### What numeric forms does the lexer accept?

Integers use `[0-9]+`; floats use `[0-9]+\.[0-9]+`. A float match is longer than its integer prefix. Explicit rules diagnose `.5`, `5.`, and exponent spelling such as `1e10`; numeric unary signs remain separate operator tokens and are not part of literals.

### How are whitespace, comments, and lines handled?

Spaces, tabs, and carriage returns are discarded. Newline is discarded as a token, but `%option yylineno` increments the scanner line. In CRLF, `\r` is ignored and `\n` increments once. `//` consumes comment text but not its line ending, so the next token receives the correct line. Block comments are deliberately unsupported.

### How are lexical errors handled?

An unmatched character or explicit unsupported numeric form prints `lexical error at line <n> [LEX_INVALID_TOKEN]: ...`, increments the scanner error count, and returns Bison's built-in `YYUNDEF`. The M3 test driver stops at the first error and exits 1 for deterministic tests. This is not a custom source-language token.

### Who owns `yytext`?

Flex owns and reuses the `yytext` buffer. `lexer_current_lexeme()` therefore returns a borrowed pointer valid only until the next `yylex()` call. The test driver prints it immediately. M4 must copy identifier text or convert literal text before requesting another token, then attach appropriate Bison destructors.

### How does the lexer end input and integrate with Bison?

Physical EOF makes `yylex()` return `0`/`YYEOF`; no custom `END` exists. Make generates `parser.tab.h` from the current 32 `%token` declarations before Flex generates `lex.yy.c`. M4 replaces only the placeholder grammar and adds semantic/location assignments while preserving this token authority.

### What do the M3 tests prove?

Ten cases cover all 32 token kinds, keyword-prefix identifiers, compact overlapping operators, exact numeric forms, layout/comments, LF/CRLF lines, the official sample, unsupported block-comment behavior, an invalid character, and malformed numbers. They prove tokenization only—not parsing, AST construction from source, semantic validity, or TAC.

### How does the M4 parser receive values and locations?

The generated `parser.tab.h` defines the token constants, `YYSTYPE`, and `YYLTYPE`. Flex copies identifier text into `yylval.text`, converts numeric lexemes into numeric union members, and sets the token's first/last line from `yylineno`. Parser actions convert the first line to `SourceLocation` before constructing an AST node.

### How is parser precedence implemented?

The grammar uses one nonterminal per precedence tier: logical OR, logical AND, equality, relational, additive, multiplicative, unary, and primary. Left recursion makes the required binary tiers left-associative; recursive `! unary_expression` is right-associative. Equality/relational tiers contain at most one operator, so unparenthesized chains reject. Bison generated zero conflicts without precedence directives.

### How do parser statement lists and empty blocks work?

While parsing, `AstNodeList *` holds statements not yet attached to a program or block. `%empty` creates a zero-count list for `{}`. Each successful statement append transfers its node into the list; completing `program` or `block` moves the nodes into the AST container in source order and frees only the temporary list storage.

### How does parser ownership stay safe during an error?

Flex owns copied identifier token text. A successful AST constructor copies the required name, after which the action frees the token copy. AST nodes own successful child transfers. Bison `%destructor` rules free token strings, nodes, and temporary lists discarded during recovery. If any lexical or syntax error remains, `parser_parse` destroys the partial root and returns no AST. This is explicit cleanup, not a claim that a leak detector was run.

### How are syntax errors and recovery handled?

The wrapper prints `syntax error at line n [SYN_UNEXPECTED_TOKEN]: ...` using Bison's detailed expected-token message. `error SEMICOLON` skips a malformed statement through `;`; `LBRACE error RBRACE` skips malformed block contents through `}`. Both call `yyerrok`, and a recovered item is not added to the AST. Tests show parsing continues far enough to find a later independent error.

### How is a duplicate lexical/syntax diagnostic avoided?

Flex reports the invalid lexeme first and returns Bison's built-in `YYUNDEF`. The parser compares the scanner's lexical-error count with the count already seen; the first parser callback for that same marker is suppressed. Because the count is then recorded, a later syntax error with no new lexical report is printed normally.

### What is the M4 public/test interface?

`parser_parse(FILE *, AstNode **)` in `src/parser/parser.h` returns success, lexical failure, syntax failure, or an internal/test failure. `tests/support/parser_driver.c` opens one source, invokes that function, prints the AST only on success, and frees it. It is a phase-test driver; semantic analysis, TAC, and the final CLI remain later milestones.

### What do the M4 tests prove?

Thirty-two parser cases cover all required statements and 14 operators, structural precedence, the manual initializer form, standalone/empty/nested blocks, the official sample, LF/CRLF lines, selected AST goldens, required syntax rejections, recovery at `;`/`}`, and lexical/syntax diagnostic separation. They prove syntax and AST construction only, not name/type correctness or TAC.

### How is the M2 AST represented?

`src/ast/ast.h` defines one `AstNodeKind` enum and one `AstNode` tagged union. The tag tells later visitors which union member is active. Simple literals store a value, named constructs own copied strings, expressions own child pointers, and program/block nodes own an `AstNodeList`.

### How do statement lists work?

`ast_new_program` and `ast_new_block` start with an empty list. `ast_add_statement` accepts only a program/block container and a statement-kind child. It grows the pointer array with `realloc` when needed and preserves insertion/source order. A count of zero represents an empty block; a block node may itself be appended as a statement for nesting.

### What are the AST ownership rules?

Every name passed as `const char *` is copied, so the AST never borrows a lexer buffer. A successful constructor owns its child nodes; if construction fails, the caller still owns them. A successful `ast_add_statement` transfers the statement to the container. `ast_destroy(NULL)` is safe and recursively frees lists, copied names, and every owned child.

### How are initialized declarations represented?

The declaration union member stores a `ValueType`, copied name, and `initializer` pointer. `NULL` means `int x;`; an expression pointer means a form such as `int x = 1;`. The constructor checks only structural validity. Type compatibility remains the future semantic analyzer's job.

### How are `if` and `if-else` distinguished?

Both use `AST_NODE_IF`. The condition and then-block are required; `else_block == NULL` means no else, while a block pointer means `if-else`. This avoids two nearly identical node layouts.

### How does the AST printer work?

`ast_print` in `src/ast/ast_print.c` recursively switches on the node tag, writes two spaces per depth, prints each node's line, and labels roles such as `Condition`, `Then`, `Else`, `Value`, `Left`, and `Right`. Statement list order is unchanged, and `<empty>` makes an empty block visible. No address or nondeterministic value is printed.

### How does the M2 Makefile prepare Flex/Bison integration?

`src/parser/parser.y` currently contains only the 32 `%token` declarations and a clearly labeled placeholder production. Bison writes `build/generated/parser.tab.h`; both the token-interface test and M3 Flex scanner include that generated header. M4 will replace the placeholder production with the approved CFG and AST actions. Token numbers are not copied into a separate production lexer enum.

### What do the M2 tests prove and not prove?

`tests/unit/test_ast.c` directly constructs every required AST shape, checks empty/multiple/nested lists, both declaration forms, assignment, unary/binary expressions, both `if` forms, `while`, print, invalid constructor arguments, repeated printer equality, and recursive destruction. `tests/run_tests.sh` compares output with a tracked golden. These tests do not prove parsing, semantic correctness, TAC, or absence of memory leaks; no leak detector was run.

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
