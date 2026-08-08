/*
 * tree.h — tree 对象：一个目录快照的序列化/反序列化。
 *
 * ============================================================================
 * 【tree 对象的 content 二进制格式】
 * ============================================================================
 *
 * tree 是若干条【entry】首尾相连拼成的字节流，每条 entry 的格式是：
 *
 *     <mode 的十进制... 不对，是八进制 ASCII，不带前导 0><空格><name>\0<20 字节原始 oid>
 *
 * 具体拆开讲：
 *   1. mode：文件权限+类型，用 ASCII 十进制数字写出【八进制数值】，
 *      不做零填充。本项目只需要支持这几种：
 *        100644  普通文件（不可执行）
 *        100755  可执行文件
 *        40000   目录（对应一个子 tree；注意这里只有 5 位数字，不是
 *                 "040000"，因为 8 进制数值 040000 本身前导 0 被省略
 *                 显示了——这是最容易写错的一个细节）
 *      （真实 git 还有 120000 符号链接、160000 子模块，本项目不要求
 *      支持，可以先不管）
 *   2. 一个空格 ' '
 *   3. name：文件名或子目录名（不含路径分隔符，只是这一级的名字）
 *   4. 一个 NUL 字节 '\0'
 *   5. 20 字节【原始二进制】oid（不是 40 位十六进制字符串！这是和
 *      commit 里引用 oid 的方式不同的地方，commit 里 parent/tree 都是
 *      写十六进制字符串，tree 里的 entry 是写原始字节，为的是省空间）
 *
 * 一个 tree 对象的 content 就是很多个这样的 entry 紧挨着拼起来，没有
 * 额外的分隔符或者条目数量字段——因为每条 entry 自己就是自描述的
 * （靠 \0 分隔 name 和 oid，靠固定的 20 字节界定 oid 的结束，也就是
 * 下一条 entry 的开始）。
 *
 * ----------------------------------------------------------------------------
 * 【条目排序 —— 一个非常容易踩的坑】
 * ----------------------------------------------------------------------------
 * Git 要求同一个 tree 里的 entries 必须按 name 的字节序排序，这样"内容
 * 相同的目录一定产出相同的 tree 对象"这个内容寻址的前提才成立。
 *
 * 但排序规则有一个反直觉的细节：对目录类型的 entry，排序时要【假装它的
 * 名字后面多了一个 '/'】再比较。举例：如果目录下同时有一个文件
 * "lib.c" 和一个子目录 "lib"，如果直接按字符串比较 "lib" < "lib.c"
 * （因为 "lib" 是 "lib.c" 的前缀，短的排前面），但 git 实际排序时把
 * 目录 "lib" 当作 "lib/" 来比较，而 '/' (0x2F) 比 '.' (0x2E) 大，所以
 * 排序结果其实是 "lib.c" < "lib/"。
 * 如果你不处理这个细节，在绝大多数简单场景下（目录名和文件名不会出现
 * 这种前缀关系）你的 tree hash 仍然会和真实 git 一致；但如果想做到
 * 100% 兼容，请在排序 comparator 里对目录名单独处理。
 * 建议：先不管这个 edge case 把整体流程跑通，最后再回来补这个细节。
 *
 * ============================================================================
 */
#ifndef MINIGIT_TREE_H
#define MINIGIT_TREE_H

#include "minigit/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 三种本项目需要支持的 mode（八进制数值，和上面注释描述的十进制/八进
 * 制 ASCII 表示是同一个数，这里只是用 C 的八进制字面量 0xxx 存成整数，
 * 打印成 ASCII 的活交给你在 serialize 里做） */
#define MINIGIT_MODE_BLOB 0100644u
#define MINIGIT_MODE_EXEC 0100755u
#define MINIGIT_MODE_TREE 0040000u

typedef struct {
    unsigned int mode;
    char *name;       /* 这一级的文件/目录名，拥有所有权，free 时要释放 */
    minigit_oid oid;
} minigit_tree_entry;

typedef struct {
    minigit_tree_entry *entries;
    size_t count;
} minigit_tree;

/* ----------------------------------------------------------------------
 * TODO(你来实现): src/tree.c
 * -------------------------------------------------------------------- */

/* 把 tree 序列化成二进制 content（调用方 free(*out_data)）。
 * 前置条件：调用方保证 tree->entries 已经按前面说的规则排好序——排序
 * 逻辑放在 index.c 的 write_tree 里做，这里只管序列化格式本身。 */
int minigit_tree_serialize(const minigit_tree *tree, unsigned char **out_data,
                            size_t *out_size);

/* 反序列化：从 minigit_object_read 得到的 content 字节流解析出 entries
 * 数组。用于 `cat-file -p` 打印 tree、或者 checkout 时展开 tree。 */
int minigit_tree_parse(const unsigned char *data, size_t size, minigit_tree *out_tree);

/* 释放 tree->entries 数组以及每个 entry->name */
void minigit_tree_free(minigit_tree *tree);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_TREE_H */
