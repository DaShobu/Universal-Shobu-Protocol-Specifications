# Universal Shōbu Protocol (USP) 1.0

USP 1.0 is a synchronous, deterministic, ASCII, newline-delimited stdin/stdout
protocol. It connects one controlling client to one engine; the same framing may
be transported over a reliable byte stream. No shell evaluation is involved.
Each request has exactly one response line. LF and CRLF input are accepted; output
uses LF (host text streams may produce CRLF, which receivers MUST accept).
Flush each response. Diagnostics belong on stderr, never stdout.

A line contains at most 4096 bytes before its terminator. EOF ends the session
without a response and an unterminated final line MUST NOT be executed. An
oversized line is drained and answered `error syntax`; processing may resume.
Only printable ASCII (0x20..0x7e) is accepted. Leading/trailing spaces, duplicate
spaces, tabs, blank lines, embedded CR/NUL and unknown commands are errors.
No unsolicited output is allowed. Commands are case-sensitive.

## Lifecycle and commands

The engine starts unnegotiated with an initial position. The first command MUST
be `hello 1`; reply `ok usp 1`. Any other command (except quit) before negotiation
returns `error handshake`. Repeating `hello 1` is permitted and preserves state;
any other `hello` version returns `error version`. Version 1 selects game 1.0 and
SFEN 1.0. There are no optional commands in this baseline.

| Request | Response / effect |
| --- | --- |
| `hello 1` | `ok usp 1` |
| `ready` | `ok ready` |
| `new` | Reset to initial state; `ok new` |
| `position startpos` | Reset to initial state; `ok position` |
| `position <SFEN>` | Validate and replace committed state; `ok position` |
| `play <turn>` | Validate and commit both legs; `ok play` |
| `state` | `state <canonical SFEN>` |
| `legal` | `legal <turn> ...` sorted; just `legal` if none |
| `result` | `result ongoing`, `result b`, or `result w` |
| `go` | `bestmove <first legal turn>`; `bestmove none` if terminal or ply exhausted |
| `quit` | `ok bye`, flush and exit successfully |

`go` is a reproducible reference selection policy, not a strength requirement:
production engines MAY choose any legal turn. It MUST NOT change the position.
The baseline has no asynchronous search, `stop`, timing, options, or ponder mode.
Commands are processed serially; clients MUST wait for a response before sending
the next. Timeouts, process crashes, or disconnected streams are transport errors
handled by the client. On stdout failure an engine should exit; no recovery
response can be guaranteed.

## Errors, limits, and atomicity

* `error syntax`: bad framing, characters, command, arity, or move syntax.
* `error handshake`: valid framing but handshake required.
* `error version`: unsupported/malformed hello version after valid framing.
* `error sfen`: invalid `position` payload (including empty payload).
* `error illegal`: well-formed but illegal turn, including play after termination
  or at the counter limit.

Precedence is framing, hello/quit handling, handshake, command syntax, payload
validation. Unknown commands after handshake return syntax. A bad move encoding
returns syntax; a correctly encoded illegal move returns illegal. SFEN errors
always return sfen. Every error preserves both position and negotiation status.
Implementations MUST use bounded input and bounded output: at most 1024 complete
turns are possible before filtering, since each of eight passive sources has
16 directions/distances and at most eight aggressive sources. A `legal` response
can exceed the request limit and clients MUST allow at least 12293 bytes before
the terminator. The examples cap requests, not responses.

## Example session

```
> hello 1
< ok usp 1
> ready
< ok ready
> state
< state WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB b 0
> play 0a1a2:1b1b2
< ok play
> state
< state WWWW/4/B3/1BBB|WWWW/4/1B2/B1BB|WWWW/4/4/BBBB|WWWW/4/4/BBBB w 1
> play 0a2a3:1b2b3
< error illegal
> result
< result ongoing
> new
< ok new
> go
< bestmove 0a1a2:1a1a2
> quit
< ok bye
```

SFEN replacement supports replay by loading a base position and issuing complete
`play` requests in order. A client records responses, final SFEN, and external
match metadata separately. This baseline does not define a saved-game container.
