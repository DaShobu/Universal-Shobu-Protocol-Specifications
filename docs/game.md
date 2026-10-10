# Universal Shōbu Protocol: game specification 1.0

Status: repository-defined standard, initial version. MUST, MUST NOT, SHOULD,
and MAY express requirements. This project is independent of the game publisher.
Shōbu is credited to [Manolis Vranas and Jamie Sajdak](https://rules.dized.com/game/VJQ5V5aAR4aqK31q1V1OXA/DD4rEBPOQSWyHEdN0nQ1Ew/credits). The Apache license covers
this repository's original text and code, not third-party rulebooks or trademarks.

## Sources and profile decisions

The publisher-verified [Dized rules](https://rules.dized.com/game/VJQ5V5aAR4aqK31q1V1OXA/shobu),
[setup](https://rules.dized.com/game/VJQ5V5aAR4aqK31q1V1OXA/eHKNib-cRR-J4AEiIJG0cA/setting-up-the-game),
[turn rules](https://rules.dized.com/game/VJQ5V5aAR4aqK31q1V1OXA/QxFjI3hVRLSgodBga_Ajtg/playing-the-game),
[passive rules](https://rules.dized.com/game/VJQ5V5aAR4aqK31q1V1OXA/iejagGBWRxKrvqOnY4lNDA/passive-move),
[aggressive rules](https://rules.dized.com/game/VJQ5V5aAR4aqK31q1V1OXA/B-R5-HjxQv6EWNWb3Gudbg/aggressive-move),
and [pushing rules](https://rules.dized.com/game/VJQ5V5aAR4aqK31q1V1OXA/T4xij74OT9y9_WCQ_9Fiyg/pushing-stones)
were consulted on 2026-10-10. This document formalizes them, without reproducing
their text. Coordinates, board IDs, serialization, and adjudication below are USP
conventions. The accessible publisher rules do not specify repetition or a
blocked-player outcome; USP explicitly chooses no automatic repetition draw and
a loss when no complete legal turn exists. Event organizers may adopt a different
profile, but MUST NOT advertise it as this unmodified rules profile.

## Board and coordinates

There are four independent 4 × 4 boards. View everything from Black's seat;
coordinates never rotate for White. Files a–d run left to right. Ranks 1–4 run
from Black toward White. Index `(rank-1)*4 + (file-'a')` is used in the examples.

```
                    White
          board 2 dark | board 3 light
          ------------ rope ---------
          board 0 dark | board 1 light
                    Black
```

Black's homeboards are 0 and 1; White's are 2 and 3. Board color is `id mod 2`.
IDs are logical: a physical layout MAY be mirrored if these logical identities
are preserved. Each square contains empty, Black (`B`), or White (`W`). Initially
every board has Black at a1,b1,c1,d1 and White at a4,b4,c4,d4. Black plays first.
Each side has at most four stones on each board. Stones never change boards,
return after capture, share a square, or acquire individual identity.

## State and turn

A committed state is `(boards[4][16], side, ply)`, where side is `b` or `w` and
ply is the number of completed turns, starting at zero. The counter is limited
to 0..2147483647 for interoperability. A terminal state can be serialized.
Intermediate passive states are private UI/engine working data, not SFEN states.

A turn has a passive move then an aggressive move. Each leg specifies a board,
source, and destination. Let the passive displacement be `(dx,dy)`. It MUST be
`n*(ux,uy)`, with n in {1,2}, ux,uy in {-1,0,1}, and at least one nonzero unit
component. Thus movement is horizontal, vertical, or diagonal; no null move.
The aggressive leg MUST have exactly the same signed displacement in global
coordinates and MUST use a board of the opposite color. Its board may be on
either player's side. Both sources MUST contain the player to move.

The passive board MUST be a homeboard of the player to move. Every visited square,
including its destination, MUST be empty; no jumping or pushing is permitted.
An aggressive move need not push, but MUST NOT push a friendly stone, jump any
stone, or push more than one opponent stone during the entire leg.

### Precise aggressive transition

Execute n unit steps on a temporary board. At each step:

1. The moving stone's next square MUST be on-board.
2. If empty, advance the moving stone.
3. If friendly, reject the entire turn.
4. If opposing, examine the next square in the same unit direction. If on-board
   and occupied by either color, reject. Otherwise shift the opposing stone there,
   or remove it if off-board, and advance the moving stone into its old square.

The same opponent stone can be pushed on both steps. Track its current square;
contact with a different opponent later in the leg MUST be rejected, even if
the first was removed. The moving stone itself MUST end on-board. For example,
a two-step mover at a2 meeting W at b2 pushes it to d2; meeting W at c2 instead
pushes it to d2 only. A mover at b2 with W at c2 pushes W off the right edge.
Two opponents at b2 and c2 block a mover at a2.

Both legs MUST succeed before committing any mutation, incrementing ply, or
changing side. A passive leg with no compatible aggressive completion is invalid.
Rejections MUST leave state unchanged. Turns from terminal states MUST be rejected.
At maximum ply, further turns are rejected as a representation limit, not a loss.

## Termination and validity

After a complete turn, if any board has zero opponent stones, the mover wins
immediately. The next side is still recorded in SFEN. There is no answering turn.
Otherwise if the next side has no complete legal turn, that side loses. A side
may resign externally; agreements, clocks, and adjudication are session metadata,
not reconstructed from SFEN. USP specifies no automatic draw for repetition or
turn count. Clients MAY negotiate an external draw policy separately.

An SFEN parser checks structural validity, not reachability from the initial
position. Every board MUST have at most four stones of either color. Positions
where both players have an empty board are rejected (contradictory winners).
An empty board for one player is a terminal loss for that player, irrespective
of side. Arbitrary study positions and counters are accepted; historical parity
and reachability are not asserted. Result precedence: empty-board loss, blocked
side loss, otherwise ongoing. The maximum counter does not change this result.

## Complete-turn notation and ordering

`0a1a2:1b1b2` means passive board 0, a1→a2; aggressive board 1, b1→b2.
Grammar: `board square square ":" board square square`, board is one digit 0–3,
square is `[a-d][1-4]`. Exactly 11 ASCII bytes; no whitespace or alternatives.
All coordinates are absolute, including White's moves. A legal list MUST contain
each complete turn once, sorted by ASCII lexicographic order. A generator may
enumerate a passive source, direction and distance, then each compatible
aggressive board/source, validate the pair, and sort the resulting encodings.
