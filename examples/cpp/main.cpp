// A C++17 client-facing facade over the shared, allocation-free C rules core.
// Compile this translation unit alone: it includes the core implementation.
#include "../c/shobu.c"
#include <stdexcept>
#include <string>
#include <vector>
namespace usp {
class Position {
    Shobu state_{};
public:
    explicit Position(const std::string& text = shobu_initial) {
        if (text.find('\0') != std::string::npos || !shobu_parse(text.c_str(), &state_)) throw std::invalid_argument("sfen");
    }
    std::string sfen() const {
        char text[USP_SFEN_SIZE]; shobu_sfen(&state_, text); return text;
    }
    Position play(const std::string& turn) const {
        if (turn.find('\0') != std::string::npos) throw std::invalid_argument("syntax");
        Position next = *this;
        int e = shobu_play(&state_, turn.c_str(), &next.state_);
        if(e) throw std::invalid_argument(e == 1 ? "syntax" : "illegal");
        return next;
    }
    std::vector<std::string> legal() const {
        ShobuMove moves[USP_MAX_MOVES]; int n = shobu_legal(&state_, moves, 1);
        std::vector<std::string> out;
        for(int i = 0; i < n; ++i) out.emplace_back(moves[i].text);
        return out;
    }
    char result() const { return shobu_result(&state_); }
};
}
#ifndef USP_CPP_NO_MAIN
int main() { return shobu_run(); }
#endif
