# Shōbu Forsyth–Edwards Notation (SFEN) 1.0

This repository defines Shōbu SFEN; it is not shogi SFEN and is not a claim of an
existing industry standard. Protocol negotiation selects this exact version.

```
sfen     = board "|" board "|" board "|" board " " side " " ply
board    = rank "/" rank "/" rank "/" rank
rank     = one or more of "B", "W", "1", "2", "3", "4"
side     = "b" / "w"
ply      = "0" / nonzero-digit *digit
```

Boards appear in ID order 0,1,2,3. Each board's ranks appear 4,3,2,1, with files
a,b,c,d in each rank. B and W encode one stone each. A digit encodes that many
consecutive empty squares. Expanded ranks MUST contain exactly four squares.
Adjacent digits MUST be rejected: each maximal empty run has one digit. No
leading/trailing space, extra fields, tabs, newline, Unicode, sign, or leading
counter zeros is allowed. Ply is 0..2147483647. Piece counts and contradictory
winners MUST be checked as specified in [game.md](game.md). Parsing a structurally
valid terminal/study position is permitted; legal reachability is not required.

The serializer MUST produce this canonical representation. Parsing then
serializing an accepted string MUST reproduce it byte for byte. Captured stones
need no field: per-board counts determine what is missing. No hands, castling,
en passant, partial-turn phase, clocks, history, or negotiated outcomes are encoded.
SFEN alone cannot establish a repetition draw or external resignation.

Initial state:

```
WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB b 0
```

After `0a1a2:1b1b2`:

```
WWWW/4/B3/1BBB|WWWW/4/1B2/B1BB|WWWW/4/4/BBBB|WWWW/4/4/BBBB w 1
```

Invalid examples: `WWWW/4/4/BBBB ... b 00` (counter), `22/4/4/B3`
(adjacent empty runs), `BBBB/B3/4/W3` (five Black stones on a board),
or a single board missing from the four-board list. Consumers MUST reject invalid
data before changing their current position. A byte limit of 4096 per protocol
line bounds work; canonical SFEN is much smaller.
