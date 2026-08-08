/*
 * test_fsutil.cpp — 对应已完整实现的 src/fsutil.c，从一开始就应该全部
 * 通过。
 */
#include <gtest/gtest.h>

#include "test_helpers.h"

extern "C" {
#include "minigit/fsutil.h"
}

#include <cstdlib>
#include <cstring>
#include <set>
#include <string>

using minigit_test::TempDir;

TEST(FsUtil, WriteFileCreatesParentDirs) {
    TempDir tmp;
    std::string path = (tmp.path() / "a" / "b" / "c.txt").string();

    ASSERT_EQ(minigit_write_file(path.c_str(), "hello", 5), MINIGIT_OK);
    EXPECT_TRUE(minigit_path_exists(path.c_str()));

    unsigned char *buf = nullptr;
    size_t len = 0;
    ASSERT_EQ(minigit_read_file(path.c_str(), &buf, &len), MINIGIT_OK);
    ASSERT_EQ(len, 5u);
    EXPECT_EQ(memcmp(buf, "hello", 5), 0);
    free(buf);
}

TEST(FsUtil, ReadMissingFileReturnsNotFound) {
    TempDir tmp;
    std::string path = (tmp.path() / "does-not-exist.txt").string();
    unsigned char *buf = nullptr;
    size_t len = 0;
    EXPECT_EQ(minigit_read_file(path.c_str(), &buf, &len), MINIGIT_ERR_NOT_FOUND);
}

TEST(FsUtil, PathJoinHandlesTrailingSlash) {
    char *joined1 = minigit_path_join("/a/b", "c");
    EXPECT_STREQ(joined1, "/a/b/c");
    free(joined1);

    char *joined2 = minigit_path_join("/a/b/", "c");
    EXPECT_STREQ(joined2, "/a/b/c");
    free(joined2);
}

TEST(FsUtil, MkdirPIsIdempotent) {
    TempDir tmp;
    std::string dir = (tmp.path() / "x" / "y" / "z").string();
    ASSERT_EQ(minigit_mkdir_p(dir.c_str()), MINIGIT_OK);
    ASSERT_EQ(minigit_mkdir_p(dir.c_str()), MINIGIT_OK); // 已存在也应该成功
    EXPECT_TRUE(minigit_path_is_dir(dir.c_str()));
}

namespace {
int collect_relpath(const char *relpath, void *userdata) {
    static_cast<std::set<std::string> *>(userdata)->insert(relpath);
    return 0;
}
} // namespace

TEST(FsUtil, WalkFilesFindsNestedFilesAndSkipsDotGit) {
    TempDir tmp;
    ASSERT_EQ(minigit_write_file((tmp.path() / "top.txt").string().c_str(), "x", 1), MINIGIT_OK);
    ASSERT_EQ(minigit_write_file((tmp.path() / "sub" / "nested.txt").string().c_str(), "y", 1),
              MINIGIT_OK);
    ASSERT_EQ(
        minigit_write_file((tmp.path() / ".git" / "objects" / "dummy").string().c_str(), "z", 1),
        MINIGIT_OK);

    std::set<std::string> seen;
    ASSERT_EQ(minigit_walk_files(tmp.path().string().c_str(), collect_relpath, &seen), MINIGIT_OK);

    EXPECT_EQ(seen.count("top.txt"), 1u);
    EXPECT_EQ(seen.count("sub/nested.txt"), 1u);
    for (const auto &relpath : seen) {
        EXPECT_EQ(relpath.find(".git"), std::string::npos) << "不应该遍历到 .git 目录内部";
    }
}
