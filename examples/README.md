# Reference engines

Every example implements the entire mandatory USP 1.0 protocol: canonical SFEN,
complete-turn validation, pushing/capture, legal generation, results, and a
deterministic first-legal-move selector. They are educational engines, not strong
search engines. Python, JavaScript, Rust, and C implement the rules directly;
C++ and Zig demonstrate native integration with the shared C core and provide
idiomatic position APIs. Neither invokes another process or language interpreter.

Run from the repository root, creating `build/` first:

| Language | Build / run |
| --- | --- |
| Python 3.10+ | `python examples/python/engine.py` |
| JavaScript, Node.js 18+ | `node examples/javascript/engine.mjs` |
| C99 | `gcc -std=c99 -O2 -Wall -Wextra -Werror examples/c/main.c examples/c/shobu.c -o build/shobu-c` |
| C++17 | `g++ -std=c++17 -O2 -Wall -Wextra -Werror examples/cpp/main.cpp -o build/shobu-cpp` |
| Rust 2021 | `rustc --edition=2021 -D warnings examples/rust/engine.rs -o build/shobu-rust` |
| Zig 0.15.2 | `zig build-exe examples/zig/main.zig examples/c/shobu.c -I examples/c -lc -O ReleaseSafe -femit-bin=build/shobu-zig` |

On Windows executables have an `.exe` extension. For Zig facade tests:
`zig test examples/zig/main.zig examples/c/shobu.c -I examples/c -lc`.

## Library use

Python exposes `parse_sfen`, `sfen`, `play`, `legal`, `result`, and `Engine`.
JavaScript exports their counterparts (`parseSFEN` and `applyRaw` included).
Rust's public `State` offers parse/sfen/play/legal/result methods (embed the file
as a module to use it without its CLI entry point). C exposes `shobu.h` with
caller-owned output storage and return codes. C++ exposes `usp::Position` with
exceptions and value semantics. Zig exposes `Position` with error unions and
caller-owned buffers. Positions MUST come from the parser/initial factory and
remain valid; manually corrupting exposed arrays is outside the API contract.

`play` is atomic: success yields a new state, errors preserve the old state.
The Python/JS raw application helpers and Rust private raw helper are internal
geometry primitives: use `play` to enforce termination and the counter limit.
Raw helpers are not full public game adjudication APIs. Legal generation at the
counter limit returns empty, while result still evaluates game rules normally.

Input is bounded and invalid data is rejected. All examples operate serially.
Their legal-move order and `go` responses match, making cross-language validation
reproducible. No network, third-party package, or runtime download is needed to
run an example once its compiler/interpreter is installed.
