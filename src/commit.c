#include "minigit/commit.h"

#include <stdlib.h>
#include <string.h>

/* TODO(你来实现)：见 commit.h 顶部的格式说明 */

int minigit_commit_serialize(const minigit_commit *commit, unsigned char **out_data,
                              size_t *out_size) {
    (void)commit;
    (void)out_data;
    (void)out_size;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_commit_parse(const unsigned char *data, size_t size, minigit_commit *out_commit) {
    (void)data;
    (void)size;
    (void)out_commit;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

void minigit_commit_free(minigit_commit *commit) {
    if (commit == NULL) {
        return;
    }
    free(commit->parents);
    free(commit->author);
    free(commit->committer);
    free(commit->message);
    memset(commit, 0, sizeof(*commit));
}
