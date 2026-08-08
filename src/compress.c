/*
 * compress.c — 已完整实现，直接使用（薄封装工具，不是本项目要学习的核心内容）。
 */
#include "minigit/compress.h"

#include <stdlib.h>
#include <string.h>
#include <zlib.h>

int minigit_compress(const void *src, size_t src_len, unsigned char **out, size_t *out_len) {
    if (out == NULL || out_len == NULL || (src == NULL && src_len != 0)) {
        return MINIGIT_ERR_INVALID;
    }

    uLongf bound = compressBound((uLong)src_len);
    unsigned char *buf = malloc(bound);
    if (buf == NULL) {
        return MINIGIT_ERR;
    }

    uLongf dest_len = bound;
    int rc = compress2(buf, &dest_len, (const Bytef *)src, (uLong)src_len, Z_DEFAULT_COMPRESSION);
    if (rc != Z_OK) {
        free(buf);
        return MINIGIT_ERR;
    }

    *out = buf;
    *out_len = (size_t)dest_len;
    return MINIGIT_OK;
}

int minigit_decompress(const void *src, size_t src_len, unsigned char **out, size_t *out_len) {
    if (out == NULL || out_len == NULL || (src == NULL && src_len != 0)) {
        return MINIGIT_ERR_INVALID;
    }

    /* 我们不预先知道解压后的大小（git object 头里虽然写了 size，但那是
     * object.c 的语义，这一层不该关心），所以用一个不断翻倍的缓冲区，
     * 循环调用底层的流式 API 直到 zlib 报告"到达输入流末尾"。 */
    size_t cap = src_len > 0 ? src_len * 4 : 64;
    unsigned char *buf = malloc(cap);
    if (buf == NULL) {
        return MINIGIT_ERR;
    }

    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    if (inflateInit(&strm) != Z_OK) {
        free(buf);
        return MINIGIT_ERR;
    }

    strm.next_in = (Bytef *)src;
    strm.avail_in = (uInt)src_len;

    size_t total = 0;
    int zrc = Z_OK;
    while (zrc != Z_STREAM_END) {
        if (total == cap) {
            cap *= 2;
            unsigned char *grown = realloc(buf, cap);
            if (grown == NULL) {
                free(buf);
                inflateEnd(&strm);
                return MINIGIT_ERR;
            }
            buf = grown;
        }
        strm.next_out = buf + total;
        strm.avail_out = (uInt)(cap - total);

        zrc = inflate(&strm, Z_NO_FLUSH);
        if (zrc != Z_OK && zrc != Z_STREAM_END) {
            free(buf);
            inflateEnd(&strm);
            return MINIGIT_ERR_INVALID; /* 数据不是合法的 zlib 流 */
        }
        total = cap - strm.avail_out;
    }

    inflateEnd(&strm);

    *out = buf;
    *out_len = total;
    return MINIGIT_OK;
}
