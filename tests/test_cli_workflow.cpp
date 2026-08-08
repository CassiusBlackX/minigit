/*
 * test_cli_workflow.cpp — 端到端集成测试：直接运行编译出来的 minigit
 * 可执行文件，模拟真实用户的操作序列。
 *
 * 这是全项目的"压轴"测试——要让这里的用例通过，前面所有阶段（object /
 * tree / commit / index / refs）以及对应的 porcelain 命令都得先跑通。
 * RootTreeHashMatchesRealGit 这个用例还会拿真实 git 当 oracle 做交叉
 * 验证，是检验你整个实现是否和真实 git 二进制兼容的最终标准。
 */
#include <gtest/gtest.h>

#include "test_helpers.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

using minigit_test::minigit_binary;
using minigit_test::run_command;
using minigit_test::TempDir;

namespace {
std::string mg(const std::string &workdir, const std::string &args) {
    return "cd " + workdir + " && " + minigit_binary() + " " + args;
}

void write_text_file(const std::filesystem::path &path, const std::string &content) {
    std::ofstream out(path, std::ios::binary);
    out << content;
}
} // namespace

TEST(CliWorkflow, InitCreatesExpectedGitLayout) {
    TempDir tmp;
    auto [code, output] = run_command(mg(tmp.path().string(), "init"));
    ASSERT_EQ(code, 0) << output;

    EXPECT_TRUE(std::filesystem::is_directory(tmp.path() / ".git" / "objects"));
    EXPECT_TRUE(std::filesystem::is_directory(tmp.path() / ".git" / "refs" / "heads"));
    EXPECT_TRUE(std::filesystem::is_regular_file(tmp.path() / ".git" / "HEAD"));
}

TEST(CliWorkflow, AddCommitLogRoundTrip) {
    TempDir tmp;
    std::string workdir = tmp.path().string();
    ASSERT_EQ(run_command(mg(workdir, "init")).first, 0);

    write_text_file(tmp.path() / "hello.txt", "hello minigit\n");
    ASSERT_EQ(run_command(mg(workdir, "add hello.txt")).first, 0);

    auto [commit_code, commit_out] = run_command(mg(workdir, "commit -m \"first commit\""));
    ASSERT_EQ(commit_code, 0) << commit_out;

    auto [log_code, log_out] = run_command(mg(workdir, "log"));
    ASSERT_EQ(log_code, 0) << log_out;
    EXPECT_NE(log_out.find("first commit"), std::string::npos) << log_out;
}

TEST(CliWorkflow, SecondCommitHasFirstAsParent) {
    TempDir tmp;
    std::string workdir = tmp.path().string();
    ASSERT_EQ(run_command(mg(workdir, "init")).first, 0);

    write_text_file(tmp.path() / "f.txt", "v1\n");
    ASSERT_EQ(run_command(mg(workdir, "add f.txt")).first, 0);
    ASSERT_EQ(run_command(mg(workdir, "commit -m \"v1\"")).first, 0);

    write_text_file(tmp.path() / "f.txt", "v2\n");
    ASSERT_EQ(run_command(mg(workdir, "add f.txt")).first, 0);
    ASSERT_EQ(run_command(mg(workdir, "commit -m \"v2\"")).first, 0);

    auto [log_code, log_out] = run_command(mg(workdir, "log"));
    ASSERT_EQ(log_code, 0);
    EXPECT_NE(log_out.find("v1"), std::string::npos);
    EXPECT_NE(log_out.find("v2"), std::string::npos);
    // "v2" 的提交说明应该比 "v1" 更早出现在 log 输出里（log 是从新到旧打印的）
    EXPECT_LT(log_out.find("v2"), log_out.find("v1"));
}

TEST(CliWorkflow, RootTreeHashMatchesRealGit) {
    if (std::system("git --version > /dev/null 2>&1") != 0) {
        GTEST_SKIP() << "系统未安装 git，跳过这个 oracle 对照测试";
    }

    TempDir tmp;
    std::string workdir = tmp.path().string();
    ASSERT_EQ(run_command(mg(workdir, "init")).first, 0);
    write_text_file(tmp.path() / "a.txt", "content a\n");
    write_text_file(tmp.path() / "b.txt", "content b\n");
    ASSERT_EQ(run_command(mg(workdir, "add a.txt b.txt")).first, 0);

    auto [wt_code, wt_out] = run_command(mg(workdir, "write-tree"));
    ASSERT_EQ(wt_code, 0) << wt_out;
    while (!wt_out.empty() && (wt_out.back() == '\n' || wt_out.back() == '\r')) {
        wt_out.pop_back();
    }

    TempDir oracle;
    std::string odir = oracle.path().string();
    ASSERT_EQ(run_command("cd " + odir + " && git init -q -b main").first, 0);
    write_text_file(oracle.path() / "a.txt", "content a\n");
    write_text_file(oracle.path() / "b.txt", "content b\n");
    ASSERT_EQ(run_command("cd " + odir + " && git add a.txt b.txt").first, 0);

    auto [ow_code, ow_out] = run_command("cd " + odir + " && git write-tree");
    ASSERT_EQ(ow_code, 0) << ow_out;
    while (!ow_out.empty() && (ow_out.back() == '\n' || ow_out.back() == '\r')) {
        ow_out.pop_back();
    }

    EXPECT_EQ(wt_out, ow_out) << "minigit 和真实 git 对同样的工作区内容算出的根 tree 哈希应该完全一致";
}

TEST(CliWorkflow, BranchCreateAndCheckoutSwitchesCurrentBranch) {
    TempDir tmp;
    std::string workdir = tmp.path().string();
    ASSERT_EQ(run_command(mg(workdir, "init")).first, 0);

    write_text_file(tmp.path() / "f.txt", "v1\n");
    ASSERT_EQ(run_command(mg(workdir, "add f.txt")).first, 0);
    ASSERT_EQ(run_command(mg(workdir, "commit -m \"v1\"")).first, 0);

    ASSERT_EQ(run_command(mg(workdir, "branch feature")).first, 0);
    ASSERT_EQ(run_command(mg(workdir, "checkout feature")).first, 0);

    auto [branch_code, branch_out] = run_command(mg(workdir, "branch"));
    ASSERT_EQ(branch_code, 0) << branch_out;
    EXPECT_NE(branch_out.find("* feature"), std::string::npos) << branch_out;
}
