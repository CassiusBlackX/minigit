/*
 * main.c — CLI 入口与子命令分发表（chore，已完整实现）。
 *
 * `minigit <subcommand> [args...]` -> 在下面的表里按名字找到对应的
 * cmd_xxx 函数，把 argv 从 subcommand 本身开始（不含 "minigit"）传给
 * 它，这样每个 cmd_xxx 内部处理参数时可以像独立的小程序一样看待自己的
 * argv[0]（子命令名，可用于打印用法提示）。
 */
#include "minigit/commands.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const char *name;
    int (*handler)(int argc, char **argv);
    const char *summary;
} minigit_command_entry;

static const minigit_command_entry COMMANDS[] = {
    {"init", cmd_init, "创建一个新的 minigit 仓库"},
    {"hash-object", cmd_hash_object, "计算（并可选写入）一个文件的 blob 对象"},
    {"cat-file", cmd_cat_file, "查看某个对象的类型/大小/内容"},
    {"write-tree", cmd_write_tree, "把当前暂存区写成一个 tree 对象"},
    {"commit-tree", cmd_commit_tree, "由 tree + parent + message 直接创建一个 commit 对象"},
    {"add", cmd_add, "把文件加入暂存区"},
    {"commit", cmd_commit, "把暂存区的内容提交为一个新 commit"},
    {"log", cmd_log, "查看提交历史"},
    {"status", cmd_status, "查看工作区/暂存区状态"},
    {"diff", cmd_diff, "查看改动内容"},
    {"branch", cmd_branch, "创建/列出分支"},
    {"checkout", cmd_checkout, "切换分支或恢复文件"},
    {"merge", cmd_merge, "合并分支"},
};

static void print_usage(const char *prog) {
    fprintf(stderr, "usage: %s <command> [<args>]\n\ncommands:\n", prog);
    for (size_t i = 0; i < sizeof(COMMANDS) / sizeof(COMMANDS[0]); i++) {
        fprintf(stderr, "  %-14s %s\n", COMMANDS[i].name, COMMANDS[i].summary);
    }
}

int main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argc > 0 ? argv[0] : "minigit");
        return argc < 2 ? 1 : 0;
    }

    const char *cmd_name = argv[1];
    for (size_t i = 0; i < sizeof(COMMANDS) / sizeof(COMMANDS[0]); i++) {
        if (strcmp(COMMANDS[i].name, cmd_name) == 0) {
            /* 把 argv 向前挪一格：子命令处理函数看到的 argv[0] 是子命令名本身 */
            return COMMANDS[i].handler(argc - 1, argv + 1);
        }
    }

    fprintf(stderr, "minigit: '%s' 不是一个已知的命令。\n\n", cmd_name);
    print_usage(argv[0]);
    return 1;
}
