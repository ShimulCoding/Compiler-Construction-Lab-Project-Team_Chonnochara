# Mandatory Language Contract

Status: **M1 contract approved and recorded by the first Team Chonnochara commit containing this file. Compiler implementation has not started.**

Authority: `Compiler Construction Lab Project Manual.pdf`, with Section 5 controlling whenever another manual example is ambiguous. This document intentionally accepts the smallest language that satisfies the manual. `docs/GRAMMAR.md` is the formal syntax companion.

## 1. Requirement classification

### 1.1 Mandatory language rules from the manual

- Types: `int`, `float`, and `bool`.
- Keywords: `int`, `float`, `bool`, `if`, `else`, `while`, `print`, `true`, and `false`.
- Statements: declaration (with or without an initializer), assignment, `if`, `if-else`, `while`, `print`, and nested braced blocks.
- Expressions: arithmetic, relational, and logical expressions using every operator listed in Section 5.3.
- Nested braced blocks with lexical scope.
- Identifiers, integer literals, floating-point literals, and Boolean literals.
- Parentheses, braces, and semicolon statement terminators.
- Whitespace and at least one supported comment form are discarded while line tracking is preserved.
- Invalid tokens, syntax errors, and the six required semantic error categories are diagnosed with line numbers where possible.

### 1.2 Implementation decisions needed for one consistent language

The manual does not formalize every boundary. The following decisions resolve only those gaps; they do not add features:

- Source keywords are lowercase and case-sensitive.
- Portable identifiers use ASCII letters: `[A-Za-z_][A-Za-z0-9_]*`.
- Integer literals are decimal digits only; floating literals require digits on both sides of one decimal point.
- Only `//` line comments are supported. The manual explicitly permits single-line and/or block comments, so one form is sufficient.
- A program is a nonempty top-level statement sequence without required outer braces.
- Every `if`, `else`, and `while` body is a braced block, and a block may also appear as a statement to provide genuine nested lexical scopes.
- A block may contain zero statements. Accepting `{ }` is an implementation edge-case decision: the manual requires nested `{ ... }` blocks but does not impose a nonempty-body restriction.
- Declarations contain exactly one identifier and may optionally contain `= expression`. This compatibility form is required so the manual's explicit semantic example `bool b = 5 + 3.2;` reaches semantic analysis.
- `print` accepts exactly one identifier without parentheses.
- Assignment is a statement, not an expression.
- Relational and equality operators are non-associative unless parentheses provide explicit grouping.
- Numeric operations may promote an `int` operand to `float` within that operation; assignments require exact types.
- `if` and `while` conditions must have type `bool`.
- Nested shadowing is allowed, but same-scope redeclaration is rejected.

### 1.3 Optional or unsupported features

These forms are not part of the mandatory language and remain unsupported:

- comma-separated declarations;
- empty source files and bare expression statements;
- unbraced `if`, `else`, or `while` bodies;
- direct `else if` syntax (nest an `if` inside an `else` block instead);
- `print` of literals or compound expressions, and `print(...)` syntax;
- unary numeric `+` or `-`, compound assignment, assignment chaining, and increment/decrement;
- leading-dot, trailing-dot, exponent, hexadecimal, binary, suffixed, or signed numeric literals;
- block comments;
- casts, strings, characters, arrays, functions, parameters, returns, `for`, `do-while`, and `switch`;
- numeric truthiness or Boolean/numeric conversion;
- constant folding, dead-code elimination, or any other optimization before mandatory completion;
- target-program machine code, assembly, register allocation, linking, executable generation, instruction scheduling, or another hardware backend.

The project still builds a compiler executable; “executable generation” above means generating an executable for the input mini-language program.

## 2. Authoritative token catalog

The lexer returns exactly these 32 source token kinds. Bison's normal end-of-input value, Bison's `error`, and any internal invalid-token marker are implementation mechanisms, not source-language tokens.

