/*
 * cmd_hash_object — 对应 `git hash-object`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit hash-object [-w] <file>`
 *   -w      除了计算哈希，还要把 blob 对象真正写入 .git/objects/
 *           （不带 -w 就只计算、打印哈希，不写盘——方便验证格式对不对
 *           而不污染对象库）
 *
 * 步骤：
 *   1. minigit_cli_require_repo 拿到 repo。
 *   2. minigit_read_file 读入 <file> 的原始内容。
 *   3. 根据是否有 -w：
 *        - 有：minigit_object_write(repo, MINIGIT_OBJ_BLOB, data, size, &oid)
 *        - 没有：minigit_object_hash(MINIGIT_OBJ_BLOB, data, size, &oid)
 *   4. minigit_oid_to_hex 打印 40 位十六进制到 stdout（换行）。
 *
 * 验证正确性的黄金标准：对同一个文件分别跑
 *   git hash-object <file>
 *   minigit hash-object <file>
 * 两边打印的哈希应该【完全一致】——如果不一致，说明 object.c 里 header
 * 拼接或者 store 内容不对。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/fsutil.h"
#include "minigit/object.h"
#include "minigit/sha1.h"

int cmd_hash_object(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
