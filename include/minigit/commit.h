/*
 * commit.h — commit 对象：一次提交的元信息 + 指向的 tree/parent。
 *
 * ============================================================================
 * 【commit 对象的 content 格式 —— 是纯文本，不是二进制！】
 * ============================================================================
 * 和 tree 不同，commit 的 content 是人类可读的文本（这也是为什么
 * `git cat-file -p <commit-hash>` 能直接打印出可读内容）。格式是：
 *
 *     tree <40位十六进制 tree oid>\n
 *     parent <40位十六进制 parent oid>\n      (0 条或多条，一个 parent 一行；
 *                                              初始提交没有 parent 行，
 *                                              merge 提交有多个 parent 行)
 *     author <name> <email> <timestamp> <timezone>\n
 *     committer <name> <email> <timestamp> <timezone>\n
 *     \n
 *     <commit message>
 *
 * 举例（一次有一个 parent 的普通提交）：
 *
 *     tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
 *     parent 7f1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b
 *     author minigit <you@example.com> 1735689600 +0800
 *     committer minigit <you@example.com> 1735689600 +0800
 *
 *     Initial commit
 *
 * 几个细节：
 *   - tree/parent 后面跟的是 40 位十六进制字符串（用
 *     minigit_oid_to_hex/from_hex 转换），不是 tree.h 里那种 20 字节
 *     原始二进制——这是本项目里两种不同引用方式并存的地方，容易搞混。
 *   - timestamp 是 Unix 时间戳（从 1970-01-01 UTC 至今的秒数）的十进制
 *     ASCII；timezone 是形如 "+0800" / "-0500" 的 5 字符时区偏移。
 *     本项目为了简化，author 和 committer 可以固定用同一份信息、同一个
 *     时间——真实 git 会区分"作者"（写代码的人）和"提交者"（执行
 *     commit 操作的人，rebase 时两者常常不同），你如果只做单人使用，
 *     两者相等即可。
 *   - author/committer 之间那一行是【空行】，用来分隔"元信息头部"和
 *     "提交说明正文"——这个结构其实和 email 格式（RFC 822 header +
 *     blank line + body）是一致的，git 最早就是围绕"用邮件补丁协作"
 *     设计的，这不是巧合。
 *   - message 后面不需要额外再补换行规则上的特殊处理，直接把用户输入
 *     的字符串拼上去即可（真实 git 会做一些首尾空白清理，本项目不强制
 *     要求）。
 * ============================================================================
 */
#ifndef MINIGIT_COMMIT_H
#define MINIGIT_COMMIT_H

#include "minigit/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    minigit_oid tree;
    minigit_oid *parents;   /* 长度为 parent_count 的数组，可以是 NULL（parent_count==0） */
    size_t parent_count;
    char *author;           /* 完整的 "name <email> timestamp tz" 字符串 */
    char *committer;
    char *message;
} minigit_commit;

/* ----------------------------------------------------------------------
 * TODO(你来实现): src/commit.c
 * -------------------------------------------------------------------- */

/* 序列化成上面描述的文本格式（调用方 free(*out_data)）。
 * 提示：可以先用一个可增长的缓冲区（或者先 snprintf 到临时 buffer 里拼
 * 接多次），逐行 append "tree ...\n"、每个 "parent ...\n"、
 * "author ...\n"、"committer ...\n"、"\n"、message。 */
int minigit_commit_serialize(const minigit_commit *commit, unsigned char **out_data,
                              size_t *out_size);

/* 反序列化：解析 minigit_object_read 得到的 content，逐行读 "key value"
 * 直到遇到空行，之后的内容都是 message。
 * 提示：可以先把 data 按 '\n' 切行处理，遇到不认识的 key（真实 git commit
 * 里还可能有 gpgsig 等，本项目不需要支持）可以直接忽略那一行。 */
int minigit_commit_parse(const unsigned char *data, size_t size, minigit_commit *out_commit);

/* 释放 parents / author / committer / message */
void minigit_commit_free(minigit_commit *commit);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_COMMIT_H */
