# Requirements Traceability Checklist

Sources compared:

1. `Compiler Construction Lab Project Manual.pdf` (highest authority, read completely on 21 July 2026)
2. Current repository baseline at `7d073fa`
3. Team Chonnochara Codex master project instruction

Status meanings: **Complete** = verified evidence exists; **Complete (documented)** = the governing constraint is recorded but does not imply implementation; **Partial** = illustrative/incomplete evidence only; **Missing** or **Missing implementation** = no required implementation/evidence; **Pending decision/environment** = a contract or prerequisite must be settled; **Ongoing** = continuous team duty; **Optional** = not required.

## Governance and scope

| Requirement | Source | Status | Evidence / action |
| --- | --- | --- | --- |
| Fixed instructor-defined language | Manual §§3, 5 | Complete (documented) | Minimal accepted contract is in `AGENTS.md` and `docs/LANGUAGE_SPEC.md` |
| Six required front-end/TAC modules | Manual §4 | Missing | No source tree exists |
| TAC is final required output | Manual §§4.6, 6 | Complete (documented) | Scope recorded; no backend planned |
| No mandatory backend, assembly, register allocation, linking, optimization, executable generation, or instruction scheduling | Manual §6 | Complete (documented) | Prohibited before core completion; optional optimization only after all mandatory work |
| C or C++ with Flex/Bison | Manual §§7, FAQ | Pending environment | C11 and pinned Ubuntu 24.04/WSL2 plan documented; no installation approved/performed |
| GNU Make single-command build | Manual §§7-8 | Missing | No Makefile |
| Clean professional structure | Manual §8 | Partial | `docs/`, `tests/`, and `examples/` exist; `src/`, Makefile, and build layout are absent |
| Generated files excluded where practical | Manual §8 | Partial | Baseline `.gitignore` covers common names; build layout still pending |
| Shared public GitHub fork | Manual §9, FAQ | Complete | Team fork exists under `ShimulCoding` |
| Regular meaningful commits from every member | Manual §9 | Partial | First meaningful team commit belongs to Shimul; Nayem, Dipro, and Mehedi remain pending |
| AI-assisted work understood by every member | Manual §10 | Ongoing | Must be verified through review/viva preparation |
| Original work and external citations | Manual §17 | Ongoing | Record references as development proceeds |
| Readable code with appropriate explanatory comments | Manual §§1, 17 | Missing implementation | No source exists; comment intent/invariants without restating obvious code |
| Strict 31 July 2026 deadline | Manual §16 | Complete (documented) | Roadmap targets 30 July freeze |

## Fixed lexical and syntactic language

| Requirement | Status | Current evidence / gap |
| --- | --- | --- |
| Types `int`, `float`, `bool` | Missing implementation | Only Markdown examples |
| Keywords `int float bool if else while print true false` | Missing implementation | No lexer |
| Identifier naming rule | Missing implementation | No lexer/boundary tests |
| Integer literals | Missing implementation | Sketches only |
| Floating-point literals | Missing implementation | No test uses a float literal |
| Boolean literals | Missing implementation | Sketches use `true`/`false` |
| Arithmetic `+ - * / %` | Missing implementation | `/` and `%` absent from tests |
| Relational `< > <= >= == !=` | Missing implementation | `<`, `<=`, `!=` absent from tests |
| Logical `&& \|\| !` | Missing implementation | Entire family absent from tests |
| Assignment `=` distinct from equality `==` | Missing implementation | No lexer/longest-match oracle |
| Braces, parentheses, semicolons | Missing implementation | Sketches only |
| Whitespace discarded with line tracking | Missing implementation | No stress test |
| Comments discarded | Missing implementation | M1 selects `//` line comments; lexer/test still missing |
| Invalid tokens reported with line number | Missing implementation | `@` sketch has expected prose without line number |
| Declarations and assignments | Missing implementation | Uninitialized forms have Markdown sketches; initialized form is documented from manual §4.5 but has no executable parser/semantic test |
| `if` and `if-else` | Missing implementation | Sketch only |
| `while` | Missing implementation | Sketch only |
| `print` statement | Missing implementation | Markdown sketches only |
| Exact `print` operand grammar | Complete (documented) | Minimal contract accepts `print IDENTIFIER;` only; inherited `print 0;` sketch must change |
| Nested blocks and proper scope | Missing implementation | CFG now accepts block statements, including nested/empty blocks; implementation and scope tests remain missing |
| Formal complete unambiguous CFG | Complete (documented) | Strict BNF and rule traceability finalized in `docs/GRAMMAR.md`; Bison implementation missing |
| Precedence and associativity | Complete (documented) | Layered table finalized; executable AST/TAC oracle missing |
| Basic Bison error recovery | Missing | Combined syntax sketch has no actual result |
| Declaration initializer syntax | Complete (documented) | Both declaration forms accepted so manual §4.5 `bool b = 5 + 3.2;` reaches semantics; implementation missing |
| Exact block/statement/empty-program grammar | Complete (documented) | Nonempty program; block statements and empty block contents accepted; bare expression statements rejected |
| Parser end-of-input boundary | Complete (documented) | Normal Bison EOF only; no custom `END` token and source catalog remains 32 tokens |
| Exact floating-literal spellings | Complete (documented) | `[0-9]+\.[0-9]+` only; other numeric forms unsupported |
| Semantic error-category taxonomy | Complete (documented) | Six non-overlapping triggers finalized in `docs/LANGUAGE_SPEC.md`; implementation/tests missing |

