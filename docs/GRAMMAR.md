# Formal Context-Free Grammar

Status: **M1 grammar approved and recorded by the first Team Chonnochara commit containing this file; not yet implemented in Bison.**

Authority: Section 5 of `Compiler Construction Lab Project Manual.pdf`, together with the manual's explicit semantic initializer example in §4.5. This grammar includes only manual-required/manual-derived forms plus documented consistency edge decisions such as allowing empty block contents. Semantic constraints are in `docs/LANGUAGE_SPEC.md`.

## 1. Notation

- Angle-bracket names are nonterminals.
- Uppercase names are the tokens defined in `docs/LANGUAGE_SPEC.md`.
- `ε` is the empty grammar production.
- The start symbol is accepted only when Bison receives its normal end-of-input value (`YYEOF`, token number `0`). EOF is parser control, not a declared token or source lexeme.
- Conceptually Bison augments the grammar with `$accept -> <program> $end`; `$accept` and `$end` are parser internals and are not part of the BNF or 32-token catalog.
- Comments and whitespace are removed by the lexer and do not appear in the CFG.
- Bison recovery productions using its special `error` symbol are implementation mechanisms and are not valid-language productions.

## 2. Complete strict BNF

```bnf
<program> ::= <statement-list>

<statement-list> ::= <statement>
                   | <statement-list> <statement>

<statement> ::= <declaration> SEMICOLON
              | <assignment> SEMICOLON
              | <if-statement>
              | <while-statement>
              | <print-statement> SEMICOLON
              | <block>

<declaration> ::= <type> IDENTIFIER
                | <type> IDENTIFIER ASSIGN <expression>

<type> ::= KW_INT
         | KW_FLOAT
         | KW_BOOL

<assignment> ::= IDENTIFIER ASSIGN <expression>

<if-statement> ::= KW_IF LPAREN <expression> RPAREN
                   <block> <else-part>

<else-part> ::= KW_ELSE <block>
              | ε

<while-statement> ::= KW_WHILE LPAREN <expression> RPAREN
                      <block>

<print-statement> ::= KW_PRINT IDENTIFIER

<block> ::= LBRACE <optional-statement-list> RBRACE

<optional-statement-list> ::= ε
                            | <statement-list>

<expression> ::= <logical-or-expression>

<logical-or-expression> ::= <logical-and-expression>
                          | <logical-or-expression> OR
                            <logical-and-expression>

<logical-and-expression> ::= <equality-expression>
                           | <logical-and-expression> AND
                             <equality-expression>

<equality-expression> ::= <relational-expression>
                        | <relational-expression>
                          <equality-operator>
                          <relational-expression>

<equality-operator> ::= EQ
                      | NE

<relational-expression> ::= <additive-expression>
                          | <additive-expression>
                            <relational-operator>
                            <additive-expression>

<relational-operator> ::= LT
                        | GT
                        | LE
                        | GE

<additive-expression> ::= <multiplicative-expression>
                        | <additive-expression> PLUS
                          <multiplicative-expression>
                        | <additive-expression> MINUS
                          <multiplicative-expression>

<multiplicative-expression> ::= <unary-expression>
                              | <multiplicative-expression> STAR
                                <unary-expression>
                              | <multiplicative-expression> SLASH
                                <unary-expression>
                              | <multiplicative-expression> PERCENT
                                <unary-expression>

<unary-expression> ::= NOT <unary-expression>
                     | <primary-expression>

<primary-expression> ::= IDENTIFIER
                       | INTEGER_LITERAL
                       | FLOAT_LITERAL
                       | KW_TRUE
                       | KW_FALSE
                       | LPAREN <expression> RPAREN
```

## 3. Precedence and associativity

Lowest precedence is listed first:

| Level | Operators/forms | Associativity | Grammar tier |
| ---: | --- | --- | --- |
| 1 | `\|\|` | left | `<logical-or-expression>` |
| 2 | `&&` | left | `<logical-and-expression>` |
| 3 | `==`, `!=` | non-associative | `<equality-expression>` |
| 4 | `<`, `>`, `<=`, `>=` | non-associative | `<relational-expression>` |
| 5 | `+`, `-` | left | `<additive-expression>` |
| 6 | `*`, `/`, `%` | left | `<multiplicative-expression>` |
| 7 | `!` | right | `<unary-expression>` |
| 8 | `( expression )`, literals, identifiers | explicit/atomic | `<primary-expression>` |

`=` is not an expression operator, so it has no precedence or associativity. It appears only as statement-level syntax in `<assignment>` and the initialized `<declaration>` alternative.

