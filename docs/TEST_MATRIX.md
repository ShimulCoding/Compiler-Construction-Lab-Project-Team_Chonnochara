# Test Matrix

Last implementation/test update: 24 July 2026.

The M7 runner adds deterministic non-control-flow TAC units/integration goldens to all M2-M6 evidence. Control-flow TAC, the final driver, and the complete end-to-end source-to-TAC case remain blocked.

## Inherited template cases

| ID | Category | Input | Expected result currently documented/inferred | Actual result | Status |
| --- | --- | --- | --- | --- | --- |
| TV-01 | Valid declaration | `tests/valid/declaration.md` | Clean declarations/boolean assignment; AST/TAC unspecified | Not run: no compiler | Blocked |
| TV-02 | Valid assignment | `tests/valid/assignment.md` | Clean identifier/literal assignments; AST/TAC unspecified | Not run: no compiler | Blocked |
| TV-03 | Valid arithmetic | `tests/valid/arithmetic.md` | `b * 2` precedes `a + ...`; print; TAC unspecified | Not run: no compiler | Blocked |
| TV-04 | Inherited if-else sketch | `tests/valid/if_else.md` | Contains `print 0;` | Not run: no compiler | Conflicts with finalized identifier-only print; revise/reclassify during fixture conversion |
| TV-05 | Valid while | `tests/valid/while.md` | Clean loop; TAC unspecified | Not run: no compiler | Blocked |
| TV-06 | Full valid sketch | `tests/valid/complete_program.md` | Clean declarations, loop, conditions, branches, print through TAC | Not run: no compiler | Blocked; candidate E2E seed |
| TI-01 | Lexical error | `tests/invalid/lexical_error.md` | `Lexical Error: Invalid token '@'` | Inherited fenced sketch not executed directly; M3's isolated executable replacement passed with exact line/code/exit | Superseded as runnable evidence by LEX-05; retain sketch for provenance |
| TI-02 | Syntax errors/recovery | `tests/invalid/syntax_error.md` | Missing `;` and missing `)` diagnostics | Not run: no parser | Partial sketch; combined/cascade-prone |
| TI-03 | Undeclared variable | `tests/invalid/undeclared_variable.md` | One diagnostic for each of its two independent undeclared `y` occurrences | Not run: no semantics | Partial sketch; reduce to one occurrence or expect two diagnostics during conversion |
| TI-04 | Redeclaration | `tests/invalid/redeclaration.md` | Same-scope redeclaration of `count` | Not run: no semantics | Partial sketch |
| TI-05 | Scope violation | `tests/invalid/scope_violation.md` | `y` reported out of scope after block | Not run: no semantics | Partial sketch; needs declaration history |
| TI-06 | Mislabeled inherited assignment error | `tests/invalid/type_mismatch.md` | Assignment expected `bool`, found `int` | Not run: no semantics | Final taxonomy classifies this as invalid assignment; replace the distinct type-mismatch fixture with the manual's initialized declaration |
| TI-07 | Invalid assignment | `tests/invalid/invalid_assignment.md` | Cannot assign `bool` to `int` | Not run: no semantics | Partial sketch; overlaps TI-06 |
| EX-01 | Valid demonstration | `examples/valid/sample_program.md` | Representative program succeeds | Not run: no compiler | Blocked; no expected output |
| EX-02 | Multi-error stress | `examples/invalid/sample_program.md` | Syntax and semantic failures | Not run: no compiler | Blocked; no deterministic oracle |

## Mandatory cases to create

