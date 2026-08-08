/*
 * fsutil.h — 文件系统相关的小工具函数（chore，已完整实现）。
 *
 * 这些是任何 C 项目都会需要的"读整个文件到内存 / 写内存到文件 / 递归
 * mkdir / 拼路径"之类的样板代码，和 git 原理无关，直接用即可。
 */
#ifndef MINIGIT_FSUTIL_H
#define MINIGIT_FSUTIL_H

#include "minigit/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 判断路径是否存在 */
int minigit_path_exists(const char *path);

/* 判断路径存在且是目录 */
int minigit_path_is_dir(const char *path);

/* 递归创建目录（等价于 `mkdir -p`）。已存在也返回 MINIGIT_OK。 */
int minigit_mkdir_p(const char *path);

/* 把整个文件读入新分配的缓冲区（调用方 free）。*len 返回文件字节数。
 * 注意：返回的缓冲区【不】保证以 '\0' 结尾——它按二进制数据处理，因为
 * git object 内容本来就可能是任意字节。如果你要当字符串用，自己在读到
 * 的基础上加一个字节。 */
int minigit_read_file(const char *path, unsigned char **buf, size_t *len);

/* 把 buf[0..len) 整体写入 path，若文件已存在则覆盖。会自动创建父目录。 */
int minigit_write_file(const char *path, const void *buf, size_t len);

/* 拼接两段路径，中间补一个 '/'（除非 a 已以 '/' 结尾）。
 * 返回新分配的字符串，调用方 free()。 */
char *minigit_path_join(const char *a, const char *b);

/* 递归遍历 root 目录下所有【普通文件】（不含目录本身），对每个文件调用
 * cb(relpath, userdata)，relpath 是相对 root 的路径（用 '/' 分隔，不带
 * 前导 "./"）。会自动跳过名为 ".git" 的目录（否则遍历工作区时会把仓库
 * 自己的内部数据当成"工作区文件"，这是几乎所有 porcelain 命令——
 * add "."/status/checkout——都需要的行为）。
 *
 * cb 返回非 0 会中止整个遍历，minigit_walk_files 把那个返回值原样传
 * 出来；cb 全部返回 0 且遍历正常结束，返回 MINIGIT_OK。
 *
 * 已完整实现（纯目录遍历的样板代码，chore）。 */
typedef int (*minigit_walk_fn)(const char *relpath, void *userdata);
int minigit_walk_files(const char *root, minigit_walk_fn cb, void *userdata);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_FSUTIL_H */
