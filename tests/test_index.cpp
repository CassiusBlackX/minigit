/*
 * test_index.cpp — 阶段 4：暂存区的增删查、落盘、以及最核心的
 * write_tree（把扁平 index 折叠成嵌套 tree 对象树）。
 *
 * WriteTreeBuildsNestedStructure 这个用例同时依赖 object.c 和 tree.c
 * 已经实现——如果你是按 TUTORIAL.md 的顺序推进的，到这里时它们应该都
 * 已经通过了各自的测试。
 */
#include <gtest/gtest.h>

#include "test_helpers.h"

extern "C" {
#include "minigit/fsutil.h"
#include "minigit/index.h"
#include "minigit/object.h"
#include "minigit/sha1.h"
#include "minigit/tree.h"
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

TEST(Index, AddKeepsSortedOrderAndFind) {
    minigit_index index;
    memset(&index, 0, sizeof(index));

    minigit_oid oid_a, oid_b, oid_c;
    minigit_sha1_buffer("a", 1, &oid_a);
    minigit_sha1_buffer("b", 1, &oid_b);
    minigit_sha1_buffer("c", 1, &oid_c);

    ASSERT_EQ(minigit_index_add(&index, "b.txt", MINIGIT_MODE_BLOB, &oid_b), MINIGIT_OK);
    ASSERT_EQ(minigit_index_add(&index, "a.txt", MINIGIT_MODE_BLOB, &oid_a), MINIGIT_OK);
    ASSERT_EQ(minigit_index_add(&index, "c.txt", MINIGIT_MODE_BLOB, &oid_c), MINIGIT_OK);
    ASSERT_EQ(index.count, 3u);

    EXPECT_STREQ(index.entries[0].path, "a.txt");
    EXPECT_STREQ(index.entries[1].path, "b.txt");
    EXPECT_STREQ(index.entries[2].path, "c.txt");

    minigit_index_entry *found = minigit_index_find(&index, "b.txt");
    ASSERT_NE(found, nullptr);
    EXPECT_TRUE(minigit_oid_equal(&found->oid, &oid_b));

    EXPECT_EQ(minigit_index_find(&index, "missing.txt"), nullptr);

    minigit_index_free(&index);
}

TEST(Index, AddUpdatesExistingEntryInPlace) {
    minigit_index index;
    memset(&index, 0, sizeof(index));

    minigit_oid oid_v1, oid_v2;
    minigit_sha1_buffer("v1", 2, &oid_v1);
    minigit_sha1_buffer("v2", 2, &oid_v2);

    ASSERT_EQ(minigit_index_add(&index, "f.txt", MINIGIT_MODE_BLOB, &oid_v1), MINIGIT_OK);
    ASSERT_EQ(minigit_index_add(&index, "f.txt", MINIGIT_MODE_BLOB, &oid_v2), MINIGIT_OK);

    ASSERT_EQ(index.count, 1u) << "同一个 path 再次 add 应该更新而不是新增一条";
    EXPECT_TRUE(minigit_oid_equal(&index.entries[0].oid, &oid_v2));

    minigit_index_free(&index);
}

TEST(Index, RemoveMissingPathReturnsNotFound) {
    minigit_index index;
    memset(&index, 0, sizeof(index));
    EXPECT_EQ(minigit_index_remove(&index, "nope.txt"), MINIGIT_ERR_NOT_FOUND);
    minigit_index_free(&index);
}

TEST(Index, SaveLoadRoundTrip) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_index index;
    memset(&index, 0, sizeof(index));
    minigit_oid oid;
    minigit_sha1_buffer("content", 7, &oid);
    ASSERT_EQ(minigit_index_add(&index, "src/main.c", MINIGIT_MODE_BLOB, &oid), MINIGIT_OK);
    ASSERT_EQ(minigit_index_save(&repo, &index), MINIGIT_OK);
    minigit_index_free(&index);

    minigit_index loaded;
    memset(&loaded, 0, sizeof(loaded));
    ASSERT_EQ(minigit_index_load(&repo, &loaded), MINIGIT_OK);
    ASSERT_EQ(loaded.count, 1u);
    EXPECT_STREQ(loaded.entries[0].path, "src/main.c");
    EXPECT_TRUE(minigit_oid_equal(&loaded.entries[0].oid, &oid));

    minigit_index_free(&loaded);
    minigit_repo_free(&repo);
}

TEST(Index, LoadMissingIndexFileYieldsEmptyIndex) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_index index;
    memset(&index, 0, sizeof(index));
    ASSERT_EQ(minigit_index_load(&repo, &index), MINIGIT_OK);
    EXPECT_EQ(index.count, 0u);

    minigit_index_free(&index);
    minigit_repo_free(&repo);
}

TEST(Index, WriteTreeBuildsNestedStructure) {
    TempDir tmp;
    minigit_repo repo = make_repo(tmp);

    minigit_oid oid_root, oid_nested;
    ASSERT_EQ(minigit_object_write(&repo, MINIGIT_OBJ_BLOB, "root file", 9, &oid_root), MINIGIT_OK);
    ASSERT_EQ(minigit_object_write(&repo, MINIGIT_OBJ_BLOB, "nested file", 11, &oid_nested),
              MINIGIT_OK);

    minigit_index index;
    memset(&index, 0, sizeof(index));
    ASSERT_EQ(minigit_index_add(&index, "root.txt", MINIGIT_MODE_BLOB, &oid_root), MINIGIT_OK);
    ASSERT_EQ(minigit_index_add(&index, "dir/nested.txt", MINIGIT_MODE_BLOB, &oid_nested),
              MINIGIT_OK);

    minigit_oid tree_oid;
    ASSERT_EQ(minigit_index_write_tree(&repo, &index, &tree_oid), MINIGIT_OK);

    minigit_object root_obj;
    ASSERT_EQ(minigit_object_read(&repo, &tree_oid, &root_obj), MINIGIT_OK);
    EXPECT_EQ(root_obj.type, MINIGIT_OBJ_TREE);

    minigit_tree root_tree;
    ASSERT_EQ(minigit_tree_parse(root_obj.data, root_obj.size, &root_tree), MINIGIT_OK);
    ASSERT_EQ(root_tree.count, 2u) << "根 tree 应该有两个 entry：root.txt 这个文件 + dir 这个子目录";

    bool found_dir = false;
    for (size_t i = 0; i < root_tree.count; i++) {
        if (strcmp(root_tree.entries[i].name, "dir") == 0) {
            found_dir = true;
            EXPECT_EQ(root_tree.entries[i].mode, static_cast<unsigned>(MINIGIT_MODE_TREE));

            minigit_object sub_obj;
            ASSERT_EQ(minigit_object_read(&repo, &root_tree.entries[i].oid, &sub_obj), MINIGIT_OK);

            minigit_tree sub_tree;
            ASSERT_EQ(minigit_tree_parse(sub_obj.data, sub_obj.size, &sub_tree), MINIGIT_OK);
            ASSERT_EQ(sub_tree.count, 1u);
            EXPECT_STREQ(sub_tree.entries[0].name, "nested.txt");
            EXPECT_TRUE(minigit_oid_equal(&sub_tree.entries[0].oid, &oid_nested));

            minigit_tree_free(&sub_tree);
            minigit_object_free(&sub_obj);
        }
    }
    EXPECT_TRUE(found_dir);

    minigit_tree_free(&root_tree);
    minigit_object_free(&root_obj);
    minigit_index_free(&index);
    minigit_repo_free(&repo);
}
