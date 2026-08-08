/*
 * test_helpers.h — 测试专用的小工具（临时目录、跑外部命令），和
 * minigit 本身的实现无关，纯粹是测试基础设施。
 */
#pragma once

#include <array>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <utility>
#include <vector>

namespace minigit_test {

// RAII 临时目录：构造时创建一个独占的空目录，析构时递归删除。
// 每个需要磁盘状态的测试都应该用它，保证测试之间互不干扰、可以并发跑。
class TempDir {
public:
    TempDir() {
        std::string tmpl = (std::filesystem::temp_directory_path() / "minigit_test_XXXXXX").string();
        std::vector<char> buf(tmpl.begin(), tmpl.end());
        buf.push_back('\0');
        if (mkdtemp(buf.data()) == nullptr) {
            throw std::runtime_error("mkdtemp failed");
        }
        path_ = buf.data();
    }

    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path_, ec);
    }

    TempDir(const TempDir &) = delete;
    TempDir &operator=(const TempDir &) = delete;

    const std::filesystem::path &path() const { return path_; }

private:
    std::filesystem::path path_;
};

// 跑一条 shell 命令，返回 {退出码, 合并后的 stdout+stderr}
inline std::pair<int, std::string> run_command(const std::string &cmd) {
    std::array<char, 4096> buffer{};
    std::string result;

    FILE *pipe = popen((cmd + " 2>&1").c_str(), "r");
    if (pipe == nullptr) {
        throw std::runtime_error("popen failed for: " + cmd);
    }

    size_t n;
    while ((n = fread(buffer.data(), 1, buffer.size(), pipe)) > 0) {
        result.append(buffer.data(), n);
    }

    int status = pclose(pipe);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return {exit_code, result};
}

// 编译期由 tests/CMakeLists.txt 通过 target_compile_definitions 注入
// 构建出来的 minigit 可执行文件的绝对路径。
inline std::string minigit_binary() {
#ifdef MINIGIT_BINARY_PATH
    return MINIGIT_BINARY_PATH;
#else
    return "minigit";
#endif
}

} // namespace minigit_test
