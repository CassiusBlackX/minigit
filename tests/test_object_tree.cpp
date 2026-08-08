/*
 * test_object_tree.cpp — 阶段 2：tree 对象的序列化/反序列化。
 * 只测试 tree.c 这一层的格式本身，不涉及磁盘读写（那是 object.c 的
 * 职责，已经在 test_object_blob.cpp 里单独测过了）。
 */
#include <gtest/gtest.h>

extern "C" {
#include "minigit/sha1.h"
#include "minigit/tree.h"
}

#include <cstdlib>
#include <cstring>

TEST(Tree, SerializeParseRoundTrip) {
    minigit_tree tree;
    tree.count = 2;
    tree.entries = static_cast<minigit_tree_entry *>(malloc(sizeof(minigit_tree_entry) * 2));

    tree.entries[0].mode = MINIGIT_MODE_BLOB;
    tree.entries[0].name = strdup("a.txt");
    minigit_sha1_buffer("blob content a", 14, &tree.entries[0].oid);

    tree.entries[1].mode = MINIGIT_MODE_TREE;
    tree.entries[1].name = strdup("subdir");
    minigit_sha1_buffer("blob content b", 14, &tree.entries[1].oid);

    unsigned char *data = nullptr;
    size_t size = 0;
    ASSERT_EQ(minigit_tree_serialize(&tree, &data, &size), MINIGIT_OK);
    ASSERT_NE(data, nullptr);

    minigit_tree parsed;
    ASSERT_EQ(minigit_tree_parse(data, size, &parsed), MINIGIT_OK);
    ASSERT_EQ(parsed.count, 2u);

    EXPECT_STREQ(parsed.entries[0].name, "a.txt");
    EXPECT_EQ(parsed.entries[0].mode, static_cast<unsigned>(MINIGIT_MODE_BLOB));
    EXPECT_TRUE(minigit_oid_equal(&parsed.entries[0].oid, &tree.entries[0].oid));

    EXPECT_STREQ(parsed.entries[1].name, "subdir");
    EXPECT_EQ(parsed.entries[1].mode, static_cast<unsigned>(MINIGIT_MODE_TREE));
    EXPECT_TRUE(minigit_oid_equal(&parsed.entries[1].oid, &tree.entries[1].oid));

    free(data);
    minigit_tree_free(&tree);
    minigit_tree_free(&parsed);
}

TEST(Tree, SerializeEmptyTree) {
    minigit_tree tree;
    tree.count = 0;
    tree.entries = nullptr;

    unsigned char *data = nullptr;
    size_t size = 0;
    ASSERT_EQ(minigit_tree_serialize(&tree, &data, &size), MINIGIT_OK);
    EXPECT_EQ(size, 0u);

    free(data);
}

TEST(Tree, ExecutableModeSurvivesRoundTrip) {
    minigit_tree tree;
    tree.count = 1;
    tree.entries = static_cast<minigit_tree_entry *>(malloc(sizeof(minigit_tree_entry)));
    tree.entries[0].mode = MINIGIT_MODE_EXEC;
    tree.entries[0].name = strdup("run.sh");
    minigit_sha1_buffer("#!/bin/sh\n", 10, &tree.entries[0].oid);

    unsigned char *data = nullptr;
    size_t size = 0;
    ASSERT_EQ(minigit_tree_serialize(&tree, &data, &size), MINIGIT_OK);

    minigit_tree parsed;
    ASSERT_EQ(minigit_tree_parse(data, size, &parsed), MINIGIT_OK);
    ASSERT_EQ(parsed.count, 1u);
    EXPECT_EQ(parsed.entries[0].mode, static_cast<unsigned>(MINIGIT_MODE_EXEC));

    free(data);
    minigit_tree_free(&tree);
    minigit_tree_free(&parsed);
}
