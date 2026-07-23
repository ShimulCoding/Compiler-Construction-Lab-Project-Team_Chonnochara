# Requirements Traceability Checklist

Sources compared:

1. `Compiler Construction Lab Project Manual.pdf` (highest authority, read completely on 21 July 2026)
2. Current committed baseline at `3386ba1` plus the uncommitted M8 working tree
3. Team Chonnochara Codex master project instruction

Status meanings: **Complete** = verified evidence exists; **Complete (documented)** = the governing constraint is recorded but does not imply implementation; **Partial** = illustrative/incomplete evidence only; **Missing** or **Missing implementation** = no required implementation/evidence; **Pending decision/environment** = a contract or prerequisite must be settled; **Ongoing** = continuous team duty; **Optional** = not required.

## Governance and scope

| Requirement | Source | Status | Evidence / action |
| --- | --- | --- | --- |
| Fixed instructor-defined language | Manual §§3, 5 | Complete (documented) | Minimal accepted contract is in `AGENTS.md` and `docs/LANGUAGE_SPEC.md` |
| Six required front-end/TAC modules | Manual §4 | Complete at library/test-driver level | Lexer through semantics are pushed; M8 implements every mandatory TAC form, while the production driver remains M9 |
| TAC is final required output | Manual §§4.6, 6 | Complete (documented) | Scope recorded; no backend planned |
| No mandatory backend, assembly, register allocation, linking, optimization, executable generation, or instruction scheduling | Manual §6 | Complete (documented) | Prohibited before core completion; optional optimization only after all mandatory work |
| C or C++ with Flex/Bison | Manual §§7, FAQ | Complete (environment) | Verified C11/Flex/Bison/GCC/Make toolchain on Ubuntu 24.04.4 LTS under WSL2 |
| GNU Make single-command build | Manual §§7-8 | Partial | The `Makefile` builds every existing front-end/TAC component and phase test; the final compiler target remains M9 |
| Clean professional structure | Manual §8 | Partial | `src/common`, `src/ast`, `src/parser`, `src/lexer`, `src/symbol_table`, `src/semantic`, fixtures, and focused expected/support infrastructure exist; codegen is added with its implementation |
| Generated files excluded where practical | Manual §8 | Complete (M8 working tree) | Generated objects, executables, Bison/Flex output, and routine test results stay under ignored `/build/` |
| Shared public GitHub fork | Manual §9, FAQ | Complete | Team fork exists under `ShimulCoding` |
| Regular meaningful commits from every member | Manual §9 | Complete through M7 | Meaningful reviewed M1-M7 commits are pushed; Mehedi's M8 remains uncommitted pending review |
| AI-assisted work understood by every member | Manual §10 | Ongoing | Must be verified through review/viva preparation |
| Original work and external citations | Manual §17 | Ongoing | Record references as development proceeds |
| Readable code with appropriate explanatory comments | Manual §§1, 17 | Partial | AST C, Flex, Bison grammar/actions, symbol-table/semantic/TAC C, and phase-test code are modular and warning-clean; the production CLI remains |
| Strict 31 July 2026 deadline | Manual §16 | Complete (documented) | Roadmap targets 30 July freeze |

## Fixed lexical and syntactic language

