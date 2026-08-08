/*
 * object.h — Git 对象模型的基石：blob / tree / commit 的统一读写接口。
 *
 * ============================================================================
 * 【你需要理解并实现的核心知识点】
 * ============================================================================
 *
 * Git 仓库本质上是一个极简的键值数据库：
 *     key   = 对象内容的 SHA-1 哈希（40 位十六进制）
 *     value = 对象内容本身（经过 zlib 压缩）
 *
 * 这种"key 由 value 的哈希决定"的存储方式叫【内容寻址存储】
 * (content-addressable storage)。它带来两个直接后果：
 *   1. 相同内容永远只存一份（天然去重）——两个文件内容一样，blob 就是
 *      同一个对象；
 *   2. 对象一旦写入就不可能在不改变哈希的情况下被篡改（完整性校验是
 *      免费的）。
 *
 * Git 定义了三种对象类型（本项目暂不实现 tag）：
 *   - blob   : 一个文件的内容，不包含文件名、权限等元信息
 *   - tree   : 一个目录的快照，是"文件名/权限 -> blob 或子 tree 的 oid"
 *              的列表
 *   - commit : 一次提交，指向一个 tree（当时的完整目录快照）+ 若干个
 *              parent commit + 作者/提交者信息 + 提交说明
 *
 * ----------------------------------------------------------------------------
 * 【松散对象 (loose object) 的磁盘格式 —— 这是你要实现的核心】
 * ----------------------------------------------------------------------------
 *
 * 给定对象类型 type（"blob" / "tree" / "commit"）和内容 content（原始
 * 字节，长度 size），构造对象的步骤是：
 *
 *   1. 拼出【header】："<type> <size>\0"
 *        - <type> 是类型名字符串（"blob"/"tree"/"commit"）
 *        - 一个空格
 *        - <size> 是 content 长度的十进制 ASCII 表示（不含前导零，
 *          size==0 时就是字符 '0'）
 *        - 一个 NUL 字节 '\0' 作为 header 结束标志
 *      例如内容是 "hello\n"（6 字节）的 blob，header 是 "blob 6\0"
 *
 *   2. 拼出【store】= header + content （注意：header 和 content 之间
 *      没有任何额外分隔符，header 本身以 \0 结尾就是分隔符）
 *
 *   3. 对【整个 store】（header+content，不是只对 content！）计算
 *      SHA-1，得到这个对象的 oid。这一点是最容易踩的坑：如果你只对
 *      content 算哈希，得到的 oid 会和真实 git 的结果对不上。
 *
 *   4. 用 zlib 压缩整个 store，得到 compressed。
 *
 *   5. 把 compressed 写入文件
 *      "<git_dir>/objects/<oid的前2位hex>/<oid的后38位hex>"
 *      例如 oid 十六进制是 e69de29bb2d1d6434b8b29ae775ad8c2e48c5391，
 *      文件路径就是 objects/e6/9de29bb2d1d6434b8b29ae775ad8c2e48c5391
 *      （用两位做一级子目录是为了避免 objects/ 下堆几万个文件拖垮
 *      文件系统性能）
 *
 *      如果目标文件已经存在，直接跳过写入即可（内容寻址天然去重，
 *      没必要覆盖一个内容完全相同的文件）。
 *
 * 读取（minigit_object_read）就是完全反过来的流程：按 oid 拼出路径 ->
 * 读文件 -> zlib 解压得到 store -> 找到第一个 '\0' 切出 header -> 解析
 * header 里的 type 和 size -> 校验 size 是否等于 store 剩余长度 ->
 * content 就是 \0 之后的部分。
 *
 * ----------------------------------------------------------------------------
 * 建议实现顺序：先写 minigit_object_write（内部会用到 sha1.h /
 * compress.h / fsutil.h，都已经帮你实现好），跑通 test_object_blob.cpp
 * 里的 "哈希值应该和系统 git hash-object 算出来的一致" 这个用例，就说明
 * 你的格式完全对了；再写 minigit_object_read 做逆过程。
 * tree / commit 的"内容"具体怎么组织，见 tree.h / commit.h ——那两个
 * 文件负责把结构体序列化成 content 字节流，本文件只管"content 字节流
 * <-> 磁盘上的松散对象文件"这一层，两者是解耦的。
 * ============================================================================
 */
#ifndef MINIGIT_OBJECT_H
#define MINIGIT_OBJECT_H

#include "minigit/common.h"
#include "minigit/repo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MINIGIT_OBJ_BLOB,
    MINIGIT_OBJ_TREE,
    MINIGIT_OBJ_COMMIT,
} minigit_obj_type;

/* 一个从磁盘读出来、已解压、已解析完 header 的对象 */
typedef struct {
    minigit_obj_type type;
    unsigned char *data; /* content，不含 header，调用方通过 minigit_object_free 释放 */
    size_t size;
} minigit_object;

/* "blob"/"tree"/"commit" <-> 枚举值 的互相转换，已完整实现（纯字符串
 * 表格查找，没有需要你学习的原理），直接用即可。 */
const char *minigit_obj_type_name(minigit_obj_type type);
int minigit_obj_type_from_name(const char *name, minigit_obj_type *out_type);

/* ----------------------------------------------------------------------
 * TODO(你来实现): src/object.c
 * -------------------------------------------------------------------- */

/* 只计算 oid，不写盘。用于 `minigit hash-object`（不带 -w）之类的场景，
 * 也方便你在写 write 函数之前先单独测试"哈希算得对不对"。 */
int minigit_object_hash(minigit_obj_type type, const void *data, size_t size,
                         minigit_oid *out_oid);

/* 完整流程：算 header+哈希 -> zlib 压缩 -> 写入 .git/objects/xx/yyyy...
 * （文件已存在则跳过写入）。成功后 *out_oid 是这个对象的 oid。 */
int minigit_object_write(const minigit_repo *repo, minigit_obj_type type,
                          const void *data, size_t size, minigit_oid *out_oid);

/* 根据 oid 读取对象：定位文件 -> 解压 -> 解析 header -> 填充 *out。
 * 找不到对应文件返回 MINIGIT_ERR_NOT_FOUND；header 格式非法或 size 对
 * 不上返回 MINIGIT_ERR_INVALID。 */
int minigit_object_read(const minigit_repo *repo, const minigit_oid *oid,
                         minigit_object *out);

/* 释放 minigit_object_read 填充的 data */
void minigit_object_free(minigit_object *obj);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_OBJECT_H */
