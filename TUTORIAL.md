# minigit 实现教程

这份教程按依赖关系把整个项目拆成 9 个阶段。每个阶段只依赖前面已经完成
的阶段，做完一个阶段、跑对应的测试全部变绿，再进入下一个——不要跳着做，
后面的阶段（尤其是 index 的 write_tree 和各个 porcelain 命令）会直接
调用前面阶段的函数，前面没打好基础后面会寸步难行。

## 0. 项目结构

```
include/minigit/    公共头文件（API + 详细注释），你主要在这里读"要做
                     什么、为什么这么设计"
src/                核心模块的实现（.c），TODO 留空的地方在这里填
src/commands/       每个 CLI 子命令一个文件
tests/               GoogleTest 测试，按模块一一对应
```

对照关系：
| 头文件 | 实现文件 | 测试文件 |
|---|---|---|
| `sha1.h` | `sha1.c`（已实现） | `test_sha1.cpp` |
| `compress.h` | `compress.c`（已实现） | `test_compress.cpp` |
| `fsutil.h` | `fsutil.c`（已实现） | `test_fsutil.cpp` |
| `repo.h` | `repo.c`（已实现） | `test_repo.cpp` |
| `object.h` | `object.c`（TODO） | `test_object_blob.cpp` |
| `tree.h` | `tree.c`（TODO） | `test_object_tree.cpp` |
| `commit.h` | `commit.c`（TODO） | `test_object_commit.cpp` |
| `index.h` | `index.c`（TODO） | `test_index.cpp` |
| `refs.h` | `refs.c`（TODO） | `test_refs.cpp` |
| `diff.h` | `diff.c`（TODO） | `test_diff.cpp` |
| `commands.h` | `commands/*.c`（TODO，12 个文件） | `test_cli_workflow.cpp` |

## 1. 构建与测试

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

- 第一次 `cmake -B build` 会通过网络拉取 GoogleTest 源码（`FetchContent`），
  之后不再需要联网。
- Debug 构建默认开启 AddressSanitizer + UndefinedBehaviorSanitizer
  （见根 `CMakeLists.txt` 的 `MINIGIT_ENABLE_SANITIZERS`），指针越界、
  内存泄漏、use-after-free 这些问题会在测试运行时直接崩溃并打印出错误
  位置，比事后 gdb 排查快得多，出现 Sanitizer 报错优先解决它。
- 只跑某个模块的测试，用 `ctest` 的 `-R`（正则匹配测试名）：
  ```sh
  ctest --test-dir build -R Sha1        # 阶段 0 之前的 chore 测试
  ctest --test-dir build -R ObjectBlob  # 阶段 2
  ctest --test-dir build -R Index       # 阶段 5
  ctest --test-dir build -R CliWorkflow # 最终集成测试
  ```
  也可以直接跑测试可执行文件本身，用 GoogleTest 原生的 filter，能看到
  更详细的单条用例输出：
  ```sh
  ./build/tests/minigit_tests --gtest_filter='ObjectBlob.*'
  ```

## 2. 阶段路线图

### 阶段 0（已完成，直接确认）：基础设施

`sha1.c` / `compress.c` / `fsutil.c` / `repo.c` 已经帮你实现好。先跑一遍：

```sh
ctest --test-dir build -R 'Sha1|Compress|FsUtil|Repo'
```

应该全部通过。如果不通过，大概率是环境问题（OpenSSL/zlib 版本、权限），
不是代码逻辑问题，先解决这个再继续。

### 阶段 1：blob 对象 —— `object.c`

读 `include/minigit/object.h` 顶部的格式说明（这是全项目最重要的一段
注释，后面 tree/commit 都建立在这个格式之上）。实现顺序建议：
`minigit_object_hash` → `minigit_object_write` → `minigit_object_read`。

```sh
ctest --test-dir build -R ObjectBlob
```

其中 `ObjectBlob.HashMatchesRealGitHashObject` 会拿系统安装的真实 git
做交叉验证——这条测试通过，就说明你对 git 对象格式的理解是完全正确的，
是这个阶段最有信心的里程碑。

### 阶段 2：tree 对象 —— `tree.c`

读 `include/minigit/tree.h`，注意 mode 的 ASCII 表示不做零填充这个坑。

```sh
ctest --test-dir build -R Tree
```

### 阶段 3：commit 对象 —— `commit.c`

读 `include/minigit/commit.h`，是纯文本格式，比 tree 简单。