| Requirement | Status | Current evidence / gap |
| --- | --- | --- |
| Types `int`, `float`, `bool` | Complete (M3 lexer) | Exact lowercase keywords return `KW_INT`, `KW_FLOAT`, and `KW_BOOL` |
| Keywords `int float bool if else while print true false` | Complete (M3 lexer) | All nine keyword/Boolean token rules and golden coverage pass |
| Identifier naming rule | Complete (M3 lexer) | `[A-Za-z_][A-Za-z0-9_]*` plus keyword-prefix/case boundary tests pass |
| Integer literals | Complete (M3 lexer) | `[0-9]+` and `0`/`42` goldens pass |
| Floating-point literals | Complete (M3 lexer) | Digits-dot-digits accepts `3.14`/`0.5`; `.5`, `5.`, and exponent form are diagnosed |
| Boolean literals | Complete (M3 lexer) | `true`/`false` return their dedicated keyword tokens |
| Arithmetic `+ - * / %` | Complete (M3 lexer) | All five token rules appear in the all-token golden |
| Relational `< > <= >= == !=` | Complete (M3 lexer) | All six rules and compact single/multi-character overlap goldens pass |
| Logical `&& \|\| !` | Complete (M3 lexer) | All three rules and adjacency golden pass |
| Assignment `=` distinct from equality `==` | Complete (M3 lexer) | `===` produces `EQ` then `ASSIGN` in the reviewed golden |
| Braces, parentheses, semicolons | Complete (M3 lexer) | All five delimiter tokens pass |
| Whitespace discarded with line tracking | Complete (M3 lexer) | Spaces/tabs/blank lines and LF/generated-CRLF location outputs pass |
| Comments discarded | Complete (M3 lexer) | Code-before-comment and full-line `//` pass; block comments remain unsupported and are not discarded |
| Invalid tokens reported with line number | Complete (M3 lexer) | Isolated `@` produces exact line-3 `LEX_INVALID_TOKEN` and exit 1 |
| Declarations and assignments | Complete (M4 syntax/AST) | Both declaration forms and assignment statements parse into the expected AST; semantic compatibility remains later |
| `if` and `if-else` | Complete (M4 syntax/AST) | Braced forms parse into one `If` node with optional else block; unbraced forms reject |
| `while` | Complete (M4 syntax/AST) | Braced loop and condition parse into the expected AST |
| `print` statement | Complete (M4 syntax/AST) | Identifier-only print parses; literal print rejects with an exact diagnostic |
| Exact `print` operand grammar | Complete (documented) | Minimal contract accepts `print IDENTIFIER;` only; inherited `print 0;` sketch must change |
| Nested blocks and proper scope | Complete (M6 pushed) | Parser-built blocks drive exactly one semantic scope each; nested shadowing, restoration, sibling isolation, and inactive-history diagnostics pass |
| Formal complete unambiguous CFG | Complete (M4 parser) | Strict BNF is implemented directly; Bison reports zero shift/reduce and zero reduce/reduce conflicts |
| Precedence and associativity | Complete (M4 parser/AST) | Layered grammar and reviewed AST golden prove grouping; TAC evidence remains later |
| Basic Bison error recovery | Complete (M4 parser) | Exact cases demonstrate synchronization at semicolon and closing brace without AST output |
| Declaration initializer syntax | Complete (M4 parser/AST) | Both forms parse, and manual §4.5 `bool b = 5 + 3.2;` reaches an initializer AST for later semantics |
| Exact block/statement/empty-program grammar | Complete (documented) | Nonempty program; block statements and empty block contents accepted; bare expression statements rejected |
| Parser end-of-input boundary | Complete (documented) | Normal Bison EOF only; no custom `END` token and source catalog remains 32 tokens |
| Exact floating-literal spellings | Complete (documented) | `[0-9]+\.[0-9]+` only; other numeric forms unsupported |
| Semantic error-category taxonomy | Complete (M6 pushed) | Six non-overlapping triggers are implemented with exact line/code/message/exit goldens |

## Compiler modules

| Module requirement | Status | Required completion evidence |
| --- | --- | --- |
| Flex lexer recognizes all tokens/comments/errors | Complete (M3 committed) | Production Flex source plus 10 lexer cases/goldens in pushed commit `084a3df` |
| Bison parser recognizes CFG and recovers basically | Complete (M4 pushed) | Complete grammar, zero-conflict build, stable diagnostics, both recovery boundaries, and 32 parser cases in pushed commit `6d6fc30` |
| AST built during parsing | Complete (M4 syntax/AST) | Valid sources construct the existing AST; seven parser-built outputs match exact goldens |
| Readable AST printing | Complete (M2 unit) | Deterministic line-aware indentation printer matches `tests/expected/ast_unit.stdout` |
| Symbol name/type/scope/declaration line | Complete (M5 pushed) | Opaque stable records expose copied name, `ValueType`, `SourceLocation`, scope ID, and depth through read-only views; unit/golden tests pass |
| Symbol insert/lookup/enter/exit scope | Complete (M5 pushed) | Explicit current/active/history lookup, insertion statuses, global/nested lifecycle, monotonic IDs, printer, and cleanup pass 30 direct tests |
| Undeclared variable detection | Complete (M6 pushed) | Isolated never-declared and self-initializer fixtures match `SEM_UNDECLARED` goldens |
| Same-scope redeclaration detection | Complete (M6 pushed) | Duplicate fixture emits `SEM_REDECLARATION`, preserves the first binding, and still checks independent initializer errors |
| Scope violation detection | Complete (M6 pushed) | Standalone/sibling exited declarations produce `SEM_SCOPE_VIOLATION`, while never-declared names remain distinct |
| Type mismatch detection | Complete (M6 pushed) | Initialized declaration, equality-domain, and non-Boolean condition fixtures match exact `SEM_TYPE_MISMATCH` goldens |
| Invalid assignment detection | Complete (M6 pushed) | Exact incompatible standalone assignment matches `SEM_INVALID_ASSIGNMENT` |
| Invalid expression/operator detection | Complete (M6 pushed) | Arithmetic, remainder, logical/not, and relational operand fixtures match `SEM_INVALID_OPERATOR` |
| Line-aware human-readable semantic errors | Complete (M6 pushed) | Twenty invalid cases match exact source-ordered stderr/exit goldens |
| TAC assignment and arithmetic | Complete (M8 regression) | Exact goldens preserve direct assignments and left-to-right temporary lowering for `+ - * / %` |
| TAC initialized declaration | Complete (M8 regression) | Initializer-expression temporaries precede the storage assignment; plain declarations emit none |
| TAC relational and logical expressions | Complete (M8 regression) | Exact goldens cover `< > <= >= == != && || !` with materialized Boolean temporaries |
| TAC `if`, `if-else`, `while` | Complete (M8 working tree) | Structural `.L<n>` labels and exact conditional/unconditional jump goldens cover simple, nested, empty, and expression conditions |
| TAC `print` | Complete (M8 regression) | Global and scope-qualified identifier print instructions match inside and outside control flow |
| TAC respects precedence | Complete (M8 working tree) | Expression and loop-condition goldens prove AST-driven ordering, including reevaluation after the loop-start label |

