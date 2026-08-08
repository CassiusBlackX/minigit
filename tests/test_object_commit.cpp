/*
 * test_object_commit.cpp — 阶段 3：commit 对象的序列化/反序列化。
 */
#include <gtest/gtest.h>

extern "C" {
#include "minigit/commit.h"
#include "minigit/sha1.h"
}

#include <cstdlib>
#include <cstring>

TEST(Commit, SerializeParseRoundTripNoParent) {
    minigit_commit c;
    memset(&c, 0, sizeof(c));
    minigit_sha1_buffer("tree content", 12, &c.tree);
    c.parents = nullptr;
    c.parent_count = 0;
    c.author = strdup("minigit <minigit@example.com> 1735689600 +0000");
    c.committer = strdup("minigit <minigit@example.com> 1735689600 +0000");
    c.message = strdup("Initial commit\n");

    unsigned char *data = nullptr;
    size_t size = 0;
    ASSERT_EQ(minigit_commit_serialize(&c, &data, &size), MINIGIT_OK);
    ASSERT_NE(data, nullptr);

    minigit_commit parsed;
    ASSERT_EQ(minigit_commit_parse(data, size, &parsed), MINIGIT_OK);

    EXPECT_TRUE(minigit_oid_equal(&parsed.tree, &c.tree));
    EXPECT_EQ(parsed.parent_count, 0u);
    EXPECT_STREQ(parsed.author, c.author);
    EXPECT_STREQ(parsed.committer, c.committer);
    EXPECT_STREQ(parsed.message, "Initial commit\n");

    free(data);
    minigit_commit_free(&c);
    minigit_commit_free(&parsed);
}

TEST(Commit, SerializeParseRoundTripWithTwoParents) {
    minigit_commit c;
    memset(&c, 0, sizeof(c));
    minigit_sha1_buffer("tree content 2", 14, &c.tree);

    c.parents = static_cast<minigit_oid *>(malloc(sizeof(minigit_oid) * 2));
    minigit_sha1_buffer("parent one", 10, &c.parents[0]);
    minigit_sha1_buffer("parent two", 10, &c.parents[1]);
    c.parent_count = 2;

    c.author = strdup("minigit <minigit@example.com> 1735689600 +0000");
    c.committer = strdup("minigit <minigit@example.com> 1735689600 +0000");
    c.message = strdup("Merge branch 'feature'\n");

    unsigned char *data = nullptr;
    size_t size = 0;
    ASSERT_EQ(minigit_commit_serialize(&c, &data, &size), MINIGIT_OK);

    minigit_commit parsed;
    ASSERT_EQ(minigit_commit_parse(data, size, &parsed), MINIGIT_OK);
    ASSERT_EQ(parsed.parent_count, 2u);
    EXPECT_TRUE(minigit_oid_equal(&parsed.parents[0], &c.parents[0]));
    EXPECT_TRUE(minigit_oid_equal(&parsed.parents[1], &c.parents[1]));
    EXPECT_STREQ(parsed.message, "Merge branch 'feature'\n");

    free(data);
    minigit_commit_free(&c);
    minigit_commit_free(&parsed);
}
