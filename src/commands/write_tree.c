/*
 * cmd_write_tree — 对应 `git write-tree`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit write-tree`（不接受参数）
 *
 * 步骤：
 *   1. minigit_cli_require_repo
 *   2. minigit_index_load 读出当前暂存区
 *   3. minigit_index_write_tree 折叠成 tree 对象树
 *   4. minigit_oid_to_hex 打印根 tree 的 oid
 *
 * 这个命令本身很短，核心工作量都在 index.c 的 write_tree 算法里。
 * 建议先跑通 test_index.cpp 里对 write_tree 的单元测试，再回来接这个
 * 命令，会更容易定位问题出在"命令层的参数/调用顺序"还是"算法本身"。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/index.h"
#include "minigit/sha1.h"

int cmd_write_tree(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
