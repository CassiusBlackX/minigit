#include "minigit/index.h"
#include "minigit/common.h"
#include "minigit/dynamic_arr.h"
#include "minigit/fsutil.h"
#include "minigit/object.h"
#include "minigit/sha1.h"
#include "minigit/tree.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const char *mode_to_str(unsigned int mode) {
  switch (mode) {
  case MINIGIT_MODE_BLOB:
    return "100644";
  case MINIGIT_MODE_EXEC:
    return "100755";
  case MINIGIT_MODE_TREE:
    return "40000";
  default:
    assert(0);
  }
}

static unsigned int str_to_mode(const char *mode_str) {
  if (strcmp("100644", mode_str) == 0)
    return MINIGIT_MODE_BLOB;
  if (strcmp("100755", mode_str) == 0)
    return MINIGIT_MODE_EXEC;
  if (strcmp("40000", mode_str) == 0)
    return MINIGIT_MODE_TREE;
  return 0;
}

#define index_da_append(mindex, item)                                          \
  do {                                                                         \
    if (mindex->count >= mindex->capacity) {                                   \
      if (mindex->capacity == 0)                                               \
        mindex->capacity = 64;                                                 \
      else                                                                     \
        mindex->capacity *= 2;                                                 \
      mindex->entries = realloc(mindex->entries,                               \
                                sizeof(*mindex->entries) * mindex->capacity);  \
    }                                                                          \
    mindex->entries[mindex->count++] = item;                                   \
  } while (0)

#define index_da_insert(mindex, item, insert_pos)                              \
  do {                                                                         \
    if (mindex->count >= mindex->capacity) {                                   \
      if (mindex->capacity == 0)                                               \
        mindex->capacity = 64;                                                 \
      else                                                                     \
        mindex->capacity *= 2;                                                 \
      mindex->entries = realloc(mindex->entries,                               \
                                sizeof(*mindex->entries) * mindex->capacity);  \
    }                                                                          \
    size_t move_count = mindex->count - insert_pos;                            \
    memmove(&mindex->entries[insert_pos + 1], &mindex->entries[insert_pos],    \
            move_count * sizeof(*mindex->entries));                            \
    mindex->entries[insert_pos] = item;                                        \
    mindex->count++;                                                           \
  } while (0)

static int entry_cmp(const void *a, const void *b) {
  const minigit_index_entry *ea = (const minigit_index_entry *)a;
  const minigit_index_entry *eb = (const minigit_index_entry *)b;
  return strcmp(ea->path, eb->path);
}

typedef struct {
  minigit_tree_entry *items;
  size_t count;
  size_t capacity;
} tree_entries;
#define tree_entries_append(da_arr, item)                                      \
  do {                                                                         \
    if (da_arr.count >= da_arr.capacity) {                                     \
      if (da_arr.capacity == 0)                                                \
        da_arr.capacity = 256;                                                 \
      else                                                                     \
        da_arr.capacity *= 2;                                                  \
      da_arr.items =                                                           \
          realloc(da_arr.items, da_arr.capacity * sizeof(*da_arr.items));      \
    }                                                                          \
    da_arr.items[da_arr.count++] = item;                                       \
  } while (0)

/* TODO(你来实现)：见 index.h 顶部的格式说明与 write_tree 的算法提示 */

