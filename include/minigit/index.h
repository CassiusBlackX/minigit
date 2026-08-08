/*
 * index.h — 暂存区 (staging area / index)：工作区和提交之间的中间状态。
 *
 * ============================================================================
 * 【index 是什么，为什么需要它】
 * ============================================================================
 * 真实 git 的 "working tree -> add -> commit" 三段式，中间的 index 其实
 * 是一份【扁平的文件清单】："这次 commit 应该包含哪些路径，每个路径对
 * 应哪个 blob"。`git add` 只是把"文件当前内容"写成一个 blob 对象、
 * 然后在这份清单里登记一条 "path -> blob oid"；`git commit` 则是把这份
 * 扁平清单，按路径的目录层级，组装成一棵 tree 对象树。
 *
 * 也就是说：blob 已经在 add 阶段就写好了，index 只是"记账本"；
 * commit 阶段真正的工作是【把扁平记账本，折叠成一棵嵌套的 tree】。
 * 这也是本文件里 minigit_index_write_tree 是全项目最有意思的一个函数
 * 的原因。
 *
 * ----------------------------------------------------------------------------
 * 【本项目对 index 磁盘格式的简化】
 * ----------------------------------------------------------------------------
 * 真实 git 的 .git/index 是一个带校验和的二进制格式（记录了 mtime、
 * inode、uid/gid 等一大堆用来加速 `git status` 的元数据）。这些元数据
 * 是性能优化，不是"git 是什么"的核心概念，本项目不实现。
 *
 * minigit 把 index 存成一个简单的文本文件（.git/index），每行一条
 * entry：
 *
 *     <mode 八进制ASCII> <oid 40位十六进制> <path>\n
 *
 * 例如：
 *     100644 e69de29bb2d1d6434b8b29ae775ad8c2e48c5391 src/main.c
 *     100644 3b18e512dba79e4c8300dd08aeb37f8e728b8dad README.md
 *
 * entries 按 path 的字典序排序并保持这个不变式（load 完、每次 add/
 * remove 之后都应该是有序的）——这不仅让文件内容稳定、方便 diff，也让
 * write_tree 的分组算法更简单（见下）。
 * ============================================================================
 */
#ifndef MINIGIT_INDEX_H
#define MINIGIT_INDEX_H

#include "minigit/common.h"
#include "minigit/repo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int mode;  /* MINIGIT_MODE_BLOB / MINIGIT_MODE_EXEC，见 tree.h */
    char *path;          /* 相对工作区根目录的完整路径，如 "src/main.c" */
    minigit_oid oid;
} minigit_index_entry;

typedef struct {
    minigit_index_entry *entries;
    size_t count;
    size_t capacity;
} minigit_index;

/* ----------------------------------------------------------------------
 * TODO(你来实现): src/index.c
 * -------------------------------------------------------------------- */

/* 从 .git/index 加载。文件不存在时视为"空 index"（返回 MINIGIT_OK，
 * out_index->count == 0），因为一个刚 init 的仓库本来就还没有任何
 * staged 文件。 */
int minigit_index_load(const minigit_repo *repo, minigit_index *out_index);

/* 把内存里的 index 完整写回 .git/index（覆盖）。 */
int minigit_index_save(const minigit_repo *repo, const minigit_index *index);

/* 新增或更新一条 entry（如果 path 已存在就更新 mode/oid，否则插入并
 * 保持数组按 path 有序）。 */
int minigit_index_add(minigit_index *index, const char *path, unsigned int mode,
                       const minigit_oid *oid);

/* 删除一条 entry；path 不存在返回 MINIGIT_ERR_NOT_FOUND。 */
int minigit_index_remove(minigit_index *index, const char *path);

/* 按 path 精确查找；找不到返回 NULL。 */
minigit_index_entry *minigit_index_find(const minigit_index *index, const char *path);

/* --------------------------------------------------------------------
 * write_tree：本模块最核心的函数。
 *
 * 把扁平的 index（一堆 "a/b/c.txt -> oid" 这样的记录）折叠成一棵嵌套
 * 的 tree 对象树，返回根 tree 的 oid。
 *
 * 算法建议（递归 + 按目录前缀分组）：
 *   把问题抽象成："给定一段已按 path 排序、且这些 path 都共享同一个
 *   目录前缀（该前缀已从 path 字符串里去掉）的 entries，构建出这一层
 *   目录对应的 tree 对象"。
 *
 *   1. 遍历这一段 entries：
 *        - 如果某条 entry 的（剥掉前缀后的）path 里不含 '/'，说明它是
 *          这一层目录的直接文件，直接生成一条
 *          {mode: entry->mode, name: path, oid: entry->oid} 的 tree
 *          entry。
 *        - 如果 path 里含 '/'，说明它属于某个子目录，取第一段（第一个
 *          '/' 之前的部分）作为子目录名。把【所有共享同一个子目录名的
 *          连续 entries】收集成一组（因为数组整体有序，同一子目录的
 *          entries 一定是连续的一段，这就是为什么"保持 index 有序"这
 *          个不变式很重要——不需要额外排序或哈希分组，一次线性扫描
 *          就能把每个子目录的 entries 段找出来）。
 *   2. 对每一组子目录 entries，把公共前缀（子目录名 + '/'）从每条
 *      path 里再去掉一层，递归调用自己，得到这个子目录的 tree oid。
 *      生成一条 {mode: MINIGIT_MODE_TREE, name: 子目录名, oid: 递归
 *      结果} 的 tree entry。
 *   3. 把这一层收集到的所有 tree entries（文件的 + 子目录的）按 tree.h
 *      里说的排序规则排好序，调用 minigit_tree_serialize + 用
 *      MINIGIT_OBJ_TREE 类型 minigit_object_write，得到并返回这一层的
 *      tree oid。
 *
 *   顶层调用时传入完整的 index->entries（前缀为空字符串），返回值就是
 *   `git write-tree` 意义上的根 tree oid。
 * ------------------------------------------------------------------ */
int minigit_index_write_tree(const minigit_repo *repo, const minigit_index *index,
                              minigit_oid *out_tree_oid);

/* 释放 entries 数组以及每个 entry->path */
void minigit_index_free(minigit_index *index);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_INDEX_H */
