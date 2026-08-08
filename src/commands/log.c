/*
 * cmd_log — 对应 `git log`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit log`（不需要支持参数）
 *
 * 步骤：
 *   1. minigit_cli_require_repo
 *   2. minigit_ref_resolve(repo, "HEAD", &oid) 得到起点 —— 注意
 *      ref_resolve 已经帮你处理了 "HEAD -> refs/heads/main -> 具体
 *      commit" 这条间接链，这里不需要再手动跳一次。
 *      如果返回 MINIGIT_ERR_NOT_FOUND，说明还没有任何提交，打印类似
 *      "没有提交历史" 的提示后直接返回成功即可（这不是错误）。
 *   3. 循环：
 *        a. minigit_object_read 读出这个 commit 对象，
 *           minigit_commit_parse 解析
 *        b. 打印这次提交：oid（可以用 minigit_oid_to_hex 打印完整 40
 *           位，真实 git 默认打印缩短的 7 位，本项目不强制要求缩短）、
 *           author、message
 *        c. 如果 commit->parent_count == 0，说明到了根提交（第一次
 *           提交，没有更早的历史了），跳出循环
 *        d. 否则 oid = commit->parents[0]（【只走第一父】——如果未来
 *           支持了 merge 提交，一次 merge 会有多个 parent，但线性
 *           历史遍历只需要沿着第一个 parent 走就能还原"主线"，这也是
 *           真实 git log 默认行为的简化版；完整遍历所有分支合并历史
 *           需要更复杂的图遍历，本项目不要求）
 *           继续循环前记得 minigit_commit_free 释放当前这个 commit。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/commit.h"
#include "minigit/object.h"
#include "minigit/refs.h"
#include "minigit/sha1.h"

int cmd_log(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
