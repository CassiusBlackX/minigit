# minigit

用 C 从零实现一个只包含核心概念的 git：对象存储、tree/commit 模型、
暂存区、引用系统、基础 diff，以及 add/commit/log/status/diff/branch/
checkout/merge 这些常用命令背后的原理。

- 项目骨架、构建系统、测试框架已经搭好。
- 压缩（zlib）、哈希（OpenSSL SHA-1）、文件系统操作这些和"理解 git 原
  理"无关的部分已经帮你实现好。
- 对象模型、暂存区、引用系统、diff 算法、各个子命令的具体实现，都以
  `TODO(你来实现)` 的形式留空，函数签名和详细的格式说明已经写在对应的
  头文件里。

**从这里开始：[TUTORIAL.md](TUTORIAL.md)**

## 快速开始

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

第一次构建会通过网络拉取 GoogleTest（`FetchContent`），需要联网。
