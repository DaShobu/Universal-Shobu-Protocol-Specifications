# Validation

Run from the repository root:

```text
python scripts/build.py --require-all
python -m unittest discover -s tests -p test_rules.py -v
node tests/javascript_api.mjs
python tests/conformance.py --require-all
```

`vectors.json` is a language-neutral, hand-written golden corpus. It specifies
opening moves, early/late two-step contact, edge and diagonal captures, White's
absolute coordinates, illegal board/direction/distance choices, blocked pushes,
terminal moves, representation limits, and malformed SFEN. Its SFEN positions
are structural study positions, not assertions of reachability.

The Python API tests check these independently recorded outcomes, a blocked
player, repeated positions without an automatic draw, canonical round trips,
sorted/unique moves, the 232-move opening, and stone conservation over seeded games.

The black-box runner sends the same transcript to every executable. Golden
responses are asserted first against the Python implementation. Complete legal
lists and state transitions from seeded games and arbitrary studies are then
compared with that reference. Cross-language agreement alone is not proof of
rules correctness, which is why the independent golden vectors are included.
Framing checks cover mixed LF/CRLF, NUL, non-ASCII, malformed handshakes, overlong
requests, recovery after errors, exact size boundaries, quit, and incomplete EOF.
Each engine has a 120-second process timeout. C/C++, JavaScript, and Zig also
have native API tests, including output preservation and embedded-NUL rejection.

To test your own engine against USP 1.0:

```text
python tests/conformance.py --engine /path/to/engine
python tests/conformance.py --engine python path/to/your_engine.py
```

These commands require no third-party Python packages. Build artifacts and local
toolchains go in ignored `build/`; no compiler downloads occur in the build script.
