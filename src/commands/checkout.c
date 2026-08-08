/*
 * cmd_checkout — 对应 `git checkout <branch>`（只要求支持切换分支，不
 * 要求支持 `git checkout -- <file>` 恢复单个文件，那是另一个加分项）
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit checkout <branch-name>`
 *
 * 核心是"把目标分支指向的 tree 展开写回工作区"，步骤：
 *
 *   1. minigit_cli_require_repo
 *   2. minigit_ref_resolve(repo, "refs/heads/<branch-name>", &commit_oid)
 *      —— 分支不存在则报错返回（真实 git 这里还会判断参数是不是一个
 *      commit 哈希从而进入 detached HEAD 模式，本项目不要求支持）。
 *   3. minigit_object_read 该 commit，拿到它的 tree oid。
 *   4. 【递归展开 tree】：写一个 static helper，类似
 *        static int checkout_tree(const minigit_repo *repo,
 *                                  const minigit_oid *tree_oid,
 *                                  const char *prefix)
 *      对 tree 里每个 entry：
 *        - 是 blob（MINIGIT_MODE_BLOB / MINIGIT_MODE_EXEC）：
 *          minigit_object_read 拿到内容，用 minigit_repo_work_path 拼出
 *          "prefix/name" 的绝对路径，minigit_write_file 写入（它会自动
 *          创建父目录）。如果 mode 是 EXEC，写完之后可以用 chmod() 加上
 *          可执行位（<sys/stat.h>），非必需但比较贴近真实行为。
 *        - 是 tree（MINIGIT_MODE_TREE）：递归调用自己，prefix 换成
 *          "prefix/name"。
 *      顶层调用时 prefix 传空字符串 ""。
 *
 *   5. 【已知的简化，建议先接受，不要在这上面卡太久】：
 *      本步骤只管"把目标 tree 里有的文件写出来"，不处理"旧分支有但新
 *      分支没有的文件应该被删除"这件事（真实 git checkout 会做这个清
 *      理）。如果你想做得更完整，可以：加载旧的 index，对比新 tree 的
 *      扁平清单，把"只在旧 index、不在新 tree"里的路径 remove() 掉
 *      （标准库 <stdio.h> 的 remove，不是 minigit_index_remove）。这个
 *      属于加分项。
 *
 *   6. 用新 tree 重建 index：你可以复用第 4 步递归展开时顺便收集到的
 *      "path -> mode/oid" 列表，逐条 minigit_index_add，最后
 *      minigit_index_save——checkout 之后 index 应该和新 HEAD 完全一致
 *      （工作区、index、HEAD 三者一致，就是 `git status` 应该打印
 *      "nothing to commit, working tree clean" 的状态）。
 *
 *   7. minigit_ref_set_head_symbolic(repo, branch_name) 把 HEAD 切过去。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/commit.h"
#include "minigit/fsutil.h"
#include "minigit/index.h"
#include "minigit/object.h"
#include "minigit/refs.h"
#include "minigit/tree.h"

int cmd_checkout(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
