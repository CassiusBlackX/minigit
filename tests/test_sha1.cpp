/*
 * test_sha1.cpp — 对应已完整实现的 src/sha1.c，这些测试从一开始就应该
 * 全部通过；如果不通过，说明环境里的 OpenSSL 有问题，而不是你的代码
 * 有问题。
 */
#include <gtest/gtest.h>

extern "C" {
#include "minigit/sha1.h"
}

TEST(Sha1, KnownVectorAbc) {
    // "abc" 的 SHA-1 是一个广为人知的标准测试向量
    minigit_oid oid;
    ASSERT_EQ(minigit_sha1_buffer("abc", 3, &oid), MINIGIT_OK);

    char hex[MINIGIT_OID_HEXSZ + 1];
    minigit_oid_to_hex(&oid, hex);
    EXPECT_STREQ(hex, "a9993e364706816aba3e25717850c26c9cd0d89d");
}

TEST(Sha1, EmptyInput) {
    minigit_oid oid;
    ASSERT_EQ(minigit_sha1_buffer("", 0, &oid), MINIGIT_OK);

    char hex[MINIGIT_OID_HEXSZ + 1];
    minigit_oid_to_hex(&oid, hex);
    EXPECT_STREQ(hex, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
}

TEST(Sha1, HexRoundTrip) {
    minigit_oid oid;
    ASSERT_EQ(minigit_sha1_buffer("minigit", 7, &oid), MINIGIT_OK);

    char hex[MINIGIT_OID_HEXSZ + 1];
    minigit_oid_to_hex(&oid, hex);

    minigit_oid parsed;
    ASSERT_EQ(minigit_oid_from_hex(hex, &parsed), MINIGIT_OK);
    EXPECT_TRUE(minigit_oid_equal(&oid, &parsed));
}

TEST(Sha1, FromHexRejectsInvalidChars) {
    minigit_oid parsed;
    EXPECT_EQ(minigit_oid_from_hex("not-valid-hex-not-valid-hex-not-valid-h", &parsed),
              MINIGIT_ERR_INVALID);
}

TEST(Sha1, OidEqualDetectsDifference) {
    minigit_oid a, b;
    ASSERT_EQ(minigit_sha1_buffer("a", 1, &a), MINIGIT_OK);
    ASSERT_EQ(minigit_sha1_buffer("b", 1, &b), MINIGIT_OK);
    EXPECT_FALSE(minigit_oid_equal(&a, &b));
}