The grammar encodes precedence structurally. Bison precedence declarations may mirror the table for documentation/defensive conflict reporting, but they must not conceal an ambiguous expression grammar.

Non-associative equality and ordering deliberately reject unrequired chains such as `a < b < c` and `a == b != c`. Parentheses can create an explicit nested expression; semantic analysis then checks the resulting operand types.

## 4. Production-to-manual traceability

Every accepted production was checked against the official manual before inclusion.

| Production family | Manual evidence | Classification and minimal boundary |
| --- | --- | --- |
| `<program>` / top-level list | §5.5 complete sample | Mandatory nonempty top-level statement sequence. Bison accepts the completed start symbol only at its normal EOF; no custom source token or specially required outer wrapper is added. A block may still be the sole top-level statement. |
| `<statement-list>` | §5.2 supported forms and the repeated sequence in §5.5 | Nonempty repetition represents a program or a populated block. |
| `<statement>` | §5.2 supported forms, including nested blocks | Selects declaration, assignment, `if`/`if-else`, `while`, `print`, or a braced block. Bare expression statements are not added. |
| `<declaration>` / `<type>` | §§5.1, 5.2 `int x;`; §4.5 `bool b = 5 + 3.2;` | Mandatory declaration plus the initializer form required to reach the manual's semantic type-mismatch case. One identifier only; no comma list. |
| `<assignment>` | §5.2: `x = 5;`; §5.5 | Mandatory statement form. Chaining and assignment expressions are not added. |
| `<if-statement>` / `<else-part>` | §5.2; exact braced forms in §5.5 | Both `if` and `if-else` mandatory. Only braced bodies are accepted. |
| `<while-statement>` | §5.2; exact form in §5.5 | Mandatory. Only a braced body is accepted. |
| `<print-statement>` | §4.6 `print a`; §5.5 `print y`/`print x` | Mandatory identifier form. Literal/expression/parenthesized forms are not added. |
| `<block>` / `<optional-statement-list>` | §§4.4, 5.2 nested `{ ... }`, 5.5 | Every brace pair is a lexical scope and may appear as a statement. Empty `{ }` is an implementation edge-case completion because the manual imposes no nonempty restriction. |
| `<expression>` | §5.3 arithmetic, relational, and logical expressions | Entry wrapper for the complete precedence hierarchy; assignment is deliberately excluded. |
| `<logical-or-expression>` | §5.3 `\|\|` | Mandatory operator; conventional precedence is an implementation decision. |
| `<logical-and-expression>` | §5.3 `&&` | Mandatory operator; conventional precedence is an implementation decision. |
| `<equality-expression>` | §5.3 `== !=`; Boolean equality in §5.5 | Mandatory. Chaining is not required, so the production permits at most one unparenthesized equality operator. |
| `<equality-operator>` | §5.3 `== !=` | Names exactly the two mandatory equality tokens without adding another comparison form. |
| `<relational-expression>` | §5.3 `< > <= >=`; condition in §5.5 | Mandatory. Chaining is not required, so the production permits at most one unparenthesized ordering operator. |
| `<relational-operator>` | §5.3 `< > <= >=` | Names exactly the four mandatory ordering tokens. |
| `<additive-expression>` | §5.3 `+ -`; §5.5 arithmetic | Mandatory; left associativity is the conventional deterministic decision. |
| `<multiplicative-expression>` | §5.3 `* / %`; TAC precedence requirement in §4.6 | Mandatory; placed above additive operators to satisfy precedence. |
| `<unary-expression>` | §5.3 logical `!` | Mandatory logical unary operator. Numeric unary signs are not added. |
| `<primary-expression>` | §5.4 identifiers, three literal families, parentheses | Mandatory atomic/grouped forms. |

## 5. Grammar ambiguity resolutions

