/*
 * sha1.h — SHA-1 哈希的薄封装（基于 OpenSSL）。
 *
 * 这一层属于"和 git 核心逻辑无关的基础设施"，已经帮你完整实现，不需要
 * 你重新造轮子。你只需要知道：
 *   - minigit_sha1_buffer() 一次性对一段内存算哈希；
 *   - 如果数据量大或者要"边读边算"（未来若你想支持大文件流式哈希可以
 *     扩展 sha1_ctx 系列函数，本项目目前用不到，先不提供）。
 *
 * 注意：真实 git 从 2005 年到最近几年一直用 SHA-1，2020 年后逐步支持
 * SHA-256 仓库（`--object-format=sha256`）。minigit 只做 SHA-1，足够
 * 覆盖"理解原理"这个目标。
 */
#ifndef MINIGIT_SHA1_H
#define MINIGIT_SHA1_H

#include "minigit/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 对 data[0..len) 计算 SHA-1，结果写入 *out。始终成功（除非 data==NULL
 * 且 len!=0），所以不需要检查返回值也可以，但为了和其他 API 风格一致
 * 仍然返回 minigit_status。 */
int minigit_sha1_buffer(const void *data, size_t len, minigit_oid *out);

/* 20 字节原始哈希 -> 40 位小写十六进制字符串。hex_out 至少要有
 * MINIGIT_OID_HEXSZ+1 字节空间（+1 是给 '\0'）。 */
void minigit_oid_to_hex(const minigit_oid *oid, char hex_out[MINIGIT_OID_HEXSZ + 1]);

/* 40 位十六进制字符串 -> 20 字节原始哈希。hex 长度必须至少是
 * MINIGIT_OID_HEXSZ（多余的字符会被忽略，方便你直接传 "abcd1234...\n"
 * 这种从文件里读出来带换行符的字符串）。
 * 成功返回 MINIGIT_OK；遇到非法十六进制字符返回 MINIGIT_ERR_INVALID。 */
int minigit_oid_from_hex(const char *hex, minigit_oid *out);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_SHA1_H */