| Group | Token | Lexeme or pattern | Value carried | Manual basis |
| --- | --- | --- | --- | --- |
| Type keyword | `KW_INT` | `int` | none | §5.1 |
| Type keyword | `KW_FLOAT` | `float` | none | §5.1 |
| Type keyword | `KW_BOOL` | `bool` | none | §5.1 |
| Statement keyword | `KW_IF` | `if` | none | §5.2 |
| Statement keyword | `KW_ELSE` | `else` | none | §5.2 |
| Statement keyword | `KW_WHILE` | `while` | none | §5.2 |
| Statement keyword | `KW_PRINT` | `print` | none | §5.2 |
| Boolean literal | `KW_TRUE` | `true` | Boolean true | §§5.1, 5.4 |
| Boolean literal | `KW_FALSE` | `false` | Boolean false | §§5.1, 5.4 |
| Identifier | `IDENTIFIER` | `[A-Za-z_][A-Za-z0-9_]*` | owned identifier text | §5.4 |
| Integer literal | `INTEGER_LITERAL` | `[0-9]+` | parsed integer value | §5.4 (`42`) |
| Floating literal | `FLOAT_LITERAL` | `[0-9]+\.[0-9]+` | parsed floating value | §5.4 (`3.14`) |
| Arithmetic | `PLUS` | `+` | none | §5.3 |
| Arithmetic | `MINUS` | `-` | none | §5.3 |
| Arithmetic | `STAR` | `*` | none | §5.3 |
| Arithmetic | `SLASH` | `/` | none | §5.3 |
| Arithmetic | `PERCENT` | `%` | none | §5.3 |
| Relational | `LT` | `<` | none | §5.3 |
| Relational | `GT` | `>` | none | §5.3 |
| Relational | `LE` | `<=` | none | §5.3 |
| Relational | `GE` | `>=` | none | §5.3 |
| Equality | `EQ` | `==` | none | §5.3 |
| Equality | `NE` | `!=` | none | §5.3 |
| Logical | `AND` | `&&` | none | §5.3 |
| Logical | `OR` | `\|\|` | none | §5.3 |
| Logical | `NOT` | `!` | none | §5.3 |
| Assignment/initializer | `ASSIGN` | `=` | none | §§4.5, 5.2, 5.5 |
| Delimiter | `LBRACE` | `{` | none | §5.4 |
| Delimiter | `RBRACE` | `}` | none | §5.4 |
| Delimiter | `LPAREN` | `(` | none | §5.4 |
| Delimiter | `RPAREN` | `)` | none | §5.4 |
| Delimiter | `SEMICOLON` | `;` | none | §5.4 |

Every returned token carries a source line/location through the lexer-parser interface even when it has no semantic value.

### 2.1 Ignored input

- Spaces, horizontal tabs, carriage returns, and newlines produce no parser token.
- A line comment begins with `//` and ends immediately before newline or at end-of-input. Its text produces no token.
- Newlines inside ignored input still advance line tracking.

### 2.2 Longest-match and invalid input

- Multi-character operators are atomic. Flex longest match distinguishes `<`/`<=`, `>`/`>=`, `!`/`!=`, and `=`/`==` regardless of rule order.
- On an equal-length tie, keyword rules precede the identifier rule; therefore `if` is `KW_IF`, while `ifvalue` is one `IDENTIFIER`.
- `//` outmatches `/` at the same position and begins a comment.
- A lone `&` or `|`, `@`, a non-ASCII identifier character, or any otherwise unmatched character is a lexical error.
- `.5`, `5.`, and exponent forms are not silently accepted as floats. The lexer must diagnose the unsupported dot/sequence rather than report a successful floating literal.

## 3. Syntactic contract

- The start form is a nonempty sequence of statements. The Bison parser accepts it only when normal lexer EOF follows; no custom `END` token is declared.
- A declaration is either `type identifier ;` or `type identifier = expression ;`.
- An assignment is `identifier = expression ;`.
- An `if` is `if ( expression ) block` with an optional `else block`.
- A `while` is `while ( expression ) block`.
- A print statement is `print identifier ;`.
- A block is `{ zero or more statements }`, may itself appear as a statement, and always establishes a child lexical scope even when empty.
- Parentheses may group expressions.
- Declarations may appear wherever a statement may appear; there is no declarations-first rule.
- The complete, unambiguous grammar and rule-by-rule manual traceability appear in `docs/GRAMMAR.md`.

