/*
 * test_object_blob.cpp — 阶段 1：blob 对象的哈希/写入/读取。
 *
 * 这些测试对应 object.h 里 TODO 的部分，实现完成前会失败——这是预期
 * 行为，不是测试框架本身的问题。先让 CliRootTreeHash 之外的用例通过，
 * 再逐步推进。
 */
#include <gtest/gtest.h>

#include "test_helpers.h"

extern "C" {
#include "minigit/fsutil.h"
#include "minigit/object.h"
#include "minigit/repo.h"
#include "minigit/sha1.h"
}

#include <cstdlib>
#include <cstring>

using minigit_test::TempDir;

namespace {
minigit_repo make_repo(const TempDir &tmp) {
    minigit_repo repo;
    repo.git_dir = strdup((tmp.path() / ".git").string().c_str());
    repo.work_tree = strdup(tmp.path().string().c_str());
    return repo;
}
} // namespace

TEST(ObjectBlob, HashMatchesRealGitHashObject) {
    if (std::system("git --version > /dev/null 2>&1") != 0) {
        GTEST_SKIP() << "系统未安装 git，跳过这个 oracle 对照测试";
    }

    const char *content = "hello minigit\n";
    minigit_oid oid;
    ASSERT_EQ(minigit_object_hash(MINIGIT_OBJ_BLOB, content, strlen(content), &oid), MINIGIT_OK);

    char hex[MINIGIT_OID_HEXSZ + 1];
    minigit_oid_to_hex(&oid, hex);

    TempDir tmp;
    std::string file = (tmp.path() / "content.txt").string();
    ASSERT_EQ(minigit_write_file(file.c_str(), content, strlen(content)), MINIGIT_OK);

    auto [code, output] = minigit_test::run_command("git hash-object " + file);
    ASSERT_EQ(code, 0) << output;
    while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
        output.pop_back();
    }

    EXPECT_EQ(std::string(hex), output)
        << "你算出来的 blob 哈希应该和真实 git 完全一致——如果不一致，"
           "多半是 header 拼接格式或者哈希覆盖范围（是否只 hash 了 "
           "content 而漏了 header）有问题";
}

TEST(ObjectBlob, WriteThenReadRoundTrip) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    const char *content = "round trip content\n";
    minigit_oid oid;
    ASSERT_EQ(minigit_object_write(&repo, MINIGIT_OBJ_BLOB, content, strlen(content), &oid),
              MINIGIT_OK);

    minigit_object obj;
    ASSERT_EQ(minigit_object_read(&repo, &oid, &obj), MINIGIT_OK);
    EXPECT_EQ(obj.type, MINIGIT_OBJ_BLOB);
    ASSERT_EQ(obj.size, strlen(content));
    EXPECT_EQ(memcmp(obj.data, content, obj.size), 0);

    minigit_object_free(&obj);
    minigit_repo_free(&repo);
}

TEST(ObjectBlob, WriteIsContentAddressedAndIdempotent) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    const char *content = "same content twice\n";
    minigit_oid oid1;
    minigit_oid oid2;
    ASSERT_EQ(minigit_object_write(&repo, MINIGIT_OBJ_BLOB, content, strlen(content), &oid1),
              MINIGIT_OK);
    ASSERT_EQ(minigit_object_write(&repo, MINIGIT_OBJ_BLOB, content, strlen(content), &oid2),
              MINIGIT_OK);

    EXPECT_TRUE(minigit_oid_equal(&oid1, &oid2))
        << "内容相同的两次写入必须产生完全相同的 oid——这就是内容寻址存储";

    minigit_repo_free(&repo);
}

TEST(ObjectBlob, ReadMissingObjectReturnsNotFound) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_oid bogus;
    minigit_sha1_buffer("this object was never written", 30, &bogus);

    minigit_object obj;
    EXPECT_EQ(minigit_object_read(&repo, &bogus, &obj), MINIGIT_ERR_NOT_FOUND);

    minigit_repo_free(&repo);
}
