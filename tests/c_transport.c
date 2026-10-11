/* Inject stream failures to distinguish transport errors from clean EOF/quit. */
#include <stdio.h>
#include <assert.h>
static const char *input;
static int input_failed, output_failed, write_failed;
static int test_getchar(void) {
    return *input ? (unsigned char)*input++ : EOF;
}
static int test_fflush(FILE *stream) {
    (void)stream;
    return output_failed ? EOF : 0;
}
static int test_ferror(FILE *stream) {
    return stream==stdin ? input_failed : write_failed;
}
#define getchar test_getchar
#define fflush test_fflush
#define ferror test_ferror
#include "../examples/c/shobu.c"
int main(void) {
    input="quit\n"; output_failed=1;
    assert(shobu_run()==1);
    input="hello 1\n";
    assert(shobu_run()==1);
    output_failed=0; write_failed=1; input="quit\n";
    assert(shobu_run()==1);
    input="hello 1\n";
    assert(shobu_run()==1);
    write_failed=0; input=""; input_failed=1;
    assert(shobu_run()==1);
    input="hello 1";
    assert(shobu_run()==1);
    input_failed=0; input="hello 1";
    assert(shobu_run()==0);
    input="quit\n";
    assert(shobu_run()==0);
}
