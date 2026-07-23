# Development and Contribution Roadmap

Status: M1-M5 are committed and pushed. M6 full semantic validation is implemented and tested, pending Nayem's review and commit approval. Nayem remains the next intended contributor.

The initialization documents were incorporated into Shimul's coherent M1 foundation milestone. This consumes Shimul's current turn and advances the intended contribution cycle to Nayem without changing the existing M2-M16 ownership sequence.

## Deadline strategy

- **21-27 July:** complete the compiler modules and first end-to-end pipeline.
- **28 July:** harden diagnostics and comprehensive automated tests.
- **29 July:** finish README/report/presentation/demo/viva evidence.
- **30 July:** clean-environment release validation and submission freeze.
- **31 July:** submission buffer only; avoid relying on deadline-day commits.

The cycle is fixed unless a real team constraint is recorded:

```text
Shimul -> Nayem -> Dipro -> Mehedi -> repeat
```

## Milestones

| ID | Target | Intended owner | Meaningful deliverable and exit criteria | Depends on |
| --- | --- | --- | --- | --- |
| M1 | 21 Jul | Shimul | Finalize the 32-token catalog, formal CFG including nested/empty blocks and optional declaration initializers, precedence/associativity, type/scope/error rules, `.mc`/source-path test conventions, normal Bison EOF boundary, and exact pinned WSL2/Ubuntu setup plan without installing it. Exit after internal consistency validation, Shimul review, and approved commit. | Audit |
| M2 | 22 Jul | Nayem | After separately approved toolchain setup succeeds, establish the C/Make foundation, minimal automated test target, generated-file layout/ignore rules, and minimal Bison `%token` interface that produces the lexer's shared header. Implement the AST data model—including block statements and optional declaration-initializer children—source locations, cleanup, printer, and focused tests. | M1 approved/committed; environment gate passed |
| M3 | 22 Jul | Dipro | Implement Flex rules for every required lexical element, comments, longest-match cases, line tracking, invalid-token diagnostics, `%option noyywrap`, and lexer tests against the generated token header. | M1-M2 token interface |
| M4 | 23 Jul | Mehedi | Implement the Bison CFG with normal EOF, both declaration forms, standalone/empty blocks, structural precedence, AST-building actions, line-aware diagnostics, semicolon/brace recovery, lexical-error duplicate suppression, zero-conflict evidence, and parser tests. Completed, reviewed, committed, and pushed as `6d6fc30`. | M1-M3 |
| M5 | 24 Jul | Shimul | Implement nested symbol-table scopes, insertion/current/active/history lookup, monotonic IDs, declaration-order retention, initializer-order support, shadowing/restoration, cleanup, printer golden, and direct unit tests. Completed, committed, and pushed as `1f319c6`. | M2 |
| M6 | 24 Jul | Nayem | Implement the complete semantic pass: declaration/use analysis, one scope per block, inferred expression types, exact initializer/assignment compatibility, operator/condition validation, deterministic diagnostics, cascade policy, and focused parser-to-AST integration tests. Implementation/tests complete; owner review and approved commit remain. This user-approved scope absorbs the former M7 type-checking deliverable. | M4-M5 |
| M7 | 25 Jul | Dipro | Implement deterministic TAC data structures plus generation for literals, identifiers, initialized declarations, arithmetic/relational/logical expressions, assignment, standalone blocks, and print with goldens. Do not add control-flow lowering yet. | M6 |
| M8 | 26 Jul | Mehedi | Extend TAC with deterministic labels and jumps for `if`, `if-else`, and `while`; add nested-control-flow goldens. | M7 |
| M9 | 26 Jul | Shimul | Integrate the CLI and full phase gates; make a non-trivial program pass source -> AST -> semantics -> TAC; define stable exit codes and output streams. | M3-M8 |
| M10 | 27 Jul | Nayem | Expand executable end-to-end `.mc`/`.txt` fixtures, expected outputs, and source-path/phase-gate coverage for every required success/error category. | M9 |
| M11 | 28 Jul | Dipro | Harden lexer/parser recovery, semantic/TAC diagnostics and output, fix edge cases found by the full suite, and validate unseen-input behavior. | M10 |
| M12 | 28 Jul | Mehedi | Conduct cross-module integration review, fill missing test-matrix evidence, and validate clean source-to-TAC behavior before documentation freeze. | M11 |
| M13 | 29 Jul | Shimul | Replace the template README with accurate project/team/build/run/test documentation; complete architecture and report technical sections with citations. | M10-M12 |
| M14 | 29 Jul | Nayem | Prepare slides, diagrams, sample outputs, screenshots, and a reproducible live-demo sequence covering valid and all error phases. | M13 |
| M15 | 30 Jul | Dipro | Conduct cross-module review and viva rehearsal; document important functions/data structures and resolve comprehensibility or code-quality findings. | M13-M14 |
| M16 | 30 Jul | Mehedi | Validate from a clean supported environment, run the full suite, verify generated files/secrets are excluded, finalize deliverables, and prepare the release/submission checklist. | M15 |

## Milestone completion rule

A milestone is complete only when:

1. the intended owner understands and accepts it;
2. affected module tests and integrations were run;
3. failures introduced by the change were resolved or honestly recorded;
4. `PROJECT_STATE`, `DEVELOPMENT_LOG`, and `TEST_MATRIX` were updated;
5. relevant architecture/report/presentation/viva documents were updated;
6. the diff and staged set contain no secrets or unnecessary generated files;
7. one accurately attributed, coherent commit was created and its metadata verified; and
8. the next intended contributor was advanced.

Parallel research or pair work is welcome, but commits must preserve truthful ownership and the intended cycle. If schedule pressure forces a cycle exception, document the reason before changing `PROJECT_STATE.md`.

## Critical path

```text
language contract
  -> AST/token interfaces
  -> lexer + parser
  -> symbol table + semantic analysis
  -> expression TAC + control-flow TAC
  -> CLI integration
  -> automated end-to-end suite
  -> documentation/demo/viva evidence
  -> clean release validation
```

Bonus features are outside this roadmap until M16 is complete.
