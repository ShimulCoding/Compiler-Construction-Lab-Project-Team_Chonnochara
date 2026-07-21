# Design Decisions

Status: M1-M2 decisions are approved and committed. M3 decisions below are validated, reviewed, and approved as Dipro's milestone.

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
| D-207 | Keep all 32 token declarations in the minimal `src/parser/parser.y`; generate the shared Bison header before any future lexer compilation. The current one-token placeholder production is not the language parser. | Establishes one token-number authority now, satisfies the M2 roadmap, and avoids duplicating token enums in Flex/C. M4 replaces the placeholder production with the approved CFG. |
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

| ID | Status | Decision area | Why deferred |
| --- | --- | --- | --- |
| D-103 | Proposed for M5 | Exact scope-record/symbol-list storage structure | Scope behavior is fixed, but data structure implementation belongs to the symbol-table milestone. |
| D-104 | Proposed for M8 | Exact TAC instruction structure and final textual spelling | Determinism is fixed; representation belongs to TAC implementation. |

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
