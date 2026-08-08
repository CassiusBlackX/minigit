/*
 * test_refs.cpp — 阶段 5：HEAD / 分支引用的解析与更新。
 */
#include <gtest/gtest.h>

#include "test_helpers.h"

extern "C" {
#include "minigit/fsutil.h"
#include "minigit/refs.h"
#include "minigit/sha1.h"
}

#include <cstdlib>
#include <cstring>
#include <set>
#include <string>

using minigit_test::TempDir;

namespace {
minigit_repo make_repo(const TempDir &tmp) {
    minigit_repo repo;
    repo.git_dir = strdup((tmp.path() / ".git").string().c_str());
    repo.work_tree = strdup(tmp.path().string().c_str());
    minigit_mkdir_p(repo.git_dir);
    return repo;
}
} // namespace

TEST(Refs, UpdateThenResolveDirectRef) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_oid oid;
    minigit_sha1_buffer("some commit content", 20, &oid);
    ASSERT_EQ(minigit_ref_update(&repo, "refs/heads/main", &oid), MINIGIT_OK);

    minigit_oid resolved;
    ASSERT_EQ(minigit_ref_resolve(&repo, "refs/heads/main", &resolved), MINIGIT_OK);
    EXPECT_TRUE(minigit_oid_equal(&oid, &resolved));

    minigit_repo_free(&repo);
}

TEST(Refs, ResolveMissingRefReturnsNotFound) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_oid oid;
    EXPECT_EQ(minigit_ref_resolve(&repo, "refs/heads/does-not-exist", &oid), MINIGIT_ERR_NOT_FOUND);

    minigit_repo_free(&repo);
}

TEST(Refs, SymbolicHeadFollowsToBranchTarget) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_oid oid;
    minigit_sha1_buffer("head target commit", 19, &oid);
    ASSERT_EQ(minigit_ref_update(&repo, "refs/heads/main", &oid), MINIGIT_OK);
    ASSERT_EQ(minigit_ref_set_head_symbolic(&repo, "main"), MINIGIT_OK);

    minigit_oid resolved;
    ASSERT_EQ(minigit_ref_resolve(&repo, "HEAD", &resolved), MINIGIT_OK);
    EXPECT_TRUE(minigit_oid_equal(&oid, &resolved));

    char *branch = nullptr;
    ASSERT_EQ(minigit_ref_current_branch(&repo, &branch), MINIGIT_OK);
    EXPECT_STREQ(branch, "main");
    free(branch);

    minigit_repo_free(&repo);
}

TEST(Refs, CreateBranchRejectsDuplicateName) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_oid oid;
    minigit_sha1_buffer("x", 1, &oid);
    ASSERT_EQ(minigit_ref_create_branch(&repo, "feature", &oid), MINIGIT_OK);
    EXPECT_EQ(minigit_ref_create_branch(&repo, "feature", &oid), MINIGIT_ERR_EXISTS);

    minigit_repo_free(&repo);
}

TEST(Refs, ListBranchesReturnsAllCreatedBranches) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_oid oid;
    minigit_sha1_buffer("x", 1, &oid);
    ASSERT_EQ(minigit_ref_create_branch(&repo, "main", &oid), MINIGIT_OK);
    ASSERT_EQ(minigit_ref_create_branch(&repo, "dev", &oid), MINIGIT_OK);

    char **names = nullptr;
    size_t count = 0;
    ASSERT_EQ(minigit_ref_list_branches(&repo, &names, &count), MINIGIT_OK);
    ASSERT_EQ(count, 2u);

    std::set<std::string> got(names, names + count);
    EXPECT_EQ(got.count("main"), 1u);
    EXPECT_EQ(got.count("dev"), 1u);

    for (size_t i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);

    minigit_repo_free(&repo);
}