| Planned ID | Category / input focus | Expected evidence | Actual result | Status |
| --- | --- | --- | --- | --- |
| E2E-01 | Non-trivial all-phase valid program | Initialized/uninitialized declarations, standalone nested scope, stable AST, and TAC with temporaries, labels, jumps, and print; exit 0 | M7 non-control-flow golden passes; labels/jumps/final CLI remain | Partial |
| LEX-01 | All keywords, literals, delimiters, and single-character operators | Correct tokenization and line tracking | `all_tokens.mc` returned every one of the 32 token kinds with expected lines/lexemes | Pass (M3 lexer) |
| LEX-02 | Identifier boundaries and keyword prefixes | Longest-match identifiers vs keywords | `_name`, `value2`, `integer`, `Int`, `ifvalue`, `while2`, and `trueValue` were identifiers while exact lowercase keywords retained keyword tokens | Pass (M3 lexer) |
| LEX-03 | `<`/`<=`, `>`/`>=`, `!`/`!=`, `=`/`==`, `&&`, and `\|\|` adjacency | Related single/multi-character operators are distinguished atomically | Compact `=== !== <<= >>= !!= &&\|\|` matched the reviewed token golden | Pass (M3 lexer) |
| LEX-04 | Supported `//` comments and whitespace | Discarded with correct later line numbers | Spaces/tabs/blank lines, code-before-comment, and full-line comments passed for LF and generated CRLF copies | Pass (M3 lexer) |
| LEX-05 | Invalid character/malformed token | Line-aware lexical diagnostic; nonzero exit | `@` after a comment/blank line produced exact `LEX_INVALID_TOKEN` at line 3 and exit 1 | Pass (M3 lexer) |
| LEX-06 | Integer/float boundaries and malformed numbers | Contract-defined forms accepted; invalid forms diagnosed without misleading splits | `0`, `42`, `3.14`, `0.5` passed; `.5`, `5.`, and `1e10` each produced one exact lexical diagnostic and exit 1 | Pass (M3 lexer) |
| LEX-07 | Unsupported `/* ... */` text | Must not be accepted as a comment | `/* block */` was not discarded; it emitted `SLASH STAR IDENTIFIER STAR SLASH` for later syntax rejection | Pass (M3 lexer boundary) |
| SYN-01 | Each statement form, both declaration forms, and standalone/nested/empty blocks | Correct AST shapes; every block is a scope and initializer is an optional declaration child | Valid parser fixtures and selected AST goldens passed | Pass (M4 parser) |
| SYN-02 | Arithmetic/relational/logical precedence and associativity | Golden AST proves grouping | `precedence.mc` golden proves `!`, multiplicative/additive, relational, equality, `&&`, `||`, and parentheses; all 14 operators also parse | Pass (M4 parser) |
| SYN-03 | Isolated missing semicolon | One stable line-aware syntax diagnostic | Exact line-2 diagnostic and exit 2 passed | Pass (M4 parser) |
| SYN-04 | Recovery across statement/block boundaries | Later safe error is also reported | Two-error semicolon case and closing-brace synchronization case passed | Pass (M4 parser) |
| SYN-05B | `print` literal, expression, and parenthesized operand | Three isolated fixtures reject; identifier-only form accepts | All three unsupported forms produced exact syntax diagnostics; identifier cases passed | Pass (M4 parser) |
| SYN-05C | Unbraced control body | Isolated `if` and `while` fixtures rejected | Both produced exact `expecting {` diagnostics | Pass (M4 parser) |
| SYN-05F | Bare expression statement | Isolated fixture rejected | `value + 1;` produced exact syntax diagnostic and exit 2 | Pass (M4 parser) |
| SYN-05G | Numeric unary sign | Isolated fixture rejected | `value = -1;` produced exact syntax diagnostic and exit 2 | Pass (M4 parser) |
| SYN-06 | Official initializer and block derivations | `bool b = 5 + 3.2;`, `{ }`, and a multiply nested standalone block all parse before semantics | All forms produced reviewed AST structures; official sample also parsed | Pass (M4 parser) |
| SEM-01 | Use before declaration/never declared | Undeclared-variable diagnostic | `undeclared.mc` and `self_initializer.mc` matched exact `SEM_UNDECLARED` goldens | Pass (M6) |
| SEM-02A | Same-scope duplicate | `SEM_REDECLARATION`; first valid binding remains active | `redeclaration.mc` emitted one redeclaration and its later exact-int assignment succeeded | Pass (M6) |
| SEM-02B | Nested shadow | Separate valid fixture succeeds and restores the outer binding on scope exit | `shadow_restore.mc` and `initializer_outer.mc` exited 0 silently | Pass (M6) |
| SEM-03 | Use after standalone/control/sibling block scope | Each brace pair establishes one scope; post-exit use receives the distinct out-of-scope diagnostic | Standalone and sibling fixtures matched exact `SEM_SCOPE_VIOLATION` goldens | Pass (M6) |
| SEM-04 | Non-Boolean `if` and `while` conditions | Two isolated `SEM_TYPE_MISMATCH` fixtures state expected `bool` and actual type | Integer `if` and float `while` fixtures matched exact line-aware goldens | Pass (M6) |
| SEM-05 | Invalid assignment | Isolated LHS/RHS compatibility diagnostic | Bool-to-int assignment matched exact `SEM_INVALID_ASSIGNMENT` at line 4 | Pass (M6) |
| SEM-06 | Invalid operator operands | Boolean arithmetic, floating remainder, numeric logical/not, and Boolean ordering reject | Five isolated fixtures matched exact `SEM_INVALID_OPERATOR` goldens | Pass (M6) |
| SEM-07 | Boolean/numeric equality | `SEM_TYPE_MISMATCH` names both incompatible domains without claiming one expected operand type | `equality_mismatch.mc` matched the exact bool/int diagnostic | Pass (M6) |
| SEM-08 | Valid mixed numeric operations and exact-type stores | Operation-local promotion plus matching assignment/initializer result types succeed; exit 0 | Mixed arithmetic, ordering, equality, and Boolean composition exited 0 silently | Pass (M6) |
| SEM-09 | Implicit assignment conversion | Isolated `float = int` fixture produces `SEM_INVALID_ASSIGNMENT`; exit 3 | `invalid_numeric_assignment.mc` matched the exact int-to-float rejection | Pass (M6) |
| SEM-10A | Valid initialized declarations | `int`, `float`, and `bool` initializers—including a mixed numeric expression yielding `float`—succeed | `core.mc` and `numeric_promotion.mc` cover all three declaration types and exited 0 | Pass (M6) |
| SEM-10B | Manual initialized-declaration mismatch | `bool b = 5 + 3.2;` produces one `SEM_TYPE_MISMATCH`, not syntax or invalid-assignment errors | Manual-derived expression matched one exact type-mismatch golden and later `print b` added no cascade | Pass (M6) |
| SEM-10C | Exact initializer compatibility | `float f = 1;` produces `SEM_TYPE_MISMATCH`; no implicit widening is invented | `initializer_exact_mismatch.mc` matched the exact int-to-float initializer rejection and its later print resolved | Pass (M6) |
| SEM-10D | Initializer cascade suppression and later visibility | Invalid operator/name reports its root error without a dependent mismatch; the fresh binding is still inserted so a later use does not add `SEM_UNDECLARED` | `initializer_cascade.mc` emitted only invalid operator, while `self_initializer.mc` emitted only undeclared; both later prints resolved | Pass (M6) |
| SEM-10E | Initializer visibility and redeclaration | New name is invisible in its initializer, outer shadow can resolve, duplicate preserves first binding, and rejected initializer is still traversed | Outer/self/redeclaration fixtures all matched expected silent or multi-error behavior | Pass (M6) |
| TAC-01 | Integer and float `+ - * /` plus contract-valid `%` | Correct temporaries, values, and evaluation order | `all_operators` and precedence goldens match deterministic left-to-right temporaries | Pass (M7) |
| TAC-02 | All relational/logical operators | Correct boolean TAC and chosen logical strategy | `< > <= >= == != && || !` all match exact materialized-value TAC | Pass (M7) |
| TAC-03 | `if`, `if-else`, nested branches | Deterministic conditional/unconditional jumps and labels | Not created | Missing |
| TAC-04 | `while` and nested control flow | Back edge and exit labels | Not created | Missing |
| TAC-05 | Assignment and print | Stable final instructions | Global and scope-qualified assignment/print lines match six valid goldens | Pass (M7) |
| TAC-06 | Initialized and uninitialized declarations | Compound initializer emits expression TAC then one store; literal initializer emits a direct store; plain declaration emits none | `basic`, `sequential`, and shadow initializer goldens match all three behaviors | Pass (M7) |
| CLI-01 | Source path conventions | `.mc`, `.txt`, and another readable supplied path are accepted without extension-based rejection | Not created | Missing |

