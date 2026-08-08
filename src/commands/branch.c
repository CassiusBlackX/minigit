/*
 * cmd_branch — 对应 `git branch`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：
 *   `minigit branch`            列出所有分支，当前分支前面加个 "* " 标记
 *   `minigit branch <name>`     基于当前 HEAD 指向的 commit 新建一个分支
 *                                （不切换过去，只是新建——切换是 checkout
 *                                的事）
 *
 * 列出分支：
 *   1. minigit_ref_list_branches(repo, &names, &count)
 *   2. minigit_ref_current_branch(repo, &current) 拿到当前分支名（用来
 *      判断该给哪一行加 "* " 前缀；如果处于 detached HEAD，current 会
 *      返回错误，这种情况可以约定"没有任何分支加星号"）
 *   3. 遍历 names 打印，记得最后把 names 数组和 current 都释放掉。
 *
 * 新建分支：
 *   1. minigit_ref_resolve(repo, "HEAD", &oid) 拿到当前 commit
 *      （如果还没有任何提交就会返回 MINIGIT_ERR_NOT_FOUND，这种情况下
 *      "基于当前 commit 建分支"这件事没有意义，可以直接报错提示用户
 *      "还没有提交，无法创建分支"）
 *   2. minigit_ref_create_branch(repo, name, &oid)
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/refs.h"
#include "minigit/sha1.h"

int cmd_branch(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