## 4. Expression and type contract

Each valid expression has type `int`, `float`, or `bool`. `TYPE_ERROR` may exist internally only to suppress cascaded diagnostics; it is not a language type.

### 4.1 Literals, identifiers, and grouping

| Expression | Result type |
| --- | --- |
| `INTEGER_LITERAL` | `int` |
| `FLOAT_LITERAL` | `float` |
| `true`, `false` | `bool` |
| resolved identifier | its declared type |
| parenthesized expression | inner expression type |

### 4.2 Arithmetic operators `+`, `-`, `*`, `/`

| Left | Right | Result |
| --- | --- | --- |
| `int` | `int` | `int` |
| `int` | `float` | `float` |
| `float` | `int` | `float` |
| `float` | `float` | `float` |

Any pair containing `bool` is an invalid expression/operator. Integer division has result type `int`. Mixed numeric promotion is local to the operation; it does not create a general assignment conversion.

### 4.3 Remainder operator `%`

| Left | Right | Result |
| --- | --- | --- |
| `int` | `int` | `int` |

Every other operand pair is an invalid expression/operator.

### 4.4 Ordering operators `<`, `>`, `<=`, `>=`

Any `int`/`float` pair is valid, including mixed pairs, and produces `bool`. Any pair containing `bool` is an invalid expression/operator.

### 4.5 Equality operators `==`, `!=`

| Left | Right | Result/classification |
| --- | --- | --- |
| numeric | numeric | `bool` (mixed numeric allowed) |
| `bool` | `bool` | `bool` |
| `bool` | numeric | type mismatch |
| numeric | `bool` | type mismatch |

The last row means Boolean/numeric equality is rejected. Both types are individually legal equality operands, but the pair has no compatible common domain.

### 4.6 Logical operators

| Operator | Valid operand(s) | Result |
| --- | --- | --- |
| `&&`, `\|\|` | `bool`, `bool` | `bool` |
| `!` | `bool` | `bool` |
| either operator form with numeric operand(s) | none | invalid expression/operator |

Logical TAC will use materialized Boolean values rather than introduce a short-circuit language guarantee. The fixed language has no calls or assignment expressions whose side effects would require a broader evaluation rule.

### 4.7 Storage compatibility

Assignment statements and declaration initializers use the same exact type-compatibility matrix:

| Target \ RHS | `int` | `float` | `bool` |
| --- | --- | --- | --- |
| `int` | allowed | incompatible | incompatible |
| `float` | incompatible | allowed | incompatible |
| `bool` | incompatible | incompatible | allowed |

Examples:

- `float f; f = 1;` is invalid.
- `float f = 1;` is also incompatible, but is diagnosed as an initializer type mismatch rather than an invalid assignment.
- `float f; f = 1 + 2.0;` is valid because the expression result is `float`.
- `float f = 1 + 2.0;` is a valid initialized declaration.
- `bool b; b = 1 < 2;` is valid.
- No cast syntax or implicit assignment widening is provided.

The diagnostic category depends on the construct: an incompatible standalone assignment uses `SEM_INVALID_ASSIGNMENT`, while an incompatible declaration initializer uses `SEM_TYPE_MISMATCH` because that is the manual's explicit classification for `bool b = 5 + 3.2;`.

### 4.8 Initialized declaration behavior

For `type name = expression ;`, semantic analysis follows this order:

1. Check `name` against the current scope. A prior declaration in that same scope produces `SEM_REDECLARATION`; the first valid binding remains active.
2. Analyze the initializer using bindings visible before this declaration statement. The new name is not visible inside its own initializer. Thus an outer declaration of the same name may be referenced, while an otherwise self-referential initializer is undeclared.
3. If the initializer has a valid inferred type and the declaration is not a redeclaration, compare it with the declared type using the exact matrix above. An incompatible pair produces `SEM_TYPE_MISMATCH`.
4. If the declaration is not a redeclaration, insert its symbol into the current scope after initializer analysis and compatibility checking. Insert it even when the initializer has a semantic error so later statements do not receive misleading undeclared-variable cascades.
5. If the initializer already produced an internal error type, suppress the dependent initializer-type diagnostic. For a rejected redeclaration, still inspect the initializer for independent expression/name errors but suppress compatibility checking for the rejected declaration.