## M1 static/document validation

These checks validate the contract documents only; they are not compiler tests.

| ID | Check | Expected | Actual | Status |
| --- | --- | --- | --- | --- |
| M1-V01 | Manual traceability | Every token/production maps to the manual or is labeled an implementation boundary | 23 nonterminals are defined and traced; all 22 non-start nonterminals are referenced on a RHS, while `<program>` is the start symbol | Pass (automated/documentary) |
| M1-V02 | Token/operator inventory | 32 source tokens; all 14 listed arithmetic/relational/logical operators plus assignment/delimiters | Automated check found 32/32 catalog tokens and all 32 used by the BNF, with no unknown/unused terminal or custom `END` | Pass (automated static) |
| M1-V03 | Official sample walkthrough | Section 5.5 program fits the CFG | Manual walkthrough completed | Pass (reasoned) |
| M1-V04 | Grammar ambiguity review | Disjoint statement/declaration suffix starters, safe optional block content, layered expressions, no dangling else | Independent construction found zero conflicts; M4 Bison generation subsequently confirmed zero shift/reduce and reduce/reduce conflicts | Pass (static and implemented) |
| M1-V05 | Semantic matrix review | Every valid signature and six error categories have one trigger | Independent review confirmed exact initializer compatibility, `SEM_TYPE_MISMATCH` classification, binding order, redeclaration behavior, cascade suppression, and TAC policy | Pass (documentary) |
| M1-V06 | Toolchain source verification | Commands match current official Microsoft/Canonical/Ubuntu guidance | Official sources checked 2026-07-21 | Pass (research only) |
| M1-V07 | System mutation check | No WSL/package installation; only the explicitly approved local M1 documentation commit | No installation command executed and no push performed | Pass |
| M1-V08 | Markdown integrity | Balanced fences/tables, no trailing whitespace, UTF-8-readable content, final LF | 15 proposed Markdown files checked; zero structural issues | Pass (automated static) |
| M1-V09 | Revised-form reachability | Standalone/empty nested blocks and both declaration forms derive; Section 5.5 still derives; normal EOF completes parsing | In-memory recognizer matched 12/12 expected cases: required forms accepted and unrelated forms rejected | Pass (static recognizer) |

