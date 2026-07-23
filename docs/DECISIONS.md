# Design Decisions

Status: M1-M6 decisions are approved, committed, and pushed. M7 decisions below are implemented and validated, pending Dipro's review and approval.

The official Project Manual remains authoritative. Decisions below fill gaps only where an implementation cannot be consistent without choosing a boundary.

## Accepted project constraints

| ID | Date | Decision | Reason |
| --- | --- | --- | --- |
| D-001 | 2026-07-21 | The official PDF manual is the highest project authority, with Section 5 controlling ambiguity elsewhere. | Explicit team instruction and manual §5. |
| D-002 | 2026-07-21 | TAC ends the mandatory target-program pipeline; no hardware backend or unrelated bonus work precedes core completion. | Manual §§4.6, 6, 14. |
| D-005 | 2026-07-21 | Generate TAC only after successful lexical, syntax, and semantic phases. | Required phase order and valid/invalid behavior. |
| D-006 | 2026-07-21 | Keep generated Flex/Bison sources and build products untracked where practical. | Manual §8 and inherited `.gitignore`. |
| D-007 | 2026-07-21 | Credit only genuine reviewed ownership; inherited and unreviewed AI-assisted work is not a member contribution. | Manual Git/viva policy and team instruction. |

## Finalized M1 language decisions

| ID | Decision | Classification | Minimal justification |
| --- | --- | --- | --- |
| D-003 | Accept a nonempty top-level statement sequence without requiring a special outer wrapper. A block may still be the sole top-level statement through the general block production. | Syntax completion | The authoritative §5.5 sample is unwrapped, while §5.2 independently requires block statements. |
| D-004 | Allow inner shadowing; reject duplicate declarations only in the same scope. | Scope completion | The manual names same-scope redeclaration and nested visibility. |
| D-106 | Support `//` line comments only. | Lexer choice allowed by manual | §4.1 says single-line and/or block comments; one form is the minimum. |
| D-107 | Require `bool` conditions and Boolean operands for logical operators. | Semantic completion | Avoids unsupported numeric truthiness and satisfies invalid-expression testing. |
| D-111 | Allow operation-local mixed `int`/`float` promotion; restrict `%` to `int,int`. | Semantic completion | Preserves the manual's `5 + 3.2` example without inventing floating remainder. |
| D-112 | Require exact-type assignment; provide no implicit assignment conversion or casts. | Semantic completion | The manual grants no widening/narrowing rule. |
| D-113 | Require braced control bodies; also accept blocks as statements and permit empty block contents. Continue rejecting bare expression statements, numeric unary signs, and unparenthesized comparison chains. | Manual-derived syntax plus edge-case completion | §5.2 explicitly requires nested blocks. `{ }` adds no construct beyond a block and avoids an undocumented nonempty restriction. |
| D-114 | Restrict print syntax to `print IDENTIFIER;`. | Minimal syntax boundary | Every manual print example uses exactly that form. |
| D-115 | Use a three-way semantic distinction: declaration-initializer/contextual/incompatible-domain type mismatch, standalone-assignment incompatibility, and invalid operator signature. | Error taxonomy | Preserves the manual's initializer example as `Type mismatch` while making all six semantic categories independently testable. |
| D-116 | Report independent errors but propagate an internal error type to suppress dependent cascades. | Diagnostic behavior | Clearer diagnostics without hiding separate occurrences. |
| D-117 | Materialize Boolean values in TAC; no short-circuit language guarantee. | TAC completion | Manual requires logical TAC but no evaluation strategy; core expressions have no side-effecting calls/assignments. |
| D-119 | Accept both `type name;` and `type name = expression;`; apply exact type compatibility and lower a valid initializer as expression TAC followed by a store. | Manual-derived compatibility | Manual §4.5 explicitly uses `bool b = 5 + 3.2;` as a semantic type-mismatch input, so the parser must accept the form. |
| D-120 | Analyze an initializer before its new binding is visible, then insert a fresh declaration even if initializer analysis failed; never replace the first binding on redeclaration. | Scope/diagnostic completion | Matches declaration-point visibility, permits outer-binding lookup during shadow initialization, and prevents later undeclared cascades. |
| D-121 | Use Bison's normal EOF acceptance and declare no custom `END` source token. | Parser boundary | EOF is parser control; the authoritative catalog remains exactly 32 source tokens. |

