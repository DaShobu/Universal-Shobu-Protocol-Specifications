#define USP_CPP_NO_MAIN
#include "../examples/cpp/main.cpp"
#include <cassert>
#include <cstring>
int main() {
    const usp::Position start;
    assert(start.legal().size() == 232);
    assert(start.result() == '-');
    const auto next = start.play("0a1a2:1b1b2");
    assert(next.sfen() == "WWWW/4/B3/1BBB|WWWW/4/1B2/B1BB|WWWW/4/4/BBBB|WWWW/4/4/BBBB w 1");
    assert(start.sfen() == shobu_initial);
    bool caught = false;
    try { start.play("2a1a2:3a1a2"); } catch (const std::invalid_argument&) { caught = true; }
    assert(caught);
    caught = false;
    try { usp::Position bad(std::string(shobu_initial) + std::string("\0junk", 5)); }
    catch (const std::invalid_argument&) { caught = true; }
    assert(caught);
    caught = false;
    try { start.play(std::string("0a1a2:1a1a2\0junk", 16)); }
    catch (const std::invalid_argument&) { caught = true; }
    assert(caught);
    // The C ABI preserves output buffers on a parse/play rejection.
    Shobu state; assert(shobu_parse(shobu_initial, &state));
    const Shobu original = state;
    assert(!shobu_parse("bad", &state));
    assert(std::memcmp(&state, &original, sizeof state) == 0);
    assert(shobu_play(&state, "2a1a2:3a1a2", &state) == 2);
    assert(std::memcmp(&state, &original, sizeof state) == 0);
    // In-place success is supported at the C ABI boundary.
    assert(shobu_play(&state, "0a1a2:1b1b2", &state) == 0);
    char text[USP_SFEN_SIZE]; shobu_sfen(&state, text);
    assert(next.sfen() == text);
}