## M2 build and AST validation

Environment: Ubuntu 24.04.4 LTS on WSL2; GCC 13.3.0, GNU Make 4.3, and Bison 3.8.2. Canonical files remained in the Windows checkout mounted through `/mnt/e`.

| ID | Check | Expected | Actual result | Status |
| --- | --- | --- | --- | --- |
| M2-V01 | Clean C11 build | AST/test sources compile with `-std=c11 -Wall -Wextra -Wpedantic`; Bison generates its header without conflicts/warnings | `make clean` then `make` exited 0 with no compiler or Bison warnings | Pass |
| M2-V02 | Shared token interface | One Bison source declares 32 unique source tokens, no custom `END`, and its generated header compiles from C | Static count found 32/32 unique tokens and `build/token_interface_test` exited 0 | Pass |
| M2-U01 | Empty block | Block list starts at zero statements and destroys safely | `test_empty_block` passed | Pass |
| M2-U02 | Statement lists | Multiple statements retain insertion/source order | `test_multiple_statements` passed | Pass |
| M2-U03 | Nested block | A block can be stored as a statement inside another block | `test_nested_block` passed | Pass |
| M2-U04 | Declaration forms | Plain declaration has no initializer; initialized declaration owns one expression child | Both declaration tests passed | Pass |
| M2-U05 | Assignment and expressions | Assignment owns its RHS; multiplication nests beneath addition; logical `!` owns one operand | Assignment, arithmetic-binary, and logical-unary tests passed | Pass |
| M2-U06 | Control statements | One `If` supports absent/present else blocks; `While` owns condition/body; print owns its identifier text | `if` (both forms), `while`, and `print` tests passed | Pass |
| M2-U07 | Constructor validation | Invalid type/required pointers/container-child combinations fail without taking caller ownership | Validation test passed | Pass |
| M2-U08 | Printer determinism | Two prints of the official-sample-shaped tree are identical and match a tracked golden | In-memory comparison and `tests/expected/ast_unit.stdout` byte comparison passed | Pass |
| M2-U09 | Recursive cleanup execution | A populated nested tree can be recursively destroyed without a crash | Cleanup execution test passed; no Valgrind/leak claim | Pass |
| M2-V03 | Automated target | `make test` builds first, validates token header, runs AST tests, and returns 0 | 15/15 AST tests passed; runner printed two PASS summaries and exited 0 | Pass |
| M2-V04 | Artifact hygiene/clean | Generated products stay ignored and `make clean` removes only `build/` | `/build/` is ignored; clean target exited 0 and was followed by a successful rebuild/test | Pass |

## M3 lexer validation

Environment: the same Ubuntu 24.04.4 LTS WSL2 toolchain, including Flex 2.6.4, Bison 3.8.2, and GCC 13.3.0.