Full rules and matrices are authoritative in `docs/LANGUAGE_SPEC.md`; formal syntax is authoritative in `docs/GRAMMAR.md`.

## Finalized M1 build and test decisions

| ID | Decision | Reason |
| --- | --- | --- |
| D-101 | Use readable C11 with Flex, Bison, GCC, and GNU Make. | Manual permits C/C++; the team instruction prefers C. |
| D-105 | Use `.mc` as the primary test/example extension, accept `.txt`, and do not reject another readable supplied path solely by extension; retain tracked expected stdout/stderr/exit files, ephemeral routine actual output, and curated actual evidence. | The official pipeline shows `.txt / .mc`; extensions organize evidence rather than define grammar. |
| D-109 | Pin Ubuntu 24.04 LTS on WSL 2 as the primary environment; do not claim native Windows support until separately validated. | Stable supported baseline compatible with the instructor guidance; exact plan in `docs/TOOLCHAIN.md`. |
| D-110 | Use Flex `%option noyywrap` unless a later reviewed need justifies `libfl`. | Avoids an unnecessary platform link dependency. |
| D-118 | Plan one-file CLI output with deterministic `AST:`/`TAC:` sections, diagnostics on stderr, and exit codes 0-4 by phase. | Enables reproducible tests and a simple live demo. |

Exact conventions are in `docs/TEST_CONVENTIONS.md`.

## M2 AST and build decisions

| ID | Decision | Reason and consequence |
| --- | --- | --- |
| D-201 | Represent the AST as one public `AstNodeKind`-tagged `AstNode` union, with explicit child pointers and one reusable `AstNodeList` dynamic array for programs/blocks. | Directly mirrors the approved grammar, stays easy to traverse in semantics/TAC, and avoids a generic container framework. |
| D-202 | Store a line-only `SourceLocation` on every AST node. | The manual requires line-aware diagnostics; columns/ranges can be added only if a later phase demonstrates a need. |
| D-203 | Copy every constructor-supplied identifier name. A successful constructor owns its child nodes; failure leaves child ownership with the caller. Successful list append transfers the statement to the list. | Gives one consistent lifetime rule for parser actions and prevents borrowed lexer buffers from outliving a token. |
| D-204 | Use one `AST_NODE_IF` with a required then-block and optional else-block; omit punctuation/parenthesis nodes. | Both required forms share structure, while the expression tree already records parenthesized grouping. |
| D-205 | Return `NULL`/`false` for invalid constructor arguments or allocation/list-growth failure; keep these internal failures separate from future `LEX_`, `SYN_`, and `SEM_` diagnostics. | Simple, testable C error handling without terminating inside a reusable data-structure module. |
| D-206 | Print a deterministic two-space-indented tree with source lines and labeled structural edges; represent an empty statement list as `<empty>`. | Produces readable presentation output and a stable byte-comparable test oracle without adding Graphviz. |
| D-207 | Keep all 32 token declarations in `src/parser/parser.y` and generate the shared Bison header before Flex compilation. M2's one-token placeholder was only a foundation; M4 replaced it with the complete grammar. | Establishes one token-number authority and avoids duplicating token enums in Flex/C while preserving the generated-header dependency through later parser work. |
| D-208 | Build only existing M2 artifacts and place every generated object, executable, Bison output, and routine test result under ignored `build/`. | Makes `make`, `make test`, and `make clean` truthful while avoiding fake rules for missing compiler phases. |

## M3 lexer decisions

