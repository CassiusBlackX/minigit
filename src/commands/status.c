/*
 * cmd_status — 对应 `git status`
 *
 * ============================================================================
 * TODO(你来实现) —— 本项目里逻辑最绕的 porcelain 命令，建议留到最后做
 * ============================================================================
 * `git status` 本质是在同时比较【三份文件清单】：
 *
 *     HEAD 指向的 tree  <---A--->  index（暂存区）  <---B--->  工作区
 *
 *   A（index 相对 HEAD 的差异）= "已暂存，将要被提交的改动"
 *       - 在 index 里但不在 HEAD tree 里（或者内容 oid 不同）-> 新增/修改，
 *         会显示成 "Changes to be committed: new file / modified"
 *       - 在 HEAD tree 里但不在 index 里 -> 被删除，会显示 "deleted"
 *
 *   B（工作区相对 index 的差异）= "已暂存但还没提交 vs 工作区当前内容
 *     是否又被改了"，也就是"改了但没 add"
 *       - index 里有记录，但工作区文件内容的哈希和 index 里存的 oid 不
 *         一致 -> "Changes not staged for commit: modified"
 *       - index 里有记录，但工作区文件已经不存在 -> "deleted"
 *       - 工作区有文件，但 index 里完全没有这条路径 -> "Untracked files"
 *
 * 建议的实现步骤：
 *   1. minigit_cli_require_repo，minigit_index_load。
 *   2. 尝试 minigit_ref_resolve(repo, "HEAD", ...) 拿到 HEAD commit，
 *      如果存在就 minigit_object_read + minigit_commit_parse 拿到它的
 *      tree oid，再 minigit_object_read + minigit_tree_parse 展开成
 *      "path -> oid" 的清单。注意 tree 是嵌套的，如果你想要一份"扁平
 *      的 path -> oid 清单"用来跟 index 比较，需要自己写一个递归函数
 *      把嵌套 tree 展开（拼接父目录名 + '/' + entry name 作为完整
 *      path，递归进入 MINIGIT_MODE_TREE 类型的 entry）。
 *      如果 HEAD 还没有任何提交（ERR_NOT_FOUND），HEAD 清单视为空。
 *   3. 用 minigit_walk_files(repo->work_tree, ...) 遍历工作区，对每个
 *      文件现算一次 blob 哈希（minigit_object_hash，不需要真的写盘）
 *      和 index 里记录的 oid 比较。
 *   4. 汇总三份清单的差异，按上面的四个分类打印（格式不强制和真实 git
 *      完全一致，但建议分成"已暂存"/"未暂存"/"未跟踪"三组，方便自己
 *      和测试判断逻辑是否正确）。
 *
 * 建议：这个命令依赖的"展开 tree 成扁平清单"逻辑，将来 checkout 也会
 * 用到，可以考虑抽成一个内部 static helper 复用。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/commit.h"
#include "minigit/fsutil.h"
#include "minigit/index.h"
#include "minigit/object.h"
#include "minigit/refs.h"
#include "minigit/sha1.h"
#include "minigit/tree.h"

int cmd_status(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