| Question | Accepted grammar decision | Why this is the minimal manual-compliant choice |
| --- | --- | --- |
| Empty program | reject | No empty program is required or demonstrated. |
| Empty block | accept | The manual requires `{ ... }` blocks but gives no nonempty restriction. The nullable content is an edge-case completion, not a bonus construct. |
| Outer program wrapper | not required or special | The §5.5 program is an unwrapped statement sequence. Because a block is also a statement, `{ ... }` remains a legal one-statement program and creates a child scope. |
| Control-body braces | require | This is the only official complete form and creates the required scopes. |
| Standalone/nested block | accept as `<statement>` | Section 5.2 explicitly lists nested blocks with proper scoping; restricting blocks to control bodies would not fully express that feature. |
| Declaration initializer | accept optional `= <expression>` | The §4.5 type-mismatch example must parse before semantic analysis can classify it. This is manual-derived compatibility, not a bonus extension. |
| Multiple declarations | reject | No comma token/form is specified. |
| Print operand | identifier only | All manual print examples use an identifier. |
| Print parentheses | reject | Not demonstrated or required. |
| Bare expression statement | reject | Expression categories are required, but discarded `expression;` syntax is not. |
| Numeric unary `+`/`-` | reject | Only `!` is mandatory; additional unary operators are bonus territory in §14. |
| Assignment expression | reject | Only statement assignment is specified. |
| Condition grammar | accept `<expression>` | The sample uses relational/equality expressions; Boolean validity is semantic. |
| Declaration placement | allow wherever statements occur | The manual states no declarations-first restriction; adding one would invent a rule. |
| Comparison/equality chains | reject without parentheses | Chaining is not required; non-associativity avoids surprising mixed-type chains. |

## 6. Why the grammar is unambiguous

- Statement starters are disjoint: type keywords begin declarations; `IDENTIFIER` begins assignment; `KW_IF`, `KW_WHILE`, and `KW_PRINT` begin their named statements; and `LBRACE` uniquely begins a block statement.
- Recursion from block contents back to `<statement>` consumes a brace before recurring, so it creates no nullable recursion cycle.
- After `<type> IDENTIFIER`, `ASSIGN` uniquely selects an initialized declaration; `SEMICOLON` completes the uninitialized alternative in the surrounding statement production.
- Inside a block, `RBRACE` selects the empty `<optional-statement-list>` alternative, while every valid statement starter selects the nonempty alternative.
- Assignment is not an expression, so `=` cannot compete with expression productions.
- Each expression operator family has its own precedence tier.
- Left recursion defines left associativity for `\|\|`, `&&`, additive, and multiplicative operators.
- Equality and relational productions contain at most one operator at their tier, defining non-associativity.
- Recursive `NOT <unary-expression>` makes `!` right-associative.
- Every control body is a braced block, and `KW_ELSE` cannot begin a normal statement. The factored `<else-part>` therefore has no dangling-`else` ambiguity.
- Parentheses enter through one primary production and explicitly restart the full expression grammar.

The core grammar is structurally capable of zero Bison shift/reduce and zero reduce/reduce conflicts: it has no competing statement starters, dangling `else`, ambiguous declaration suffix, or nullable recursion cycle. An independent read-only M1 construction for this exact BNF produced 298 canonical LR(1) states and 85 merged LALR cores with zero shift/reduce or reduce/reduce conflicts. This is strong static evidence, but an actual Bison warning/counterexample report remains mandatory once the approved toolchain exists. Basic recovery rules will synchronize at semicolons or closing braces but will be documented as parser behavior, not as valid-language CFG.

## 7. Manual-sample walkthrough

The Section 5.5 program is accepted as follows:

- top-level `int x;`, `int y;`, and `bool flag;` are declarations;
- `x = 10;`, `y = 0;`, and `flag = true;` are assignments;
- `while (x > 0) { ... }` uses a relational condition and braced block;
- its body uses additive assignment expressions;
- `if (flag == true) { print y; } else { print x; }` uses Boolean equality, two sibling blocks, and identifier print statements;
- Bison's normal EOF follows the completed `<program>`; no custom token is consumed.

The precedence example `c = a + b * 2;` has one parse: multiplication is inside the right operand of addition.

## 8. Revised-form walkthroughs

- `{ }` derives as `<statement> -> <block> -> LBRACE <optional-statement-list> RBRACE`, with the optional list choosing `ε`.
- `{ int x; { bool ready = true; } }` derives through a block statement whose nonempty list contains a declaration and another block statement. Each `<block>` establishes a distinct scope.
- `bool b = 5 + 3.2;` derives through the initialized `<declaration>` alternative. The expression is syntactically valid, so semantic analysis can infer `float` and report the manual's `bool`/`float` mismatch.
- `int x = 1;`, `float f = 1.0;`, and `bool ok = x < 2;` exercise valid exact-type initialized declarations.

## 9. Explicit rejection examples

```text
int x, y;                   // comma declarations unsupported
print 1;                    // print requires an identifier
print(x);                   // print parentheses unsupported
if (true) print x;          // body must be braced
x + 1;                      // bare expression statement unsupported
x = -1;                     // numeric unary minus unsupported
x = a < b < c;              // chained ordering unsupported
```

Each input is rejected syntactically; none should be reinterpreted as an optional feature.
