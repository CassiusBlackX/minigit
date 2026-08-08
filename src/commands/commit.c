/*
 * cmd_commit — 对应 `git commit`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit commit -m <message>`
 *
 * 这个命令是把你已经写好的几块拼在一起：index -> write-tree -> 找到
 * 当前 parent -> commit-tree -> 更新分支引用。步骤：
 *
 *   1. minigit_cli_require_repo
 *   2. minigit_index_load，minigit_index_write_tree 得到 tree oid
 *      （和 cmd_write_tree 做的事完全一样，可以直接复用同样的两行代码）
 *   3. 找 parent：
 *        - minigit_ref_resolve(repo, "HEAD", &parent_oid)
 *        - 如果返回 MINIGIT_ERR_NOT_FOUND：说明这是【第一次提交】
 *          （refs/heads/main 还不存在），parents 数组为空
 *          （parent_count = 0）——注意这不是错误，是正常情况，不要把
 *          这个 error code 当成失败直接 return。
 *        - 否则：parents 数组只有一个元素 parent_oid
 *   4. 组装 minigit_commit（author/committer 时间戳同 cmd_commit_tree
 *      的做法），minigit_commit_serialize，
 *      minigit_object_write(..., MINIGIT_OBJ_COMMIT, ...) 得到新 commit
 *      的 oid。
 *   5. 更新分支指向：
 *        - minigit_ref_current_branch(repo, &branch_name) 拿到当前分支名
 *          （刚 init 完还没有第一次提交时，HEAD 已经是
 *          "ref: refs/heads/main"，所以这一步始终能拿到 "main"，不需要
 *          特判第一次提交）
 *        - minigit_ref_update(repo, "refs/heads/<branch_name>", &new_oid)
 *          （用 snprintf 拼出 "refs/heads/xxx" 这个 ref_name 字符串）
 *   6. 打印一行提交成功的信息（比如分支名 + 新 commit 的短哈希 + 提交
 *      说明第一行），格式不强制，测试只关心磁盘状态和 log 的输出。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/commit.h"
#include "minigit/index.h"
#include "minigit/object.h"
#include "minigit/refs.h"
#include "minigit/sha1.h"

int cmd_commit(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