```sh
ctest --test-dir build -R Commit
```

### 阶段 4：暂存区 —— `index.c`

读 `include/minigit/index.h`。先实现 `load`/`save`/`add`/`remove`/`find`
（都是常规的数组增删查 + 文本文件读写），最后实现 `write_tree`——这是
全项目算法上最有意思的一个函数，头文件里给了详细的分组递归思路。

```sh
ctest --test-dir build -R Index
```

`Index.WriteTreeBuildsNestedStructure` 依赖阶段 1、2 已经完成（它会真的
调用 `minigit_object_write`/`minigit_tree_parse`），如果这条测试失败，
先确认阶段 1/2 的测试是不是全绿。

### 阶段 5：引用系统 —— `refs.c`

读 `include/minigit/refs.h`，理解 HEAD 的两种状态（符号引用 / 分离头
指针）。

```sh
ctest --test-dir build -R Refs
```

### 阶段 6：行级 diff —— `diff.c`

读 `include/minigit/diff.h` 里的 LCS 算法说明（标准动态规划 + 回溯）。
这个模块和其他模块解耦，可以插到阶段 1-5 之间任意位置做，不影响别的
依赖关系。

```sh
ctest --test-dir build -R Diff
```

### 阶段 7：CLI 子命令 —— `src/commands/*.c`

到这里，所有底层模块都完成了，子命令基本就是"按顺序调用它们"。建议
实现顺序（越靠前越简单、越能验证前面模块是否正确）：

1. `init.c` —— 不依赖任何其他 TODO 模块，纯粹是创建目录结构
2. `hash_object.c` / `cat_file.c` —— 直接包装 object.c，是验证阶段 1
   最直接的方式
3. `write_tree.c` —— 包装 index.c 的 write_tree
4. `commit_tree.c` —— 包装 commit.c
5. `add.c` —— 包装 object.c + index.c
6. `commit.c` —— 把 index/refs/commit 串起来，这是"提交"这个动作第一
   次完整跑通
7. `log.c` —— 包装 refs + commit，验证提交历史链是否正确
8. `branch.c` —— 包装 refs
9. `checkout.c` —— 依赖 tree + refs + index，是仅次于 write_tree 的
   第二个复杂点
10. `status.c` / `diff_cmd.c` —— 需要同时对比三份状态，建议放最后
11. `merge.c` —— 选做，fast-forward 做完就已经达标

每实现完一两个命令，可以手动跑一遍体验效果：

```sh
cmake --build build -j
cd /tmp && rm -rf demo && mkdir demo && cd demo
/path/to/build/minigit init
echo hello > a.txt
/path/to/build/minigit add a.txt
/path/to/build/minigit commit -m "first commit"
/path/to/build/minigit log
```

### 阶段 8：集成测试 —— `test_cli_workflow.cpp`

```sh
ctest --test-dir build -R CliWorkflow
```

`CliWorkflow.RootTreeHashMatchesRealGit` 是整个项目的最终验收标准：让
minigit 和真实 git 对同一份工作区内容独立计算 tree 哈希，两者完全相等，
就证明你的实现从 blob 格式、tree 格式到暂存区折叠算法，每一层都和真实
git 二进制兼容。

## 3. 遇到"哈希对不上真实 git"时怎么排查

1. 先怀疑 header：是不是拼成了 `"blob <size>"` 却忘了拼 NUL 字节，或者
   size 前面多/少了空格。
2. 再怀疑哈希范围：是不是只对 content 算了哈希，漏了 header（这是最常
   见的错误）。
3. 用 `git cat-file -p <你算出来的hash>`（如果这个对象真的被写进了某个
   git 仓库的 `.git/objects` 里）反向验证——如果真实 git 都不认识这个
   哈希，先确认写盘路径、zlib 压缩是否正确。
4. tree 对象排序问题：确认没有把目录当文件排序（见 `tree.h` 里的排序
   说明），以及 mode 的 ASCII 是否漏了零填充规则（是 "40000" 不是
   "040000"）。

## 4. 可选的进阶方向（做完以上全部之后）

- `merge.c` 的完整三路合并（目前的 TODO 注释里给了思路）
- `.gitignore` 支持
- `checkout` 时清理"旧分支有、新分支没有"的文件
- packfile / delta 压缩（这是难度台阶跳得最大的一块，建议只在你想更
  深入理解 git 网络传输和存储优化时再做）