| ID | Check | Expected | Actual result | Status |
| --- | --- | --- | --- | --- |
| M3-V01 | Clean generation/build | Bison header precedes Flex generation; generated scanner compiles with C11 warning flags and no `libfl` | `make clean` then `make` generated `parser.tab.h`/`lex.yy.c` under `build/` and linked `lexer_test` with no actionable warning or `-lfl` | Pass |
| M3-V02 | Token-authority accounting | Parser declarations, lexer returns, test display map, and all-token golden contain the same 32 source kinds; no custom `END` | Automated set comparison reported 32/32 in all four locations and no `END` | Pass |
| M3-L01 | Complete token/boundary stream | All token families plus keyword-prefixed identifiers match stable output | `all_tokens` and `operator_adjacency` passed byte-for-byte | Pass |
| M3-L02 | Layout/comments/locations | Ignored text emits no token and later lines remain correct under LF and CRLF | LF fixture and generated CRLF copy produced identical reviewed output; code-before-comment and full-line comment both passed | Pass |
| M3-L03 | Official Section 5.5 sample | Complete sample tokenizes through normal EOF with correct lines | `official_sample` matched its token golden and exited 0 | Pass |
| M3-L04 | Invalid input | Generic invalid character and three unsupported numeric spellings produce deterministic first-error output | Four isolated fixtures matched exact stderr/exit-1 goldens | Pass |
| M3-L05 | Unsupported block comment | Scanner does not silently add block-comment support | Compact block-comment spelling emitted operator/content tokens rather than disappearing | Pass |
| M3-V03 | Automated/regression target | New lexer tests and every M2 test pass together | `make test` reported generated-header PASS, AST/golden PASS, and 10 lexer cases PASS; AST binary remains 15/15 | Pass |

## M4 parser and AST-integration validation

Environment: Ubuntu 24.04.4 LTS on WSL2 with Bison 3.8.2, Flex 2.6.4, and GCC 13.3.0. `tests/support/parser_driver.c` is test-only; semantic/TAC validity is not claimed.

| ID | Check | Expected | Actual result | Status |
| --- | --- | --- | --- | --- |
| M4-V01 | Grammar generation/build | Complete CFG; zero shift/reduce and reduce/reduce conflicts; warning-clean C11/Flex integration | Bison ran with conflict warnings promoted to errors; Bison/Flex/GCC completed with zero conflicts and no actionable warnings | Pass |
| M4-P01 | Required statement/block forms | Both declarations, assignment, print, `if`, `if-else`, `while`, standalone/empty/nested blocks, and multiple top-level statements build ASTs | All valid fixtures exited 0; selected initialized/nested/control outputs matched exact goldens | Pass |
| M4-P02 | Expression construction | All 14 operators parse; AST reflects structural precedence, associativity, parentheses, and right-recursive `!` | All-operator source passed; precedence golden placed `*` under `+`, relation under `!`, `&&` above `||`, and honored grouping | Pass |
| M4-P03 | Manual compatibility/sample | `bool b = 5 + 3.2;` reaches AST; Section 5.5 sample parses through normal EOF | Manual initializer appeared as a declaration with `+` initializer subtree; official sample matched its reviewed AST golden | Pass |
| M4-P04 | Locations and line endings | Comments/blank lines preserve node/diagnostic lines for LF and CRLF | LF and generated CRLF sources produced the same line-3/line-5 AST; invalid source after comments reported line 4 | Pass |
| M4-P05 | Syntax boundaries | Missing punctuation/braces, unbraced bodies, misplaced `else`, unsupported print operands, empty source, bare expression, numeric unary minus, and comparison chains reject | Fifteen isolated syntax fixtures matched exact `SYN_UNEXPECTED_TOKEN` stderr and exit 2 | Pass |
| M4-P06 | Basic recovery | Parser continues after malformed input at `;` and `}` without producing an authoritative AST | Semicolon fixture reported independent errors on lines 1 and 3; closing-brace fixture synchronized and returned syntax failure with empty stdout | Pass |
| M4-P07 | Lexical/syntax integration | One lexical root error has no duplicate syntax diagnostic; later independent syntax remains visible | Isolated `@` emitted only `LEX_INVALID_TOKEN`; combined case emitted that lexical error plus a later line-3 syntax error and kept exit 1 | Pass |
| M4-P08 | Ownership/error-path policy | Persistent identifier text is copied; discarded text/nodes/lists have Bison destructors; failures return no AST | Parser actions/destructors and empty-stdout assertions exercised normal/recovery cleanup without crashes; no leak-detector claim | Pass |
| M4-V02 | Automated regressions | M2 AST and M3 lexer behavior remain unchanged | `make test` reported header PASS, 15/15 AST/golden PASS, 10 lexer PASS, and 32 parser PASS | Pass |

