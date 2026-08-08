#include "minigit/diff.h"

#include <stdlib.h>
#include <string.h>

/* TODO(你来实现)：见 diff.h 顶部的 LCS 算法说明 */

int minigit_diff_lines(const char *a, size_t a_len, const char *b, size_t b_len,
                        minigit_diff_result *out) {
    (void)a;
    (void)a_len;
    (void)b;
    (void)b_len;
    (void)out;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

void minigit_diff_free(minigit_diff_result *out) {
    if (out == NULL) {
        return;
    }
    free(out->lines);
    out->lines = NULL;
    out->count = 0;
}
