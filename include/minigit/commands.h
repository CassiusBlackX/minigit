/*
 * commands.h — CLI 子命令入口点。
 *
 * main.c（已完整实现，属于 chore）负责把 `minigit <subcommand> args...`
 * 分发到这里声明的 cmd_xxx 函数，调用约定和 main() 本身一致：
 * argv[0] 是子命令名本身（例如 "add"），argv[1..argc-1] 是这个子命令
 * 自己的参数。返回值是进程退出码（0 表示成功）。
 *
 * 每个 cmd_xxx 的实现放在 src/commands/<name>.c 里，TODO 由你完成；
 * 具体应该做什么、对应真实 git 的哪个命令、需要用到本项目哪些模块，
 * 写在各自 .c 文件顶部的注释里。
 */
#ifndef MINIGIT_COMMANDS_H
#define MINIGIT_COMMANDS_H

#include "minigit/repo.h"

#ifdef __cplusplus
extern "C" {
#endif

/* chore，已完整实现：定位当前仓库；找不到 .git 时会向 stderr 打印一条
 * 类似真实 git 的错误信息（"fatal: not a git repository ..."）并返回
 * MINIGIT_ERR_NOT_A_REPO。几乎每个子命令（除了 init）第一步都是调用
 * 这个函数，所以抽出来避免重复代码。用完记得 minigit_repo_free。 */
int minigit_cli_require_repo(minigit_repo *out_repo);

int cmd_init(int argc, char **argv);
int cmd_hash_object(int argc, char **argv);
int cmd_cat_file(int argc, char **argv);
int cmd_write_tree(int argc, char **argv);
int cmd_commit_tree(int argc, char **argv);
int cmd_add(int argc, char **argv);
int cmd_commit(int argc, char **argv);
int cmd_log(int argc, char **argv);
int cmd_status(int argc, char **argv);
int cmd_diff(int argc, char **argv);
int cmd_branch(int argc, char **argv);
int cmd_checkout(int argc, char **argv);
int cmd_merge(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_COMMANDS_H */
