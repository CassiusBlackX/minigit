/*
 * refs.h — 引用系统：给 commit 哈希起"人话名字"，并追踪"当前在哪"。
 *
 * ============================================================================
 * 【引用系统的两层结构】
 * ============================================================================
 * 到目前为止（object/tree/commit），仓库里的一切都靠 40 位十六进制哈希
 * 定位——但没人会记得住哈希。引用系统在哈希上面盖了一层"人话名字"：
 *
 *   .git/refs/heads/<branch>   内容是一行 40 位十六进制 oid + '\n'
 *       例如 .git/refs/heads/main 里存的就是 main 分支当前指向的
 *       commit 哈希。"创建分支"在磁盘上就是"新建一个这样的文件"，
 *       "分支前进一步"就是"把文件内容改成新的 commit 哈希"——理解了
 *       这一点，你会发现分支在 git 里极其轻量。
 *
 *   .git/HEAD                  【当前所在位置】，两种可能的内容：
 *       - "ref: refs/heads/main\n"   —— 【符号引用】，表示"我在 main
 *         分支上"，实际 commit 要再跳一层去读 refs/heads/main 才知道。
 *         这是绝大多数时候的正常状态。
 *       - "7f1b2c3d...（40位哈希）\n" —— 【分离头指针 / detached HEAD】，
 *         直接指向某个 commit，不属于任何分支。真实 git 里
 *         `git checkout <commit-hash>`（而不是 checkout 一个分支名）
 *         就会进入这个状态。本项目的 checkout 可以只支持"切到分支"，
 *         不强制要求支持 detached HEAD，但 minigit_ref_resolve 建议
 *         把这个情况也处理了（判断内容是不是以 "ref: " 开头即可区分），
 *         因为逻辑并不复杂，还能让 `minigit log` 在极端情况下更健壮。
 *
 * 也就是说，"HEAD 指向的 commit 是谁"这个问题，最多需要跳两层间接：
 *     HEAD --(符号引用)--> refs/heads/main --(内容就是 oid)--> commit
 *
 * ----------------------------------------------------------------------------
 * 关于函数参数里的 "ref_name"：统一约定为【相对 .git 目录的路径】，例如
 * "HEAD"、"refs/heads/main"。这样 minigit_repo_git_path 可以直接拿它
 * 拼出完整文件路径，不需要额外做 "HEAD 是特例" 之类的分支判断。
 * ============================================================================
 */
#ifndef MINIGIT_REFS_H
#define MINIGIT_REFS_H

#include "minigit/common.h"
#include "minigit/repo.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * TODO(你来实现): src/refs.c
 * -------------------------------------------------------------------- */

/* 解析 ref_name 最终指向的 commit oid。
 * 需要处理：
 *   - ref_name 对应的文件内容以 "ref: " 开头 -> 是符号引用，去掉前缀、
 *     去掉结尾换行，得到新的 ref_name，递归/循环继续解析；
 *   - 否则内容就是 40 位十六进制 oid，用 minigit_oid_from_hex 转换后
 *     返回。
 * 文件不存在（比如刚 init 完还没有任何 commit 时的 refs/heads/main）
 * 返回 MINIGIT_ERR_NOT_FOUND —— 调用方（比如 log/status）要能正确处理
 * "仓库存在但还没有任何提交"这种情况。 */
int minigit_ref_resolve(const minigit_repo *repo, const char *ref_name, minigit_oid *out_oid);

/* 直接把 ref_name 对应的文件内容设为 oid 的十六进制形式（会自动创建
 * 父目录，如首次创建 refs/heads/main）。不做符号引用跟随——如果
 * ref_name 本身是符号引用（如 "HEAD" 处于跟随 main 的状态），调用方
 * 应该自己先解析出真正要写的文件是哪个（通常是配合
 * minigit_ref_current_branch 得到分支名，再更新 refs/heads/<branch>）。 */
int minigit_ref_update(const minigit_repo *repo, const char *ref_name, const minigit_oid *oid);

/* 读取 HEAD：
 *   - 如果是符号引用且指向 refs/heads/<name>，*out_name 返回新分配的
 *     字符串 "<name>"（调用方 free），返回 MINIGIT_OK；
 *   - 如果是 detached（内容是原始 oid），返回 MINIGIT_ERR_INVALID，
 *     *out_name 不修改——调用方据此判断"当前处于 detached HEAD"。 */
int minigit_ref_current_branch(const minigit_repo *repo, char **out_name);

/* 把 HEAD 设为符号引用，指向 refs/heads/<branch_name>（不要求
 * branch_name 对应的文件已存在——`minigit checkout -b` 之类场景可能需
 * 要先切 HEAD 再单独创建分支文件，两步操作解耦更灵活）。 */
int minigit_ref_set_head_symbolic(const minigit_repo *repo, const char *branch_name);

/* 创建一个新分支：写 refs/heads/<name> = oid 的十六进制。
 * 如果同名分支已存在，返回 MINIGIT_ERR_EXISTS，不覆盖。 */
int minigit_ref_create_branch(const minigit_repo *repo, const char *name, const minigit_oid *oid);

/* 列出 refs/heads/ 下所有分支名（不含路径前缀）。
 * *out_names 是新分配的字符串数组（每个字符串和数组本身都要 free），
 * *out_count 是数量。用于 `minigit branch`（不带参数，列出所有分支）。 */
int minigit_ref_list_branches(const minigit_repo *repo, char ***out_names, size_t *out_count);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_REFS_H */
