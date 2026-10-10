#ifndef SHOBU_H
#define SHOBU_H
/* USP 1.0 shared C99 API. Also valid C++17. No dynamic allocation. */
#include <stdint.h>
#define USP_MAX_PLY INT32_MAX
#define USP_MAX_MOVES 1024
#define USP_SFEN_SIZE 128
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { char board[4][16]; char side; int32_t ply; } Shobu;
typedef struct { char text[12]; } ShobuMove;
extern const char *const shobu_initial;
int shobu_parse(const char *text, Shobu *out);
void shobu_sfen(const Shobu *s, char out[USP_SFEN_SIZE]);
/* play: 0 success, 1 bad syntax, 2 illegal; never modifies input. */
int shobu_play(const Shobu *s, const char *move, Shobu *out);
int shobu_legal(const Shobu *s, ShobuMove out[USP_MAX_MOVES], int enforce_limit);
/* result: 'b', 'w', or '-' for ongoing. */
char shobu_result(const Shobu *s);
int shobu_run(void);
#ifdef __cplusplus
}
#endif
#endif
