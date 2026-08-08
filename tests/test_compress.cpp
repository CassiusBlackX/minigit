/*
 * test_compress.cpp — 对应已完整实现的 src/compress.c，从一开始就应该
 * 全部通过。
 */
#include <gtest/gtest.h>

extern "C" {
#include "minigit/compress.h"
}

#include <cstdlib>
#include <cstring>

TEST(Compress, RoundTripRepetitiveData) {
    const char *data =
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog.";
    size_t data_len = strlen(data);

    unsigned char *compressed = nullptr;
    size_t compressed_len = 0;
    ASSERT_EQ(minigit_compress(data, data_len, &compressed, &compressed_len), MINIGIT_OK);
    ASSERT_NE(compressed, nullptr);
    EXPECT_LT(compressed_len, data_len) << "重复度这么高的数据压缩后应该比原始数据小";

    unsigned char *decompressed = nullptr;
    size_t decompressed_len = 0;
    ASSERT_EQ(minigit_decompress(compressed, compressed_len, &decompressed, &decompressed_len),
              MINIGIT_OK);
    ASSERT_EQ(decompressed_len, data_len);
    EXPECT_EQ(memcmp(decompressed, data, data_len), 0);

    free(compressed);
    free(decompressed);
}

TEST(Compress, RoundTripEmptyInput) {
    unsigned char *compressed = nullptr;
    size_t compressed_len = 0;
    ASSERT_EQ(minigit_compress("", 0, &compressed, &compressed_len), MINIGIT_OK);

    unsigned char *decompressed = nullptr;
    size_t decompressed_len = 0;
    ASSERT_EQ(minigit_decompress(compressed, compressed_len, &decompressed, &decompressed_len),
              MINIGIT_OK);
    EXPECT_EQ(decompressed_len, 0u);

    free(compressed);
    free(decompressed);
}

TEST(Compress, DecompressRejectsGarbageInput) {
    const char *garbage = "this is definitely not a valid zlib stream";
    unsigned char *decompressed = nullptr;
    size_t decompressed_len = 0;
    EXPECT_EQ(minigit_decompress(garbage, strlen(garbage), &decompressed, &decompressed_len),
              MINIGIT_ERR_INVALID);
}
