/*
 * cmd_commit_tree — 对应 `git commit-tree`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit commit-tree <tree-oid> [-p <parent-oid>]... -m <message>`
 *   <tree-oid>   这次提交要指向的 tree（必填，位置参数）
 *   -p           父提交 oid，可以出现 0 次（首次提交）、1 次（普通提
 *                交）或多次（未来做 merge 提交时会用到）
 *   -m           提交说明（必填）
 *
 * 步骤：
 *   1. minigit_cli_require_repo
 *   2. 解析出 tree oid、parents 数组、message（都是纯字符串/参数解析，
 *      没有 git 原理可言，正常写业务逻辑即可）
 *   3. 构造作者/提交者字符串。可以先写死一个简单值，比如：
 *        "minigit <minigit@example.com> <unix时间戳> +0000"
 *      时间戳用 time(NULL) 即可（<time.h>）。真实姓名/邮箱在真实 git 里
 *      是从 user.name / user.email 配置读的，本项目不需要做配置系统，
 *      硬编码或者读一个环境变量都可以，你自己决定。
 *   4. 组装 minigit_commit 结构体，minigit_commit_serialize 得到 content
 *      字节流。
 *   5. minigit_object_write(repo, MINIGIT_OBJ_COMMIT, content, content_size,
 *      &oid) —— 注意 commit.h 只负责"结构体 -> 字节流"，真正写盘、算哈希
 *      还是要经过 object.c 那一层统一的对象写入管线（这样 commit 对象
 *      和 blob/tree 对象共享同一套存储格式，是符合 git 设计的）。
 *   6. 打印 oid 的十六进制。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/commit.h"
#include "minigit/object.h"
#include "minigit/sha1.h"

int cmd_commit_tree(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
