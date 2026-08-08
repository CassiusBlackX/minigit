/*
 * sha1.c — 已完整实现，直接使用（薄封装工具，不是本项目要学习的核心内容）。
 */
#include "minigit/sha1.h"

#include <ctype.h>
#include <openssl/evp.h>
#include <string.h>

int minigit_oid_equal(const minigit_oid *a, const minigit_oid *b) {
    return memcmp(a->id, b->id, MINIGIT_OID_RAWSZ) == 0;
}

const minigit_oid MINIGIT_OID_ZERO = {{0}};

int minigit_sha1_buffer(const void *data, size_t len, minigit_oid *out) {
    if (out == NULL || (data == NULL && len != 0)) {
        return MINIGIT_ERR_INVALID;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return MINIGIT_ERR;
    }

    unsigned int digest_len = 0;
    int ok = EVP_DigestInit_ex(ctx, EVP_sha1(), NULL) == 1 &&
             EVP_DigestUpdate(ctx, data, len) == 1 &&
             EVP_DigestFinal_ex(ctx, out->id, &digest_len) == 1;

    EVP_MD_CTX_free(ctx);

    if (!ok || digest_len != MINIGIT_OID_RAWSZ) {
        return MINIGIT_ERR;
    }
    return MINIGIT_OK;
}

void minigit_oid_to_hex(const minigit_oid *oid, char hex_out[MINIGIT_OID_HEXSZ + 1]) {
    static const char digits[] = "0123456789abcdef";
    for (int i = 0; i < MINIGIT_OID_RAWSZ; i++) {
        hex_out[i * 2] = digits[(oid->id[i] >> 4) & 0xF];
        hex_out[i * 2 + 1] = digits[oid->id[i] & 0xF];
    }
    hex_out[MINIGIT_OID_HEXSZ] = '\0';
}

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = (char)tolower((unsigned char)c);
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

int minigit_oid_from_hex(const char *hex, minigit_oid *out) {
    if (hex == NULL || out == NULL) {
        return MINIGIT_ERR_INVALID;
    }
    for (int i = 0; i < MINIGIT_OID_RAWSZ; i++) {
        int hi = hex_nibble(hex[i * 2]);
        int lo = hex_nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) {
            return MINIGIT_ERR_INVALID;
        }
        out->id[i] = (unsigned char)((hi << 4) | lo);
    }
    return MINIGIT_OK;
}