## M5 symbol-table validation

Environment: Ubuntu 24.04.4 LTS on WSL2 with GCC 13.3.0 and GNU Make 4.3. Tests call the symbol-table API directly; they do not traverse parser-built ASTs or emit semantic diagnostics.

| ID | Check | Expected | Actual result | Status |
| --- | --- | --- | --- | --- |
| M5-V01 | Clean C11 build | `symbol_table.c` and its test compile with `-std=c11 -Wall -Wextra -Wpedantic` and no actionable warning | Focused and integrated builds completed without warnings | Pass |
| M5-U01 | Records/global scope | Creation starts at scope ID 0/depth 0; inserted records retain copied name, type, line, ID, and depth | Creation, metadata, name-copy, and global insertion checks passed | Pass |
| M5-U02 | Scope lifecycle | Child entry increments ID/depth; nested and later sibling IDs are unique; global exit is rejected safely | IDs `0,1,2,3` and depths `0,1,2,1` appeared in the reviewed golden; lifecycle checks passed | Pass |
| M5-U03 | Lookup modes | Current lookup stays local; active lookup is innermost-first; history searches inactive scopes only | Missing/current/active/history, multiple-level, and borrowed-pointer checks passed | Pass |
| M5-U04 | Duplicate/shadow behavior | Same-scope duplicate fails without replacing the first record; child shadow succeeds and outer binding returns after exit | Duplicate, original-preservation, shadow, innermost, and restoration checks passed | Pass |
| M5-U05 | Sibling/history distinction | An exited sibling cannot resolve actively but remains historical; never-declared names fail both searches | Sibling isolation and inactive-versus-never-declared checks passed | Pass |
| M5-U06 | Initializer-order support | Before an inner declaration is inserted, active lookup returns the outer name; afterward it returns the inner, then restores outer on exit | One explicit workflow exercised all three lookup points successfully | Pass |
| M5-U07 | Ownership/cleanup | Names are copied; lookup pointers remain stable; destroying global-only, active-nested, exited, and NULL tables does not crash | Ownership, stable pointer, and cleanup execution checks passed; no leak-detector claim | Pass |
| M5-U08 | Deterministic printer | Scope/symbol ordering and active state match one tracked golden and repeat identically | Two executions matched each other and `tests/expected/symbol_table_unit.stdout`; 30/30 tests passed | Pass |
| M5-V02 | M2-M4 regressions | Header, AST/golden, lexer, parser/goldens, diagnostics, recovery, and official sample remain unchanged | `make test` reported header PASS, 15 AST PASS, 30 symbol-table PASS, 10 lexer PASS, and 32 parser PASS | Pass |

## M6 semantic-analysis validation

Environment: Ubuntu 24.04.4 LTS on WSL2 with GCC 13.3.0, GNU Make 4.3, Flex 2.6.4, and Bison 3.8.2. `tests/support/semantic_driver.c` is test-only; TAC and final compiler behavior are not claimed.

| ID | Check | Expected | Actual result | Status |
| --- | --- | --- | --- | --- |
| M6-V01 | Clean integration build | Parser, lexer, AST, symbol table, semantic analyzer, and test driver compile with existing C11 warning flags; Bison remains conflict-free | Focused and integrated builds completed with zero Bison conflicts and no actionable GCC/Flex/Bison warnings | Pass |
| M6-S01 | Source-order declarations/uses | No hoisting; uninitialized and initialized declarations become visible after their statements | Valid core passed; self-initializer produced one undeclared diagnostic while its later print resolved | Pass |
| M6-S02 | Block scope behavior | Exactly one scope per block; nested shadowing, restoration, and sibling isolation work | Four valid scope fixtures passed; two inactive-use fixtures produced exact scope-violation diagnostics | Pass |
| M6-S03 | Expression typing/operators | Numeric promotion, integer `%`, numeric ordering, compatible equality, and Boolean logical/not rules | Valid mixed operations passed; arithmetic/remainder/logical/not/relational/equality invalid cases matched exact codes/messages | Pass |
| M6-S04 | Storage/context rules | Initializers and assignments use exact compatibility; `if`/`while` require bool; print identifiers resolve | Valid core passed; initializer, assignment, and two condition cases matched exact line-aware goldens | Pass |
| M6-S05 | Diagnostic classification/cascades | All approved codes are deterministic; dependent follow-ups are suppressed while independent errors continue | 20 invalid cases matched stderr and exit-3 goldens; multi-error case preserved source order; rejected initializer reported two independent errors | Pass |
| M6-V02 | Full regressions | M2-M5 behavior remains unchanged | `make test` reported header PASS, 15 AST PASS, 30 symbol-table PASS, 10 lexer PASS, 32 parser PASS, and 26 semantic PASS | Pass |

