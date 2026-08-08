/*
 * test_diff.cpp — 阶段 6：行级 diff（LCS）。
 */
#include <gtest/gtest.h>

extern "C" {
#include "minigit/diff.h"
}

#include <cstring>

TEST(Diff, IdenticalTextIsAllEqual) {
    const char *a = "line1\nline2\nline3\n";
    minigit_diff_result result;
    ASSERT_EQ(minigit_diff_lines(a, strlen(a), a, strlen(a), &result), MINIGIT_OK);

    ASSERT_GT(result.count, 0u);
    for (size_t i = 0; i < result.count; i++) {
        EXPECT_EQ(result.lines[i].op, MINIGIT_DIFF_EQUAL);
    }

    minigit_diff_free(&result);
}

TEST(Diff, DetectsOneAddedAndOneRemovedLine) {
    const char *a = "keep1\nremoveme\nkeep2\n";
    const char *b = "keep1\nkeep2\naddedline\n";

    minigit_diff_result result;
    ASSERT_EQ(minigit_diff_lines(a, strlen(a), b, strlen(b), &result), MINIGIT_OK);

    int adds = 0, dels = 0, equals = 0;
    for (size_t i = 0; i < result.count; i++) {
        switch (result.lines[i].op) {
            case MINIGIT_DIFF_ADD: adds++; break;
            case MINIGIT_DIFF_DEL: dels++; break;
            case MINIGIT_DIFF_EQUAL: equals++; break;
        }
    }
    EXPECT_EQ(dels, 1);
    EXPECT_EQ(adds, 1);
    EXPECT_EQ(equals, 2);

    minigit_diff_free(&result);
}

TEST(Diff, EmptyToNonEmptyIsAllAdds) {
    const char *b = "a\nb\n";
    minigit_diff_result result;
    ASSERT_EQ(minigit_diff_lines("", 0, b, strlen(b), &result), MINIGIT_OK);

    for (size_t i = 0; i < result.count; i++) {
        EXPECT_EQ(result.lines[i].op, MINIGIT_DIFF_ADD);
    }

    minigit_diff_free(&result);
}

TEST(Diff, NonEmptyToEmptyIsAllDeletes) {
    const char *a = "a\nb\nc\n";
    minigit_diff_result result;
    ASSERT_EQ(minigit_diff_lines(a, strlen(a), "", 0, &result), MINIGIT_OK);

    for (size_t i = 0; i < result.count; i++) {
        EXPECT_EQ(result.lines[i].op, MINIGIT_DIFF_DEL);
    }

    minigit_diff_free(&result);
}
