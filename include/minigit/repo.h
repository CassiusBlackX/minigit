/*
 * repo.h — 仓库句柄：定位 .git 目录、拼接仓库内部路径。
 *
 * 真实 git 的任何命令，第一步都是"仓库发现"：从当前目录开始，一层一层
 * 往上找，直到找到一个包含 .git 的目录为止（这就是为什么你在项目里的
 * 任何子目录下执行 git 命令都能生效）。找到之后，`.git` 所在目录就是
 * "工作区根目录 (work tree)"，`.git` 本身就是"git 目录 (git dir)"。
 *
 * minigit_repo_discover 这部分是纯粹的路径遍历逻辑，价值主要在工程上
 * 而不是 git 原理上，已经帮你实现好。真正体现"git 内部结构"的是
 * minigit_repo_init ——它要创建哪些文件/目录，这个留给你在
 * src/commands/init.c 里实现。
 */
#ifndef MINIGIT_REPO_H
#define MINIGIT_REPO_H

#include "minigit/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *git_dir;    /* 例如 "/home/you/project/.git"，无结尾斜杠 */
    char *work_tree;  /* 例如 "/home/you/project"，无结尾斜杠 */
} minigit_repo;

/* 从 start_path（通常传 "."）开始向上查找 .git 目录。
 * 成功：MINIGIT_OK，repo->git_dir / repo->work_tree 被填充（需要调用
 *       minigit_repo_free 释放）。
 * 失败：一路找到文件系统根都没找到 .git，返回 MINIGIT_ERR_NOT_A_REPO。
 *
 * 已完整实现。 */
int minigit_repo_discover(const char *start_path, minigit_repo *out_repo);

/* 释放 minigit_repo_discover 分配的内存 */
void minigit_repo_free(minigit_repo *repo);

/* 拼接 "<git_dir>/<relpath>"，例如 relpath="refs/heads/main"。
 * 返回新分配的字符串，调用方 free()。已完整实现。 */
char *minigit_repo_git_path(const minigit_repo *repo, const char *relpath);

/* 拼接 "<work_tree>/<relpath>"。已完整实现。 */
char *minigit_repo_work_path(const minigit_repo *repo, const char *relpath);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_REPO_H */