int minigit_index_load(const minigit_repo *repo, minigit_index *out_index) {
  int status = MINIGIT_OK;
  const char *minigit_index_file_path =
      minigit_path_join(repo->git_dir, "index");
  unsigned char *index_file_content = NULL;

  // if 'index' does not exist, return MINIGIT_OK with count=0
  if (!minigit_path_exists(minigit_index_file_path)) {
    out_index->count = 0;
    status = MINIGIT_OK;
    goto cleanup;
  }

  // open index file
  size_t index_file_size = 0;
  status = minigit_read_file(minigit_index_file_path, &index_file_content,
                             &index_file_size);
  if (status != MINIGIT_OK)
    goto cleanup;

  // parse index file
  for (size_t i = 0; i < index_file_size;) {
    minigit_index_entry entry = {};

    // parse mode
    char mode_str[7] = {};
    size_t j = 0;
    while (index_file_content[i] != ' ' && j < 7) {
      mode_str[j++] = index_file_content[i++];
    }
    if (index_file_content[i] != ' ') { // mode_str too long
      status = MINIGIT_ERR_INVALID;
      goto cleanup;
    }
    mode_str[j] = '\0';
    i++; // skip ' '
    entry.mode = str_to_mode(mode_str);
    if (entry.mode == 0) {
      status = MINIGIT_ERR_INVALID;
      goto cleanup;
    }

    // parse oid
    char oid_hex[MINIGIT_OID_HEXSZ + 1] = {};
    j = 0;
    while (index_file_content[i] != ' ' && j <= MINIGIT_OID_HEXSZ) {
      oid_hex[j++] = index_file_content[i++];
    }
    if (index_file_content[i] != ' ') { // oid_hex too long
      status = MINIGIT_ERR_INVALID;
      goto cleanup;
    }
    oid_hex[j] = '\0';
    i++; // skip ' '
    status = minigit_oid_from_hex(oid_hex, &entry.oid);
    if (status != MINIGIT_OK)
      goto cleanup;

    // parse path
    j = i; // start index for path
    while (index_file_content[i] != '\n')
      i++;
    size_t path_len = i - j + 1;
    entry.path = malloc(path_len);
    memcpy(entry.path, (char *)index_file_content + j, path_len - 1);
    entry.path[path_len - 1] = '\0';
    i++; // skip '\n'

    index_da_append(out_index, entry);
  }
  qsort(out_index->entries, out_index->count, sizeof(*out_index->entries),
        entry_cmp);

cleanup:
  if (minigit_index_file_path)
    free((void *)minigit_index_file_path);
  if (index_file_content)
    free(index_file_content);
  return status;
}

int minigit_index_save(const minigit_repo *repo, const minigit_index *index) {
  const char *minigit_index_file_path =
      minigit_path_join(repo->git_dir, "index");
  buffer buf = {};

  for (size_t i = 0; i < index->count; i++) {
    minigit_index_entry entry = index->entries[i];
    const char *mode_str = mode_to_str(entry.mode);
    char oid_hex[MINIGIT_OID_HEXSZ + 1] = {};
    minigit_oid_to_hex(&entry.oid, oid_hex);
    buf_append_printf(buf, "%s %s %s\n", mode_str, oid_hex, entry.path);
  }

  int status = minigit_write_file(minigit_index_file_path, buf.arr, buf.count);
  free(buf.arr);
  return status;
}

int minigit_index_add(minigit_index *index, const char *path, unsigned int mode,
                      const minigit_oid *oid) {
  for (size_t i = 0; i < index->count; i++) {
    minigit_index_entry entry = index->entries[i];
    // path exists, only update mode/oid
    if (strcmp(entry.path, path) == 0) {
      index->entries[i].mode = mode;
      memcpy(&index->entries[i].oid, oid, sizeof(*oid));
      return MINIGIT_OK;
    }

    if (strcmp(entry.path, path) > 0) {
      // we should insert entry to the previous
      minigit_index_entry new_entry = {.mode = mode};
      memcpy(&new_entry.oid, oid, sizeof(*oid));
      size_t path_len = strlen(path);
      new_entry.path = malloc(path_len + 1);
      memcpy(new_entry.path, path, path_len);
      new_entry.path[path_len] = '\0';

      index_da_insert(index, new_entry, i);
      return MINIGIT_OK;
    }
  }

  // if new_entry is the larges one
  minigit_index_entry new_entry = {.mode = mode};
  memcpy(&new_entry.oid, oid, sizeof(*oid));
  size_t path_len = strlen(path);
  new_entry.path = malloc(path_len + 1);
  memcpy(new_entry.path, path, path_len);
  new_entry.path[path_len] = '\0';
  index_da_append(index, new_entry);
  return MINIGIT_OK;
}

int minigit_index_remove(minigit_index *index, const char *path) {
  for (size_t i = 0; i < index->count; i++) {
    minigit_index_entry cur_entry = index->entries[i];
    if (strcmp(cur_entry.path, path) == 0) {
      size_t remove_count = index->count - i - 1;
      memmove(&index->entries[i], &index->entries[i + 1],
              remove_count * sizeof(*index->entries));
      index->count--;
      return MINIGIT_OK;
    }
  }
  return MINIGIT_ERR_NOT_FOUND;
}