An uninitialized declaration performs the same current-scope redeclaration check and then inserts the symbol immediately. Definite-assignment analysis is not required.

For a semantically valid initialized declaration, TAC evaluates the initializer in normal expression order and then stores the resulting operand in the declared name. For example, `int c = a + b * 2;` may emit temporaries for multiplication/addition followed by `c = t2`. An uninitialized declaration emits no TAC. As with every semantic failure, an invalid initializer prevents authoritative TAC for the program.

### 4.9 Context requirements

| Context | Required type |
| --- | --- |
| `if` condition | `bool` |
| `while` condition | `bool` |
| `print` operand | any declared `int`, `float`, or `bool` identifier |

Numeric truthiness is unsupported. Compile-time division/modulo-by-zero checks, overflow checks, definite-assignment analysis, and constant folding are not mandatory semantic rules and are not added.

## 5. Scope rules

1. The top-level statement sequence owns the global scope.
2. Every block creates exactly one child lexical scope, whether it is an `if`/`else`/`while` body or a standalone nested block. An empty block still enters and exits a scope; its surrounding control statement does not create a second scope for the same brace pair.
3. Then/else branches are sibling scopes; declarations do not cross between them.
4. A declaration becomes visible immediately after its complete declaration statement and remains visible to the end of that scope. There is no hoisting; an initializer is analyzed before its new binding becomes visible.
5. Lookup searches the innermost active scope outward.
6. A second declaration of the same name in the same scope is a redeclaration, even if the declared type differs.
7. A nested scope may shadow an active outer declaration. After exit, the outer declaration becomes visible again.
8. A loop body has one compile-time lexical scope; runtime iterations do not create new symbol declarations during compilation.
9. On an erroneous redeclaration, the first valid declaration remains the binding.
10. Only declarations already visited affect diagnostics. A later declaration does not retroactively resolve an earlier use.

To distinguish the manual's undeclared and scope-violation categories, lookup first checks active scopes, then previously visited inactive declaration history.

## 6. Error taxonomy and precedence

### 6.1 Compiler phases

| Phase/category | Stable code | Trigger |
| --- | --- | --- |
| Lexical error | `LEX_INVALID_TOKEN` | Input cannot form a supported token/comment/whitespace sequence |
| Syntax error | `SYN_UNEXPECTED_TOKEN` | Valid tokens do not match `docs/GRAMMAR.md` |
| Undeclared variable | `SEM_UNDECLARED` | No active prior declaration and no previously visited inactive declaration of the name |
| Redeclaration | `SEM_REDECLARATION` | The current scope already contains the declared name |
| Scope violation | `SEM_SCOPE_VIOLATION` | No active binding exists, but a previously visited declaration exists only in an inactive scope |
| Type mismatch | `SEM_TYPE_MISMATCH` | A declaration initializer conflicts with its declared type, a well-typed value is used in another non-assignment context requiring an incompatible type, or equality operands have incompatible domains |
| Invalid assignment | `SEM_INVALID_ASSIGNMENT` | A standalone assignment target and RHS resolve/type-check, but the exact compatibility matrix rejects the pair |
| Invalid expression/operator | `SEM_INVALID_OPERATOR` | At least one operand type is not admitted by that operator family (for example, `bool + int`, `float % int`, or `int && bool`); incompatible Boolean/numeric equality is reserved for `SEM_TYPE_MISMATCH` |

Isolated examples:

```text
x = 1; int x;                         SEM_UNDECLARED
int x; int x;                         SEM_REDECLARATION
if (true) { int x; } print x;         SEM_SCOPE_VIOLATION
bool b = 5 + 3.2;                     SEM_TYPE_MISMATCH
int x; bool b; b = true; x = b;       SEM_INVALID_ASSIGNMENT
int x; x = true + 1;                  SEM_INVALID_OPERATOR
```

