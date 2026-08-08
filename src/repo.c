/*
 * repo.c — 仓库发现与路径拼接，已完整实现。
 */
#include "minigit/repo.h"
#include "minigit/fsutil.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

int minigit_repo_discover(const char *start_path, minigit_repo *out_repo) {
    if (start_path == NULL || out_repo == NULL) {
        return MINIGIT_ERR_INVALID;
    }

    char cur[PATH_MAX];
    if (realpath(start_path, cur) == NULL) {
        return MINIGIT_ERR_IO;
    }

    for (;;) {
        char *git_dir = minigit_path_join(cur, ".git");
        if (git_dir != NULL && minigit_path_is_dir(git_dir)) {
            out_repo->git_dir = git_dir;
            out_repo->work_tree = strdup(cur);
            if (out_repo->work_tree == NULL) {
                free(git_dir);
                return MINIGIT_ERR;
            }
            return MINIGIT_OK;
        }
        free(git_dir);

        if (strcmp(cur, "/") == 0) {
            return MINIGIT_ERR_NOT_A_REPO;
        }

        char *slash = strrchr(cur, '/');
        if (slash == NULL) {
            return MINIGIT_ERR_NOT_A_REPO;
        } else if (slash == cur) {
            cur[1] = '\0'; /* 到达根目录 "/" */
        } else {
            *slash = '\0';
        }
    }
}

void minigit_repo_free(minigit_repo *repo) {
    if (repo == NULL) {
        return;
    }
    free(repo->git_dir);
    free(repo->work_tree);
    repo->git_dir = NULL;
    repo->work_tree = NULL;
}

char *minigit_repo_git_path(const minigit_repo *repo, const char *relpath) {
    return minigit_path_join(repo->git_dir, relpath);
}

char *minigit_repo_work_path(const minigit_repo *repo, const char *relpath) {
    return minigit_path_join(repo->work_tree, relpath);
}
