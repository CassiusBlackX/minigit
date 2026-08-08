include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
# Windows 相关设置，Linux 上无影响，保留是为了将来跨平台不用改
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(minigit_tests
  test_sha1.cpp
  test_compress.cpp
  test_fsutil.cpp
  test_repo.cpp
  test_object_blob.cpp
  test_object_tree.cpp
  test_object_commit.cpp
  test_index.cpp
  test_refs.cpp
  test_diff.cpp
  test_cli_workflow.cpp
)

target_link_libraries(minigit_tests PRIVATE minigit_core GTest::gtest_main)

# 集成测试需要能找到构建出来的 minigit 可执行文件的路径，以及需要它先被构建好
target_compile_definitions(minigit_tests PRIVATE
  MINIGIT_BINARY_PATH="$<TARGET_FILE:minigit>"
)
add_dependencies(minigit_tests minigit)

include(GoogleTest)
gtest_discover_tests(minigit_tests)
