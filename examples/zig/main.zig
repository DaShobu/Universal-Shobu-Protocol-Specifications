// Zig 0.15.2 facade over the same C rules core used by the C++ example.
// zig build-exe examples/zig/main.zig examples/c/shobu.c -I examples/c -lc
const std = @import("std");
const c = @cImport({ @cInclude("shobu.h"); });

pub const Position = struct {
    state: c.Shobu,

    pub fn parse(text: [:0]const u8) error{InvalidSfen}!Position {
        if (std.mem.indexOfScalar(u8, text, 0) != null) return error.InvalidSfen;
        var out: Position = undefined;
        if (c.shobu_parse(text.ptr, &out.state) == 0) return error.InvalidSfen;
        return out;
    }

    pub fn initial() Position {
        var out: Position = undefined;
        std.debug.assert(c.shobu_parse(c.shobu_initial, &out.state) != 0);
        return out;
    }

    // Caller owns storage; the returned slice is valid while storage is alive.
    pub fn sfen(self: *const Position, storage: *[c.USP_SFEN_SIZE]u8) []const u8 {
        c.shobu_sfen(&self.state, storage);
        const end = std.mem.indexOfScalar(u8, storage, 0).?;
        return storage[0..end];
    }

    pub fn play(self: *const Position, turn: [:0]const u8) error{InvalidSyntax, IllegalTurn}!Position {
        if (std.mem.indexOfScalar(u8, turn, 0) != null) return error.InvalidSyntax;
        var out: Position = undefined;
        switch (c.shobu_play(&self.state, turn.ptr, &out.state)) {
            0 => return out,
            1 => return error.InvalidSyntax,
            else => return error.IllegalTurn,
        }
    }

    pub fn legal(self: *const Position, storage: *[c.USP_MAX_MOVES]c.ShobuMove) []const c.ShobuMove {
        const count: usize = @intCast(c.shobu_legal(&self.state, storage, 1));
        return storage[0..count];
    }

    pub fn result(self: *const Position) u8 {
        return @intCast(c.shobu_result(&self.state));
    }
};

pub fn main() void {
    if (c.shobu_run() != 0) std.process.exit(1);
}

test "Zig facade parses, applies, serializes and generates" {
    const start = Position.initial();
    const next = try start.play("0a1a2:1b1b2");
    var text: [c.USP_SFEN_SIZE]u8 = undefined;
    try std.testing.expectEqualStrings(
        "WWWW/4/B3/1BBB|WWWW/4/1B2/B1BB|WWWW/4/4/BBBB|WWWW/4/4/BBBB w 1",
        next.sfen(&text),
    );
    const parsed = try Position.parse("WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB|WWWW/4/4/BBBB b 0");
    var moves: [c.USP_MAX_MOVES]c.ShobuMove = undefined;
    try std.testing.expect(parsed.legal(&moves).len > 0);
    try std.testing.expectEqual(@as(u8, '-'), parsed.result());
    try std.testing.expectError(error.InvalidSfen, Position.parse("bad"));
    try std.testing.expectError(error.IllegalTurn, start.play("2a1a2:3a1a2"));
}
