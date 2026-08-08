/*
 * common.h — 贯穿全项目的基础类型与错误码。
 *
 * 放在这里而不是散落在各个头文件里，是为了避免 object.h / index.h /
 * refs.h 之间互相 #include 导致的循环依赖：大家都只依赖 common.h。
 */
#ifndef MINIGIT_COMMON_H
#define MINIGIT_COMMON_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------
 * 错误码
 *
 * 约定：所有 minigit_* API 以 int 返回值表示成败，0 表示成功，
 * 负数表示失败（具体含义见下）。这是 C 标准库 / POSIX 的惯用法，
 * 比"返回 bool + errno"更容易在整个项目里保持一致。
 * ------------------------------------------------------------------- */
typedef enum {
    MINIGIT_OK = 0,
    MINIGIT_ERR = -1,              /* 未归类的通用错误 */
    MINIGIT_ERR_NOT_FOUND = -2,    /* 对象 / 引用 / 文件不存在 */
    MINIGIT_ERR_IO = -3,           /* 文件系统读写失败 */
    MINIGIT_ERR_INVALID = -4,      /* 参数或数据格式非法 */
    MINIGIT_ERR_EXISTS = -5,       /* 试图创建已存在的东西（如重复分支名）*/
    MINIGIT_ERR_NOT_A_REPO = -6,   /* 当前目录及其所有上级都没有 .git */
    MINIGIT_ERR_NOT_IMPLEMENTED = -100 /* 占位骨架尚未被实现——见 TUTORIAL.md */
} minigit_status;

/* ---------------------------------------------------------------------
 * 对象 ID（OID）
 *
 * Git 用 SHA-1 对"对象内容"做哈希，作为对象在 .git/objects/ 下的唯一
 * 身份证。两个字节序列一样的对象，哈希必然一样——这就是"内容寻址存储
 * (content-addressable storage)"，也是 git 天然去重、天然支持完整性
 * 校验的原因。
 *
 * 这里存的是"原始 20 字节二进制"，而不是我们平时在命令行看到的 40 位
 * 十六进制字符串。原始字节更省空间（tree 对象里存的也是这 20 字节），
 * 十六进制只是给人看的展示形式，需要用 minigit_oid_to_hex /
 * minigit_oid_from_hex 相互转换。
 * ------------------------------------------------------------------- */
#define MINIGIT_OID_RAWSZ 20   /* SHA-1 摘要长度：20 字节 = 160 位 */
#define MINIGIT_OID_HEXSZ 40   /* 对应的十六进制字符串长度（不含 \0） */

typedef struct {
    unsigned char id[MINIGIT_OID_RAWSZ];
} minigit_oid;

/* 判断两个 OID 是否相同；等价于 memcmp(a,b,20)==0，但语义更清晰 */
int minigit_oid_equal(const minigit_oid *a, const minigit_oid *b);

/* 全零 OID 常用来表示"不存在的父提交" / "空值"，例如 refs 文件不存在时 */
extern const minigit_oid MINIGIT_OID_ZERO;

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_COMMON_H */
