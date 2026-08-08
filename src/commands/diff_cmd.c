/*
 * cmd_diff — 对应 `git diff`
 *
 * 文件名故意叫 diff_cmd.c 而不是 diff.c，是为了不和 src/diff.c（行级
 * diff 算法本体）撞名。
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit diff [<file>]`
 *   不带参数：对工作区里所有"已跟踪且被改动"的文件，依次打印 diff
 *             （对比【工作区当前内容】 vs 【index 里记录的 oid 对应的
 *             blob 内容】——也就是"还没 add 的改动"，这是 git diff 不
 *             带参数时的默认语义；--cached 对比 index vs HEAD 是加分
 *             项，可以不做）
 *   带 <file>：只对这一个文件做上面的对比
 *
 * 步骤（针对每个要比较的文件）：
 *   1. minigit_index_find 找到这个 path 在 index 里的 entry，没有就
 *      跳过（说明是 untracked，`git diff` 不管 untracked 文件）。
 *   2. minigit_object_read(repo, &entry->oid, &old_obj) 拿到"暂存版本"
 *      的内容（old_obj.data/old_obj.size）。
 *   3. minigit_read_file 读工作区当前内容作为"新版本"。
 *   4. minigit_diff_lines(old, old_len, new, new_len, &result)。
 *   5. 遍历 result.lines，按 op 打印：
 *        MINIGIT_DIFF_EQUAL -> 前缀空格 ' '（或者干脆不打印 EQUAL 行，
 *                               只打印发生变化的行，更接近真实 diff 的
 *                               "上下文行数"概念，但完整打印所有行更
 *                               简单，两种都可以接受）
 *        MINIGIT_DIFF_DEL   -> 前缀 '-'
 *        MINIGIT_DIFF_ADD   -> 前缀 '+'
 *      每行内容记得用 line/len 而不是当成 C 字符串处理（它没有 '\0'
 *      结尾），可以用 printf("%.*s\n", (int)line->len, line->line)。
 *   6. minigit_diff_free(&result)，minigit_object_free(&old_obj)。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/diff.h"
#include "minigit/fsutil.h"
#include "minigit/index.h"
#include "minigit/object.h"

int cmd_diff(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
