# Universal Shōbu Protocol Specifications

Formal game specification, canonical state serialization (SFEN), and communication
protocol for the abstract strategy game Shōbu, with runnable examples in **C, C++,
Python, Zig, Rust, and JavaScript**.

**Version 1.0 — independent, repository-defined standard.** The game rules are
grounded in publisher-maintained sources. USP's representation and adjudication
conventions are explicitly documented; this is not a publisher endorsement or a
claim of adoption by other software.

| Document | Contents |
| --- | --- |
| [Formal game specification](docs/game.md) | Board identity, coordinates, setup, complete turns, exact pushing transitions, validity and outcomes |
| [SFEN 1.0](docs/sfen.md) | Grammar, canonical parsing, structural validation, examples and limits |
| [SSGF v1](docs/ssgf-v1.md) | Exact cani saved-game specification: RSSGF, BSSGF, metadata, replay and results |
| [USP 1.0](docs/protocol.md) | Transport, negotiation, commands, errors, atomicity and example session |
| [Reference examples](examples/README.md) | Six language entry points, build commands and library APIs |
| [Conformance tests](tests/README.md) | Golden fixtures, seeded differential checks and framing tests |

## Quick start

Run `python examples/python/engine.py`, then type:

```text
hello 1
state
play 0a1a2:1b1b2
state
go
quit
```

Each request yields exactly one response. `go` proposes a legal turn without
committing it; `play` validates and commits both parts atomically. The engines use
the first legal turn as a reproducible selection policy and do not implement a
competitive search algorithm.

Initial SFEN (boards 0–3, ranks 4–1, Black to play, zero completed turns):

```text
WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB b 0
```

The complete-turn notation `0a1a2:1b1b2` encodes the passive leg on board 0 and
the matching aggressive leg on board 1. Coordinates are always from Black's
perspective. See the formal specification before implementing an independent
engine.

## Build and verify

Python 3.10+ is required for the tooling; example engines otherwise need only
their own compiler/interpreter. Native builds require GCC, G++, Rust, and pinned
Zig 0.15.2. JavaScript requires Node.js 18+.

```text
python scripts/build.py --require-all
python -m unittest discover -s tests -p test_rules.py -v
node tests/javascript_api.mjs
python tests/conformance.py --require-all
```

Use `--zig /path/to/zig` for a compiler outside PATH. Without `--require-all`,
the build skips unavailable compilers and conformance tests use available engines.
The strict flags are recommended for release checks. GitHub Actions builds and
checks all six on Ubuntu. C++ and Zig intentionally share the C core through native
facades; Python, JavaScript, Rust, and C contain direct rules implementations.

## Versioning and contribution

The [SSGF v1 specification](docs/ssgf-v1.md), saved-game examples in
`examples/ssgf/`, and fixtures in `tests/fixtures/ssgf/` are verbatim copies from
[cani commit 18ef4ab](https://github.com/DaShobu/cani/commit/18ef4abbd48aa2ced7b262e1296635d82c8347d3).
They define the same SSGF, without local rule changes. SSGF is separate from the
USP 1.0 command protocol; its codecs are supplied by cani's optional `ssgf` feature.

Any incompatible grammar, command semantics, coordinate convention, or game
profile change requires a new protocol major version. Clarifications and
additional tests that preserve accepted inputs and outputs may be minor revisions.
New mandatory commands cannot silently extend version 1. Changes should update
the normative documents, hand-written vectors, and all affected examples together.
The documents take precedence over an implementation bug; disagreement between
examples is a conformance failure, not a new rule. This repository's original
specifications and code are licensed under [Apache 2.0](LICENSE).