| ID | Decision | Reason and consequence |
| --- | --- | --- |
| D-301 | Use a simple non-reentrant Flex scanner with `%option noyywrap yylineno noinput nounput never-interactive nodefault`. | Matches the one-file compiler workflow, avoids `libfl` and unused generated helpers, and gives understandable line tracking without scanner-state machinery. |
| D-302 | Keep `src/parser/parser.y` as the sole 32-token authority. The lexer includes its generated header, returns those constants, returns normal `YYEOF`, and uses only built-in `YYUNDEF` as the invalid-input marker. | Prevents token-number drift and adds no source token or custom `END`. |
| D-303 | Expose the current line as `SourceLocation` and expose `yytext` only through a borrowed accessor valid until the next `yylex()` call. Defer copied/converted Bison semantic values and `%locations` assignments to M4. | Tests can validate lexemes and lines now without inventing a premature semantic union; future parser actions receive a clear ownership boundary. |
| D-304 | Report one stable `LEX_INVALID_TOKEN` diagnostic per invalid scanner match. The M3 test driver stops after the first such result and exits 1. | Produces a simple deterministic oracle while preserving `YYUNDEF` for later parser recovery/phase-gate integration. |
| D-305 | Support only `//` comments. A `/* ... */` spelling is not discarded and is emitted as its individually valid slash/star/identifier tokens for later syntax rejection. | Does not add the unsupported block-comment feature or mislabel valid operator characters as an unmatchable token. |
| D-306 | Keep the 32-name display switch in `tests/support/lexer_driver.c` only; it maps generated Bison constants for golden output and is not a production token definition. | Provides deterministic phase tests while the complete parser/driver do not exist; static validation checks it against the parser catalog. |
| D-307 | During M4, distinguish an already-reported lexical failure from an independent syntax error. If `YYUNDEF` represents the same token for which the lexer emitted `LEX_INVALID_TOKEN`, parser/compiler integration must not emit a duplicate generic syntax diagnostic for that root cause. | Preserves one clear primary diagnostic while still allowing genuinely independent syntax errors to be reported. This is an M4 integration contract, not M3 parser implementation. |

## M4 parser and AST-integration decisions

| ID | Decision | Reason and consequence |
| --- | --- | --- |
| D-401 | Use one Bison `%union` containing only `AstNode *`, temporary `AstNodeList *`, copied identifier text, numeric values, `ValueType`, and binary-operator tags. | These are the exact values required to construct syntax trees; symbol-table and semantic-analysis state remain outside the parser. |
| D-402 | Encode precedence and associativity through the approved layered nonterminals, with equality and relational productions admitting at most one operator at their tier. | The grammar stays readable and unambiguous, rejects unparenthesized comparison chains, and generated with zero conflicts without precedence directives hiding ambiguity. |
| D-403 | Enable Bison locations and have Flex set each token's first/last line from `yylineno`; convert the first line to the existing line-only `SourceLocation` at each AST action. | Preserves diagnostic lines across comments, blank lines, LF, and CRLF without redesigning the M2 location abstraction. |
| D-404 | Copy identifier lexemes in Flex, let AST constructors copy required names, then free the token copy in successful actions. Use `%destructor` for identifier strings, AST nodes, and parser-owned temporary statement lists discarded during recovery. | `yytext` is never retained, successful ownership transfer is explicit, and error paths have matching cleanup without changing the AST API. |
| D-405 | Report Bison detailed expectations through the stable `SYN_UNEXPECTED_TOKEN` wrapper and recover with `error SEMICOLON` plus `LBRACE error RBRACE`. A recovered statement contributes no AST node, and any syntax-error count invalidates/destroys the partial AST. | Gives understandable basic recovery at two safe boundaries, avoids infinite loops, and prevents malformed trees from reaching later phases. |
| D-406 | Track scanner errors already reported for `YYUNDEF`. Suppress exactly the corresponding parser callback; do not suppress a later independent syntax error after recovery. | Implements D-307 and keeps one root lexical diagnostic while retaining useful independent syntax evidence. |
| D-407 | Keep `tests/support/parser_driver.c` as a phase-test tool that prints the parser-built AST and returns 1 for lexical failure, 2 for syntax failure, and 4 for usage/I/O/internal failure. | Enables M4 automation without pretending the final semantic/TAC compiler driver from M10 exists. |
| D-408 | Convert integer/floating token text with C's `strtoll`/`strtod` into the existing AST `long long`/`double` fields; report an out-of-range conversion through the existing lexical failure path rather than store a saturated/infinite value. | M4 must create persistent numeric semantic values, and silently changing a literal's value would corrupt the AST. This adds no token, spelling, or language feature. |

## M5 symbol-table decisions

