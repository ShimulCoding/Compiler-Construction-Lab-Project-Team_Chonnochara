# Requirements Traceability Checklist

Sources compared:

1. `Compiler Construction Lab Project Manual.pdf` (highest authority, read completely on 21 July 2026)
2. Current committed baseline at `6d6fc30` plus the uncommitted M5 working tree
3. Team Chonnochara Codex master project instruction

Status meanings: **Complete** = verified evidence exists; **Complete (documented)** = the governing constraint is recorded but does not imply implementation; **Partial** = illustrative/incomplete evidence only; **Missing** or **Missing implementation** = no required implementation/evidence; **Pending decision/environment** = a contract or prerequisite must be settled; **Ongoing** = continuous team duty; **Optional** = not required.

## Governance and scope

| Requirement | Source | Status | Evidence / action |
| --- | --- | --- | --- |
| Fixed instructor-defined language | Manual §§3, 5 | Complete (documented) | Minimal accepted contract is in `AGENTS.md` and `docs/LANGUAGE_SPEC.md` |
| Six required front-end/TAC modules | Manual §4 | Partial | Lexer, parser, and AST are pushed; the isolated symbol table is implemented/validated in the M5 working tree; semantics, TAC, and final driver remain missing |
| TAC is final required output | Manual §§4.6, 6 | Complete (documented) | Scope recorded; no backend planned |
| No mandatory backend, assembly, register allocation, linking, optimization, executable generation, or instruction scheduling | Manual §6 | Complete (documented) | Prohibited before core completion; optional optimization only after all mandatory work |
| C or C++ with Flex/Bison | Manual §§7, FAQ | Complete (environment) | Verified C11/Flex/Bison/GCC/Make toolchain on Ubuntu 24.04.4 LTS under WSL2 |
| GNU Make single-command build | Manual §§7-8 | Partial | The `Makefile` builds every existing AST/lexer/parser/symbol-table component and phase test; the full compiler target grows with later modules |
| Clean professional structure | Manual §8 | Partial | `src/common`, `src/ast`, `src/parser`, `src/lexer`, `src/symbol_table`, fixtures, and focused expected/support infrastructure exist; remaining module directories are added with their implementations |
| Generated files excluded where practical | Manual §8 | Complete (M5 working tree) | Generated objects, executables, Bison/Flex output, and routine test results stay under ignored `/build/` |
| Shared public GitHub fork | Manual §9, FAQ | Complete | Team fork exists under `ShimulCoding` |
| Regular meaningful commits from every member | Manual §9 | Complete for first cycle | Meaningful reviewed commits from Shimul, Nayem, Dipro, and Mehedi are pushed; M5 begins the second cycle and remains uncommitted pending Shimul review |
| AI-assisted work understood by every member | Manual §10 | Ongoing | Must be verified through review/viva preparation |
| Original work and external citations | Manual §17 | Ongoing | Record references as development proceeds |
| Readable code with appropriate explanatory comments | Manual §§1, 17 | Partial | AST C, Flex, Bison grammar/actions, symbol-table C, and phase-test code are modular and warning-clean; later modules remain |
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
| Nested blocks and proper scope | Partial | Parser-built block ASTs and isolated M5 global/nested/sibling scope mechanics pass; semantic AST traversal/enforcement remains M6 |
| Formal complete unambiguous CFG | Complete (M4 parser) | Strict BNF is implemented directly; Bison reports zero shift/reduce and zero reduce/reduce conflicts |
| Precedence and associativity | Complete (M4 parser/AST) | Layered grammar and reviewed AST golden prove grouping; TAC evidence remains later |
| Basic Bison error recovery | Complete (M4 parser) | Exact cases demonstrate synchronization at semicolon and closing brace without AST output |
| Declaration initializer syntax | Complete (M4 parser/AST) | Both forms parse, and manual §4.5 `bool b = 5 + 3.2;` reaches an initializer AST for later semantics |
| Exact block/statement/empty-program grammar | Complete (documented) | Nonempty program; block statements and empty block contents accepted; bare expression statements rejected |
| Parser end-of-input boundary | Complete (documented) | Normal Bison EOF only; no custom `END` token and source catalog remains 32 tokens |
| Exact floating-literal spellings | Complete (documented) | `[0-9]+\.[0-9]+` only; other numeric forms unsupported |
| Semantic error-category taxonomy | Complete (documented) | Six non-overlapping triggers finalized in `docs/LANGUAGE_SPEC.md`; implementation/tests missing |

## Compiler modules

