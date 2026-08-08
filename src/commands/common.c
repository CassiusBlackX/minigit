/*
 * common.c — 子命令共享的小工具（chore，已完整实现）。
 */
#include "minigit/commands.h"

#include <stdio.h>

int minigit_cli_require_repo(minigit_repo *out_repo) {
    int rc = minigit_repo_discover(".", out_repo);
    if (rc == MINIGIT_ERR_NOT_A_REPO) {
        fprintf(stderr, "fatal: not a minigit repository (or any of the parent directories): .git\n");
    } else if (rc != MINIGIT_OK) {
        fprintf(stderr, "fatal: failed to access repository (error %d)\n", rc);
    }
    return rc;
}