| ID | Decision | Reason and consequence |
| --- | --- | --- |
| D-501 | Keep every scope frame for the table's lifetime in creation order. Each frame has an active flag, a parent pointer, and a declaration-ordered linked list of individually allocated symbols. | Exiting a scope removes it from active lookup without losing the history needed to distinguish out-of-scope from never declared; individual symbol addresses remain stable for borrowed lookup results. |
| D-502 | Create global scope ID 0/depth 0 automatically; assign later IDs monotonically and never reuse them. Reject ordinary exit of the global scope. | IDs identify creation history while depth describes nesting; later siblings receive distinct IDs at the same depth, and the table always retains a valid active root. |
| D-503 | Expose separate current-scope, innermost-to-global active, and inactive-history lookups. Historical lookup searches exited scopes newest first and never resolves a binding as active. | Each future semantic question has one explicit operation: redeclaration detection, normal resolution, or scope-violation evidence. |
| D-504 | Copy each successfully inserted name, reject same-scope duplicates before allocation, and permit child-scope shadowing. | The first binding remains unchanged after a duplicate attempt, caller buffers may change safely, and exiting a shadow restores the outer binding naturally through parent lookup. |
| D-505 | Let semantic analysis control declaration order: check current scope, analyze an initializer against existing active bindings, then insert the fresh declaration. | The symbol table stays independent of AST traversal while directly supporting the approved initializer-before-binding rule. |
| D-506 | Return internal scope/insertion statuses rather than emitting `SEM_...` diagnostics. | M5 is a reusable data structure; M6 owns source-facing classification and messages. |
| D-507 | Print scopes in creation order and declarations in insertion order, including scope ID/depth/active state plus symbol name/type/line. Empty scopes print `<empty>`. | The output is deterministic, readable, address-free, and suitable for golden tests and viva explanation without becoming compiler output. |
| D-508 | Keep `Symbol` and `SymbolTable` opaque. Lookups return borrowed `const Symbol *`, and `symbol_get_info` returns a read-only view whose name remains table-owned. | Callers cannot mutate table records accidentally; one destroy operation frees active/inactive frames, symbols, and names exactly once. |

## M6 semantic-analysis decisions

| ID | Decision | Reason and consequence |
| --- | --- | --- |
| D-601 | Expose one `semantic_analyze(const AstNode *, FILE *, SemanticResult *)` operation. The caller retains the AST; each analysis owns and destroys a private symbol table and returns success, semantic-error, or internal-error status. | Keeps semantic state isolated per source file, preserves existing AST ownership, and provides a small interface for the later compiler driver. |
| D-602 | Represent expression analysis internally as `{valid, ValueType}` rather than adding an error/unknown member to the source-language `ValueType`. | The fixed language still has exactly three types; an invalid child suppresses only dependent diagnostics while independent traversal continues. |
| D-603 | Map invalid operator signatures to `SEM_INVALID_OPERATOR`, incompatible Boolean/numeric equality and non-Boolean conditions to `SEM_TYPE_MISMATCH`, and exact standalone storage failure to `SEM_INVALID_ASSIGNMENT`. | Implements the already approved non-overlapping taxonomy without inventing the prompt's illustrative `SEM_INVALID_EXPRESSION` or `SEM_INVALID_CONDITION` names. |
| D-604 | Resolve identifiers through active lookup first and inactive history second. A historical record classifies the use but never becomes a usable binding. | Produces the manual-required distinction between `SEM_SCOPE_VIOLATION` and `SEM_UNDECLARED` while preserving lexical visibility. |
| D-605 | Traverse both operands/branches and later statements after recoverable semantic errors, but do not issue a dependent type/assignment/context diagnostic when a required expression result is invalid. | Reports multiple independent source errors in deterministic order without misleading cascades. |
| D-606 | Keep `tests/support/semantic_driver.c` test-only: it parses one supplied file, runs semantics only after parse success, emits no stdout, and returns status 3 when semantic diagnostics exist. | Enables exact M6 integration tests without pretending TAC or the final source-to-TAC CLI is implemented. |
| D-607 | Consolidate the roadmap's former M6 declaration/use pass and M7 type-checking pass into this user-approved M6 milestone. Reassign M7 to the first TAC milestone without changing the contributor cycle. | The requested M6 explicitly requires the complete semantic contract; maintaining a second empty/duplicate semantic milestone would be misleading. |

## M7 TAC decisions

