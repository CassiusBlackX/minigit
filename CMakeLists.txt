cmake_minimum_required(VERSION 3.20)
project(minigit C CXX)

# ---------------------------------------------------------------------------
# 全局设置
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)          # 仅测试代码（GoogleTest）用到 C++
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 生成 compile_commands.json，方便 clangd / VSCode 等编辑器做代码跳转和补全
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra)
# Debug 构建下开启 AddressSanitizer + UndefinedBehaviorSanitizer。
# 你会频繁操作裸指针、手写序列化/反序列化，ASan/UBSan 能在第一时间抓到
# 越界读写、use-after-free、整数溢出这些问题，比事后用 gdb 排查划算得多。
option(MINIGIT_ENABLE_SANITIZERS "Enable ASan/UBSan in Debug builds" ON)
if(MINIGIT_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  # 有些工具链（尤其是精简过的发行版镜像）没有装 sanitizer 运行时库，
  # -fsanitize 编译能过但链接会失败。这里先试链接一个最小程序，失败就
  # 静默跳过，避免在这类环境上把整个构建搞挂。
  include(CheckCSourceCompiles)
  set(CMAKE_REQUIRED_FLAGS "-fsanitize=address,undefined")
  set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address,undefined")
  check_c_source_compiles("int main(void){return 0;}" MINIGIT_SANITIZERS_LINK_OK)
  unset(CMAKE_REQUIRED_FLAGS)
  unset(CMAKE_REQUIRED_LIBRARIES)

  if(MINIGIT_SANITIZERS_LINK_OK)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
  else()
    message(WARNING "ASan/UBSan 运行时在当前工具链上不可用，已跳过 sanitizer；"
                     "不影响正常构建和测试。")
  endif()
endif()

# ---------------------------------------------------------------------------
# 第三方依赖：只用于"与 git 核心逻辑无关"的部分（压缩、哈希）
# ---------------------------------------------------------------------------
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

# ---------------------------------------------------------------------------
# minigit_core：核心库，被可执行文件和测试共用
# ---------------------------------------------------------------------------
add_library(minigit_core STATIC
  src/sha1.c
  src/compress.c
  src/fsutil.c
  src/dynamic_arr.c
  src/repo.c
  src/object.c
  src/tree.c
  src/commit.c
  src/index.c
  src/refs.c
  src/diff.c
  src/commands/common.c
  src/commands/init.c
  src/commands/hash_object.c
  src/commands/cat_file.c
  src/commands/write_tree.c
  src/commands/commit_tree.c
  src/commands/add.c
  src/commands/commit.c
  src/commands/log.c
  src/commands/status.c
  src/commands/diff_cmd.c
  src/commands/branch.c
  src/commands/checkout.c
  src/commands/merge.c
)
target_include_directories(minigit_core PUBLIC include)
target_link_libraries(minigit_core PUBLIC ZLIB::ZLIB OpenSSL::Crypto)

# ---------------------------------------------------------------------------
# minigit 可执行文件
# ---------------------------------------------------------------------------
add_executable(minigit src/main.c)
target_link_libraries(minigit PRIVATE minigit_core)

# ---------------------------------------------------------------------------
# 测试
# ---------------------------------------------------------------------------
option(MINIGIT_BUILD_TESTS "Build the GoogleTest-based test suite" ON)
if(MINIGIT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
