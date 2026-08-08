/*
 * test_repo.cpp — 对应已完整实现的 src/repo.c（仓库发现），从一开始就
 * 应该全部通过。
 */
#include <gtest/gtest.h>

#include "test_helpers.h"

extern "C" {
#include "minigit/fsutil.h"
#include "minigit/repo.h"
}

#include <cstdlib>
#include <cstring>
#include <string>

using minigit_test::TempDir;

TEST(Repo, DiscoverFindsGitDirFromNestedSubdirectory) {
    TempDir tmp;
    std::string gitdir = (tmp.path() / ".git").string();
    ASSERT_EQ(minigit_mkdir_p(gitdir.c_str()), MINIGIT_OK);

    std::string nested = (tmp.path() / "a" / "b" / "c").string();
    ASSERT_EQ(minigit_mkdir_p(nested.c_str()), MINIGIT_OK);

    minigit_repo repo;
    ASSERT_EQ(minigit_repo_discover(nested.c_str(), &repo), MINIGIT_OK);

    EXPECT_TRUE(std::filesystem::equivalent(repo.git_dir, gitdir));
    EXPECT_TRUE(std::filesystem::equivalent(repo.work_tree, tmp.path()));

    minigit_repo_free(&repo);
}

TEST(Repo, DiscoverFailsWithoutAnyGitDir) {
    TempDir tmp; // 全新的空临时目录，没有 .git
    minigit_repo repo;
    EXPECT_EQ(minigit_repo_discover(tmp.path().string().c_str(), &repo), MINIGIT_ERR_NOT_A_REPO);
}

TEST(Repo, GitPathAndWorkPathJoinCorrectly) {
    TempDir tmp;
    minigit_repo repo;
    repo.git_dir = strdup((tmp.path() / ".git").string().c_str());
    repo.work_tree = strdup(tmp.path().string().c_str());

    char *p1 = minigit_repo_git_path(&repo, "refs/heads/main");
    EXPECT_EQ(std::string(p1), (tmp.path() / ".git" / "refs" / "heads" / "main").string());
    free(p1);

    char *p2 = minigit_repo_work_path(&repo, "src/main.c");
    EXPECT_EQ(std::string(p2), (tmp.path() / "src" / "main.c").string());
    free(p2);

    minigit_repo_free(&repo);
}
