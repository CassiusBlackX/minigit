/*
 * cmd_cat_file — 对应 `git cat-file`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit cat-file (-t|-s|-p) <oid>`
 *   -t      只打印对象类型（"blob"/"tree"/"commit"）
 *   -s      只打印对象内容大小（字节数）
 *   -p      按类型美观打印内容：
 *             blob   -> 原样把 content 当字节流输出（可能是文本，也
 *                       可能不是，直接 fwrite 到 stdout 即可，不用假设
 *                       它是以 '\0' 结尾的字符串）
 *             tree   -> 用 minigit_tree_parse 解析后，每行打印
 *                       "<mode> <type> <oid的hex>\t<name>"
 *                       （<type> 根据 mode 是 MINIGIT_MODE_TREE 判断
 *                       打 "tree" 还是 "blob"）
 *             commit -> 用 minigit_commit_parse 解析后，按 commit.h
 *                       里描述的文本格式重新拼出来打印（或者更省事：
 *                       object.c 里 content 本身就是这个格式，直接原样
 *                       输出也完全可以）
 *
 * 步骤：
 *   1. minigit_cli_require_repo
 *   2. minigit_oid_from_hex 把参数字符串转成 oid
 *   3. minigit_object_read
 *   4. 按 flag 打印，最后 minigit_object_free
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/commit.h"
#include "minigit/object.h"
#include "minigit/sha1.h"
#include "minigit/tree.h"

int cmd_cat_file(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