## Testing and quality evidence

| Requirement | Status | Current evidence / action |
| --- | --- | --- |
| Non-trivial valid source reaches TAC | Complete at test-driver level (M8) | `full_control_flow.mc` reaches exact while plus if-else TAC; the production CLI remains M9 |
| Lexical error test | Complete (M3 lexer) | Runnable invalid-character and malformed-number fixtures have exact stderr/exit goldens |
| Syntax error test | Complete (M4 parser) | Fifteen isolated syntax fixtures plus two recovery cases and two lexical/parser integration cases have exact stderr/exit evidence |
| Separate test for every six semantic rules | Complete (M6 pushed) | Isolated executable fixtures cover undeclared, redeclaration, scope violation, type mismatch, invalid assignment, and invalid operator |
| Every input paired with expected/actual output | Partial | Invalid semantic cases now have exact tracked stderr/exit plus ephemeral actual results; TAC/end-to-end evidence remains |
| Executable test fixtures | Partial | Lexer/parser/semantic `.mc` fixtures plus one `.txt` and generated CRLF coverage run automatically; TAC/end-to-end fixtures remain |
| Automated test command | Partial | `make test` validates the header, 15 AST, 30 symbol-table, 10 lexer, 32 parser, 26 semantic, 17 TAC unit, and 20 TAC integration cases; final CLI/release evidence remain |
| Full type/literal/operator coverage | Partial | Lexical/parser coverage is complete and M6 validates every operator family, mixed numeric promotion, and representative exact storage combinations including rejected numeric conversion; exhaustive nine-cell and TAC coverage remain |
| Scope/control-flow integration coverage | Complete through semantics | Parser-built standalone/control/nested/sibling blocks pass semantic scope, shadowing, restoration, condition, and history tests |
| AST and TAC golden output | Complete through TAC library | Direct/seven parser-built AST goldens plus 19 valid TAC goldens cover expressions, scopes, and mandatory control flow |
| Build/run/error screenshots | Missing | Final-deliverable task |
| Clean-environment validation | Missing | M16 exit criterion |

## Documentation and delivery

| Deliverable | Status | Current evidence / action |
| --- | --- | --- |
| Team project README with exact build/run instructions | Missing | Inherited template README is generic and mojibaked in places |
| Complete buildable source | Partial | Front-end plus complete mandatory TAC library and phase tests are buildable; the production driver remains |
| Formal CFG in report | Missing | M1 then report integration |
| Project report with manual chapter structure | Partial | `REPORT_OUTLINE.md` initialized; content/evidence pending |
| Architecture/module interaction documentation | Partial | AST through M8 TAC, scope-safe storage, labels/jumps, ownership, gates, dependencies, and tests are documented; production CLI remains |
| Presentation slides | Missing | Notes skeleton only |
| Valid and invalid sample programs | Partial | Executable phase fixtures exist; final curated source-to-TAC examples remain |
| Sample outputs | Partial | Exact phase and full-language TAC goldens exist; final production compiler output remains missing |
| Screenshots | Missing | Capture after stable integration |
| Exact compilation/execution instructions | Partial | `make`, `make test`, and `make clean` validate the M8 library/test-driver pipeline; the final compiler command awaits integration |
| Live valid and invalid demonstration | Missing | Demo sequence planned, compiler absent |
| Lexical, syntax, semantic, and TAC demonstrations | Partial | Test-only executables now demonstrate complete control-flow TAC; final CLI/live-demo packaging remains |
| Individual viva preparation | Partial | AST/build, Flex, Bison, symbol table, semantics, and complete M8 TAC are documented; final CLI/release topics remain |
| Optional video | Optional | Consider only after mandatory deliverables |

## Evaluation priority awareness

Indicative weights from the manual: lexer 10%, parser 15%, AST 10%, symbol table 10%, semantic analysis 20%, TAC 15%, documentation/report 10%, presentation/demo 10%. Individual viva is a pass/fail understanding gate that may scale individual results. The weights guide scheduling but do not waive any requirement.
