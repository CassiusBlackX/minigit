/*
 * compress.h — zlib (DEFLATE) 的薄封装。
 *
 * Git 的每个 object 文件（.git/objects/xx/yyyy...）都是先拼出
 * "type size\0content" 这样一段明文，再整体做 zlib 压缩后落盘的。
 * 这层压缩本身不是"git 是什么"的核心知识点，OpenSSL/zlib 这种成熟库
 * 该用就用，所以这里已经帮你实现好。你在 object.c 里会直接调用
 * minigit_compress / minigit_decompress。
 */
#ifndef MINIGIT_COMPRESS_H
#define MINIGIT_COMPRESS_H

#include "minigit/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 压缩 src[0..src_len) -> 新分配的缓冲区 *out（调用方用 free() 释放），
 * 压缩后长度写入 *out_len。 */
int minigit_compress(const void *src, size_t src_len, unsigned char **out, size_t *out_len);

/* 解压 src[0..src_len) -> 新分配的缓冲区 *out（调用方用 free() 释放），
 * 解压后长度写入 *out_len。
 * 不需要提前知道解压后的大小：内部会按需扩容缓冲区。 */
int minigit_decompress(const void *src, size_t src_len, unsigned char **out, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_COMPRESS_H */