| ID | Decision | Reason and consequence |
| --- | --- | --- |
| D-701 | Represent TAC as an owned dynamic array of tagged assignment, unary, binary, and print instructions. Each instruction owns copied text fields. | The representation is direct enough for viva explanation, preserves an at-most-three-address shape, and provides one safe cleanup path without exposing AST or lexer storage. |
| D-702 | Lower expressions recursively from left to right. Literals and identifiers are direct operands; every unary/binary node emits the lowest available `tN` result beginning at `t1` per generation. | AST structure already preserves precedence; deterministic traversal and counter reset make exact goldens repeatable without optimization. |
| D-703 | Emit `target = operand` for initialized declarations and assignments, `print storage` for print, and no instruction for plain declarations or empty blocks. Format booleans as `true`/`false` and readable floats with `%.15g`, retaining `.0` for integral float values. | Matches the manual's illustrative TAC and keeps literal spelling deterministic and understandable. |
| D-704 | Reuse a private `SymbolTable` during generation and associate each returned `Symbol *` with one owned storage name. Global bindings keep source names; non-global bindings use `name@scope-id`. | Distinct storage identities preserve shadowing, sibling isolation, initializer-before-inner-binding behavior, and outer restoration without modifying the AST or duplicating scope rules. |
| D-705 | Treat `&&`, `||`, and `!` as ordinary value-producing TAC operations in M7. Do not add short-circuit jumps or optimization. | This implements the approved materialized-Boolean strategy; control-flow labels and jumps remain one coherent M8 milestone. |
| D-706 | Return `TAC_STATUS_UNSUPPORTED_NODE` for `if` or `while` and destroy all partial output. The test driver invokes TAC only after semantic success. | Control-flow ASTs are never silently omitted, invalid programs emit existing semantic diagnostics with no TAC, and M7 does not pretend the manual's full TAC requirement is complete. |
| D-707 | Before emission, reserve every direct program-level declaration name and add each allocated temporary to the same per-generation reserved-name set. Skip collisions while scanning `t1`, `t2`, ... upward. | A legal global named `t1` cannot be confused with a compiler temporary, even when declared later in source order. Ordinary names keep existing TAC spelling, while nested `name@scope-id` storage remains inherently distinct. |

## Resolved ambiguity register

| Prior ID | Question | Resolution |
| --- | --- | --- |
| P-001 | Declaration initializers | Supported as an optional declaration suffix because the manual's explicit type-mismatch example must reach semantics. |
| P-002 | Print operand | Identifier only. The inherited `print 0;` sketch must be revised/reclassified later. |
| P-003 | Mixed numeric behavior | Promote `int` within mixed numeric operations; exact-type assignments only. |
| P-004 | `%` domain | `int,int -> int` only. |
| P-005 | Numeric unary minus | Unsupported; use binary subtraction such as `0 - 5`. |
| P-006 | Logical TAC | Materialized Boolean values; no short-circuit guarantee. |
| P-007 | Diagnostic count | Report each independent occurrence; suppress dependent cascades. |
| P-008 | CLI/output | One supplied source path; `.mc` primary and `.txt` accepted without extension enforcement; deterministic stdout, line-aware stderr, phase-specific exit status. |
| P-009 | Blocks/statements | Blocks are statements and lexical scopes; empty blocks are accepted as an edge case; control bodies remain braced and bare expression statements remain unsupported. |
| P-012 | Parser end-of-input | Use normal Bison EOF, not a declared custom `END` token. |
| P-010 | Floating syntax | Decimal digits on both sides of one dot; no exponent/leading-dot/trailing-dot forms. |
| P-011 | Semantic categories | Use the non-overlapping triggers in `docs/LANGUAGE_SPEC.md` §6. |

## Decisions deferred to later milestones

M8 still owns label naming and exact conditional/unconditional jump spelling. M9 owns the final CLI section layout and phase exit integration. M7 resolves the former D-104 instruction-representation question through D-701 to D-706.

Deferred decisions must not change the accepted language. Any actual language extension belongs to optional post-core work and requires explicit later approval.

## Decision record template

```text
ID / date / status
Decision:
Manual basis:
Alternatives considered:
Reason:
Consequences:
Tests/docs affected:
```