## Compiler modules

| Module requirement | Status | Required completion evidence |
| --- | --- | --- |
| Flex lexer recognizes all tokens/comments/errors | Missing | Lexer source + lexical goldens |
| Bison parser recognizes CFG and recovers basically | Missing | Grammar + conflict report/recovery tests |
| AST built during parsing | Missing | Node implementation + parser actions |
| Readable AST printing | Missing | Stable golden output |
| Symbol name/type/scope/declaration line | Missing | Entry structure + tests |
| Symbol insert/lookup/enter/exit scope | Missing | Unit/integration tests |
| Undeclared variable detection | Missing | Isolated semantic test |
| Same-scope redeclaration detection | Missing | Isolated semantic test |
| Scope violation detection | Missing | Active/history lookup test |
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
| Lexical error test | Partial | `@` sketch exists; no runnable input/actual output |
| Syntax error test | Partial | Combined missing `;`/`)` sketch; no actual output |
| Separate test for every six semantic rules | Partial | Five labels exist; invalid expression is absent and two overlap |
| Every input paired with expected/actual output | Missing | Invalid prose expectations only; no actual outputs |
| Executable test fixtures | Missing | All current inputs are fenced Markdown; future fixtures use primary `.mc` with `.txt` compatibility coverage |
| Automated test command | Missing | No runner or `make test` |
| Full type/literal/operator coverage | Missing | Major families absent |
| Scope/control-flow integration coverage | Missing | No valid nested/shadowing case |
| AST and TAC golden output | Missing | No output artifacts |
| Build/run/error screenshots | Missing | Final-deliverable task |
| Clean-environment validation | Missing | M16 exit criterion |

## Documentation and delivery

| Deliverable | Status | Current evidence / action |
| --- | --- | --- |
| Team project README with exact build/run instructions | Missing | Inherited template README is generic and mojibaked in places |
| Complete buildable source | Missing | No source |
| Formal CFG in report | Missing | M1 then report integration |
| Project report with manual chapter structure | Partial | `REPORT_OUTLINE.md` initialized; content/evidence pending |
| Architecture/module interaction documentation | Partial | Proposed architecture only; update with implementation |
| Presentation slides | Missing | Notes skeleton only |
| Valid and invalid sample programs | Partial | Markdown sketches only |
| Sample outputs | Missing | No actual compiler output |
| Screenshots | Missing | Capture after stable integration |
| Exact compilation/execution instructions | Missing | Generic template commands are not executable here |
| Live valid and invalid demonstration | Missing | Demo sequence planned, compiler absent |
| Lexical, syntax, semantic, and TAC demonstrations | Missing | No executable |
| Individual viva preparation | Partial | Notes initialized; implementation explanations pending |
| Optional video | Optional | Consider only after mandatory deliverables |

## Evaluation priority awareness

Indicative weights from the manual: lexer 10%, parser 15%, AST 10%, symbol table 10%, semantic analysis 20%, TAC 15%, documentation/report 10%, presentation/demo 10%. Individual viva is a pass/fail understanding gate that may scale individual results. The weights guide scheduling but do not waive any requirement.