minigit_index_entry *minigit_index_find(const minigit_index *index,
                                        const char *path) {
  for (size_t i = 0; i < index->count; i++) {
    minigit_index_entry entry = index->entries[i];
    if (strcmp(entry.path, path) == 0)
      return index->entries + i;
  }
  return NULL;
}

static size_t group_end(const minigit_index_entry *entries, size_t count,
                        size_t prefix_len) {
  char *first_slash = strchr(entries[0].path + prefix_len, '/');
  if (!first_slash)
    return 0; // already file, not dir
  char *new_prefix = malloc(first_slash - entries[0].path - prefix_len + 1);
  memcpy(new_prefix, entries[0].path + prefix_len,
          first_slash - entries[0].path - prefix_len);
  new_prefix[first_slash - entries[0].path - prefix_len] = '\0';
  size_t return_val = 0;
  for (size_t i = 1; i < count; i++) {
    char *new_slash = strchr(entries[i].path + prefix_len, '/');
    if (!new_slash) {
      // no more same prefix, entries[i] is anothor new file
      return_val = i;
      goto cleanup;
    }

    char *cur_prefix = malloc(new_slash - entries[i].path - prefix_len + 1);
    memcpy(cur_prefix, entries[i].path + prefix_len,
            new_slash - entries[i].path - prefix_len);
    cur_prefix[new_slash - entries[i].path - prefix_len] = '\0';
    if (strcmp(new_prefix, cur_prefix) != 0) {
      // found group end, break
      return_val = i;
      free(cur_prefix);
      goto cleanup;
    }
    free(cur_prefix);
  }
  return_val = count;

cleanup:
  free(new_prefix);
  return return_val;
}

static int build_tree(const minigit_repo *repo,
                      const minigit_index_entry *entries, size_t count,
                      size_t prefix_len, minigit_oid *out_oid) {
  int status = MINIGIT_OK;
  unsigned char *tree_data = NULL;
  tree_entries tents = {};
  for (size_t i = 0; i < count;) {
    size_t g = group_end(entries + i, count - i, prefix_len);
    minigit_index_entry ient = entries[i];
    if (g == 0) {  // no subdir, is a file
      minigit_tree_entry tent = {
        .mode = ient.mode,
        .oid = ient.oid,
      };
      tent.name = malloc(strlen(ient.path + prefix_len) + 1);
      memcpy(tent.name, ient.path +prefix_len, strlen(ient.path + prefix_len));
      tent.name[strlen(ient.path + prefix_len)] = '\0';
      tree_entries_append(tents, tent);
      i++;
    } else {  // subdir, recursive build_tree
      char *first_slash = strchr(ient.path + prefix_len, '/');
      minigit_oid sub_oid = {};
      status = build_tree(repo, entries + i, g, first_slash - ient.path + 1, &sub_oid);
      if (status != MINIGIT_OK) goto cleanup;
      minigit_tree_entry tent = {
        .mode = MINIGIT_MODE_TREE,
        .oid = sub_oid,
      };
      tent.name = malloc(first_slash - ient.path - prefix_len + 1);
      memcpy(tent.name, ient.path + prefix_len, first_slash - ient.path - prefix_len);
      tent.name[first_slash - ient.path - prefix_len] = '\0';
      tree_entries_append(tents, tent);
      i += g;
    }
  }

  minigit_tree tree = {
      .entries = tents.items,
      .count = tents.count,
  };
  size_t tree_size = 0;
  status = minigit_tree_serialize(&tree, &tree_data, &tree_size);
  if (status != MINIGIT_OK)
    goto cleanup;
  status = minigit_object_write(repo, MINIGIT_OBJ_TREE, tree_data, tree_size,
                                out_oid);

cleanup:
  free(tree_data);
  minigit_tree_free(&tree);
  return status;
}

int minigit_index_write_tree(const minigit_repo *repo,
                             const minigit_index *index,
                             minigit_oid *out_tree_oid) {
  return build_tree(repo, index->entries, index->count, 0, out_tree_oid);
}

void minigit_index_free(minigit_index *index) {
  if (index == NULL) {
    return;
  }
  for (size_t i = 0; i < index->count; i++) {
    free(index->entries[i].path);
  }
  free(index->entries);
  index->entries = NULL;
  index->count = 0;
  index->capacity = 0;
}