### 6.2 Classification order and cascade suppression

1. Resolve identifiers at each occurrence.
2. Diagnose a declaration against the current scope, then analyze its optional initializer using only previously visible bindings.
3. Infer child expression types and diagnose the innermost invalid operator or incompatible equality pair.
4. Check declaration-initializer compatibility and other contextual requirements such as Boolean conditions.
5. Insert each non-redeclared declaration into its current scope after its initializer analysis.
6. Check standalone assignment compatibility.

An internal error result suppresses dependent follow-up errors. For example, `x = true + 1;` reports the invalid `+` but not a second invalid-assignment error caused by the failed RHS; similarly, `int n = true + 1;` reports `SEM_INVALID_OPERATOR` without an additional initializer type mismatch. Independent later occurrences still receive their own diagnostics.

Lexical errors prevent authoritative syntax/AST output; syntax errors prevent semantic analysis; any semantic error prevents TAC generation.

## 7. Final ambiguity resolutions

| Prior question | Final decision | Justification |
| --- | --- | --- |
| Declaration initializer | support one optional `= expression` | Manual §4.5 explicitly requires `bool b = 5 + 3.2;` to reach semantic type checking; accepting the form is a manual-derived compatibility requirement, not a bonus extension. |
| `print` operand | identifier only | Every official manual form is `print` followed by an identifier. An inherited Markdown test is not higher authority. |
| Mixed numeric expressions | operation-local `int` to `float` promotion | The §4.5 `5 + 3.2` example implies a valid numeric RHS; this does not grant assignment conversion. |
| Assignment conversion | exact types only | The manual specifies no widening, narrowing, or cast syntax. |
| `%` operands | `int`, `int` only | Smallest conventional rule without inventing floating remainder behavior. |
| Numeric unary sign | unsupported | Only logical `!` is required; additional unary operators are bonus territory in §14. |
| Logical evaluation | materialized Boolean TAC; no short-circuit guarantee | Manual requires logical TAC but does not require short circuit; the core grammar has no expression side effects. |
| Diagnostic repetition | report each independent occurrence; suppress dependent cascades | Provides clear evidence without multiplying one root error. |
| CLI/output | one supplied source path, deterministic AST/TAC, phase-specific exit codes | Required for reproducible tests; `.mc` is the primary fixture extension, `.txt` is accepted, and the driver does not reject another extension solely by name. |
| Block/statement forms | blocks are statements and may be empty; control bodies remain braced | Section 5.2 explicitly lists nested blocks. Empty `{ }` is a conflict-free edge-case completion because the manual gives no nonempty restriction. Bare expression statements remain unsupported. |
| Parser completion | normal Bison EOF | EOF is parser control rather than a source token; the catalog remains exactly 32 tokens. |
| Floating spelling | `[0-9]+\.[0-9]+` only | Matches the only official form without adding C literal syntax. |
| Semantic labels | use the non-overlapping triggers in Section 6 | Allows one isolated test per mandatory manual category. |

## 8. Manual examples and inherited sketches

- The complete Section 5.5 sample is accepted by this contract.
- Standalone blocks such as `{ int x; { bool ready = true; } }` are accepted and create one scope per brace pair; `{ }` is also valid.
- The manual's `bool b = 5 + 3.2;` is syntactically accepted, reaches semantic analysis, infers a `float` initializer, and produces `SEM_TYPE_MISMATCH` against declared type `bool`.
- Valid initialized declarations such as `int x = 1;`, `float f = 1.0;`, and `bool ok = x < 2;` are accepted and lower to initializer-expression TAC followed by a store.
- `tests/valid/complete_program.md` is a suitable future end-to-end seed.
- `tests/valid/if_else.md` contains `print 0;`, which is outside this finalized grammar and must be revised or reclassified when executable fixtures are created.
- `tests/invalid/type_mismatch.md` and `tests/invalid/invalid_assignment.md` both remain assignment-error sketches. The manual's initialized declaration supplies the primary distinct type-mismatch fixture; non-Boolean conditions and incompatible equality provide additional coverage.