## M7 expression and statement TAC validation

Environment: Ubuntu 24.04.4 LTS on WSL2 with GCC 13.3.0, GNU Make 4.3, Flex 2.6.4, and Bison 3.8.2. `tests/support/tac_driver.c` is test-only; M8 control-flow TAC and the final compiler driver are not claimed.

| ID | Check | Expected | Actual result | Status |
| --- | --- | --- | --- | --- |
| M7-V01 | Clean integration build | TAC core/unit/driver compile under existing C11 warnings; Bison remains conflict-free | Integrated build completed with zero Bison conflicts and no actionable GCC/Flex/Bison warnings | Pass |
| M7-U01 | Program model/ownership | Empty program prints nothing; statuses/invalid arguments are explicit; destroy(NULL) and copied instruction text are safe | 14/14 direct unit checks passed and exact output repeated identically | Pass |
| M7-T01 | Direct operands/statements | Plain declaration emits none; initialized declaration/assignment/print emit canonical lines; numeric/Boolean literals are stable | `basic` and `sequential` matched exact TAC goldens | Pass |
| M7-T02 | Every expression operator | `!` and all 13 binary AST operators emit deterministic temporaries with no optimization or short-circuit jumps | `all_operators` matched a `t1` through `t15` exact golden | Pass |
| M7-T03 | Precedence/evaluation order | AST grouping lowers left-to-right and nested results feed parents | Parenthesized/nested and non-trivial precedence goldens matched exactly | Pass |
| M7-T04 | Block binding identity | Globals keep names; nested shadows use monotonic `name@scope-id`; outer/sibling bindings restore correctly | `blocks_shadowing` and `sequential` matched scope IDs 1, 2, and 3 as expected | Pass |
| M7-T05 | Initializer-before-binding | An inner declaration initializer resolves the active outer binding before creating the inner storage name | Golden emitted `t1 = x + 1` before `x@1 = t1` | Pass |
| M7-T06 | Phase/failure gates | Semantic error produces existing diagnostic/no TAC; control flow is explicit unsupported/no partial output | Semantic fixture exited 3; isolated `if` and `while` matched `TAC_UNSUPPORTED_NODE` stderr and exit 4 | Pass |
| M7-T07 | Repeat determinism | New generation resets temporary/scope counters and repeated output is byte-identical | Direct reset checks and a repeated shadowing integration run passed | Pass |
| M7-T08 | Temporary/global collision safety | Any direct global `tN` declaration, including a later declaration, reserves that storage name before emission | Before/after/initialized `t1` fixtures all used `t2`; repeated generation reset to the same output | Pass |
| M7-V02 | Full regressions | M2-M6 behavior remains unchanged | `make test` reported header PASS, 15 AST, 30 symbol-table, 10 lexer, 32 parser, 26 semantic, 14 TAC unit, and 12 TAC integration PASS | Pass |

## Audit commands and results

| Date | Command/check | Result |
| --- | --- | --- |
| 2026-07-21 | Native `make` | Failed: command not found |
| 2026-07-21 | `C:\MinGW\bin\mingw32-make.exe` | Failed: no Makefile/targets |
| 2026-07-21 | Source/build/test inventory | No source, Makefile, executable, generated code, or runner |
| 2026-07-21 | Tool discovery | Git available; no usable Flex/Bison toolchain found on `PATH` or in checked common locations |

## Required test-record format

For each future run record:

```text
Date / environment:
Command:
Input:
Expected output and exit code:
Actual output and exit code:
Status:
Related commit:
```
