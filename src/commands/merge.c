/*
 * cmd_merge — 对应 `git merge <branch>`
 *
 * ============================================================================
 * TODO(你来实现) —— 进阶/选做内容，建议放在其他所有功能都跑通之后再做
 * ============================================================================
 * 用法：`minigit merge <branch-name>`（把 <branch-name> 合并到当前分支）
 *
 * 【第一步：快进合并 (fast-forward)，建议先只做这个】
 *   如果当前分支的 commit 就是 <branch-name> 的祖先（也就是说从当前
 *   commit 沿着 parent 链能走到 <branch-name> 指向的 commit），那么
 *   "合并"不需要产生新的提交，只需要把当前分支的引用直接移动到
 *   <branch-name> 指向的 commit，再把工作区展开成那个 commit 的样子
 *   （复用 cmd_checkout 里"展开 tree 到工作区"的逻辑）即可。
 *
 *   判断"是否祖先"：从 <branch-name> 的 commit 开始，沿 parents[0]（本
 *   项目历史是线性的，只要之前没做过 merge，沿 parent 链走就是完整
 *   历史）一路回溯，看会不会经过当前分支的 commit。
 *
 * 【第二步：真正的三路合并 (three-way merge)，选做】
 *   如果两个分支互不为祖先（分叉了），需要：
 *     1. 找 merge-base：两个分支历史的"最近公共祖先"。本项目历史是线
 *        性单链（没有更早的 merge），可以简化成：分别把两条分支的祖先
 *        链收集成集合，找交集里"离两边都最近"的那个 commit。
 *     2. 对每个在任一分支变动过的文件，做三路对比（base 版本 / 当前
 *        分支版本 / 目标分支版本）：
 *          - 只有一边改了 -> 采用改的那一边
 *          - 两边都没改 -> 保持 base 版本
 *          - 两边都改了且改成不一样的内容 -> 冲突，在文件里写入类似
 *            "<<<<<<< HEAD\n...\n=======\n...\n>>>>>>> branch\n" 的冲突
 *            标记（可以直接复用 minigit_diff_lines 逐行比较，也可以先
 *            实现一个更简单粗暴的版本：只要两边内容不完全一致就整体
 *            当冲突处理，不做逐行合并）。
 *     3. 如果没有冲突，自动创建一个新的 merge commit（parent_count==2，
 *        parents 是两条分支各自的 commit）。如果有冲突，提示用户手动
 *        解决后再 commit（本项目不要求实现"继续合并"的状态机）。
 *
 * 这是全项目里最复杂的一块，建议明确评估自己的时间预算后再决定要做到
 * 哪一步——只做 fast-forward 也已经能体现"理解分支合并的基本原理"这个
 * 学习目标了。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/commit.h"
#include "minigit/object.h"
#include "minigit/refs.h"

int cmd_merge(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