| Module requirement | Status | Required completion evidence |
| --- | --- | --- |
| Flex lexer recognizes all tokens/comments/errors | Complete (M3 committed) | Production Flex source plus 10 lexer cases/goldens in pushed commit `084a3df` |
| Bison parser recognizes CFG and recovers basically | Complete (M4 pushed) | Complete grammar, zero-conflict build, stable diagnostics, both recovery boundaries, and 32 parser cases in pushed commit `6d6fc30` |
| AST built during parsing | Complete (M4 syntax/AST) | Valid sources construct the existing AST; seven parser-built outputs match exact goldens |
| Readable AST printing | Complete (M2 unit) | Deterministic line-aware indentation printer matches `tests/expected/ast_unit.stdout` |
| Symbol name/type/scope/declaration line | Implemented/validated (M5 working tree) | Opaque stable records expose copied name, `ValueType`, `SourceLocation`, scope ID, and depth through read-only views; unit/golden tests pass |
| Symbol insert/lookup/enter/exit scope | Implemented/validated (M5 working tree) | Explicit current/active/history lookup, insertion statuses, global/nested lifecycle, monotonic IDs, printer, and cleanup pass 30 direct tests |
| Undeclared variable detection | Missing | Isolated semantic test |
| Same-scope redeclaration detection | Partial | M5 rejects duplicate insertion and preserves the first binding; M6 must translate this status into `SEM_REDECLARATION` while walking declarations |
| Scope violation detection | Partial | M5 active/history lookup distinguishes inactive from never-declared records; M6 must emit the correct line-aware semantic diagnostic |
| Type mismatch detection | Missing | Defined type matrix + tests |
| Invalid assignment detection | Missing | Isolated assignment-policy test |
| Invalid expression/operator detection | Missing | No inherited test exists; add explicit operand test |
| Line-aware human-readable semantic errors | Missing | Golden diagnostics |
| TAC assignment and arithmetic | Missing | Temporary-variable goldens |
| TAC initialized declaration | Missing | Initializer-expression temporaries followed by declared-name store; plain declaration emits none |
| TAC relational and logical expressions | Missing | Boolean-expression goldens |
| TAC `if`, `if-else`, `while` | Missing | Label/jump goldens |
| TAC `print` | Missing | Golden TAC |
| TAC respects precedence | Missing | AST/TAC precedence goldens |

## Testing and quality evidence

| Requirement | Status | Current evidence / action |
| --- | --- | --- |
| Non-trivial valid source reaches TAC | Missing | `complete_program.md` is only a sketch |
| Lexical error test | Complete (M3 lexer) | Runnable invalid-character and malformed-number fixtures have exact stderr/exit goldens |
| Syntax error test | Complete (M4 parser) | Fifteen isolated syntax fixtures plus two recovery cases and two lexical/parser integration cases have exact stderr/exit evidence |
| Separate test for every six semantic rules | Partial | Five labels exist; invalid expression is absent and two overlap |
| Every input paired with expected/actual output | Partial | AST, symbol-table, lexer, and parser phase cases use tracked expected plus ephemeral actual results; semantic/TAC cases remain |
| Executable test fixtures | Partial | M3 provides primary `.mc`, one `.txt`, generated CRLF, expected token streams, and lexical diagnostics; later compiler-phase fixtures remain |
| Automated test command | Partial | `make test` validates the header, 15 AST tests/golden, 30 symbol-table tests/golden, 10 lexer cases, and 32 parser cases; semantic/TAC phases must extend it |
| Full type/literal/operator coverage | Partial | Lexical and parser coverage for all required literal/operator families passes; semantic signatures and TAC remain |
| Scope/control-flow integration coverage | Partial | M5 directly covers nested shadowing, outer restoration, sibling isolation, and inactive history; AST/block semantic integration remains M6 |
| AST and TAC golden output | Partial | Direct-construction AST golden plus seven parser-built AST goldens pass; all TAC goldens remain |
| Build/run/error screenshots | Missing | Final-deliverable task |
| Clean-environment validation | Missing | M16 exit criterion |

## Documentation and delivery

| Deliverable | Status | Current evidence / action |
| --- | --- | --- |
| Team project README with exact build/run instructions | Missing | Inherited template README is generic and mojibaked in places |
| Complete buildable source | Partial | AST, generated lexer/parser, symbol table, and phase tests are buildable; semantic/TAC/final driver remain |
| Formal CFG in report | Missing | M1 then report integration |
| Project report with manual chapter structure | Partial | `REPORT_OUTLINE.md` initialized; content/evidence pending |
| Architecture/module interaction documentation | Partial | AST, lexer, parser, symbol-table records/scopes/lookups/ownership, generated dependencies, and tests are documented; later modules remain |
| Presentation slides | Missing | Notes skeleton only |
| Valid and invalid sample programs | Partial | Markdown sketches only |
| Sample outputs | Missing | No actual compiler output |
| Screenshots | Missing | Capture after stable integration |
| Exact compilation/execution instructions | Partial | `make`, `make test`, and `make clean` work through M5; temporary phase tests are documented, while the final compiler command awaits integration |
| Live valid and invalid demonstration | Missing | Demo sequence planned, compiler absent |
| Lexical, syntax, semantic, and TAC demonstrations | Missing | No executable |
| Individual viva preparation | Partial | AST/build, Flex, Bison, and implemented symbol-table records/scopes/lookups/ownership are documented; semantic/TAC modules remain |
| Optional video | Optional | Consider only after mandatory deliverables |

## Evaluation priority awareness

Indicative weights from the manual: lexer 10%, parser 15%, AST 10%, symbol table 10%, semantic analysis 20%, TAC 15%, documentation/report 10%, presentation/demo 10%. Individual viva is a pass/fail understanding gate that may scale individual results. The weights guide scheduling but do not waive any requirement.
