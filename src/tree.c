#include "minigit/tree.h"
#include "minigit/common.h"
#include "minigit/fsutil.h"

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
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

static int entry_cmp(const void *a, const void *b) {
  const minigit_tree_entry *ea = (const minigit_tree_entry *)a;
  const minigit_tree_entry *eb = (const minigit_tree_entry *)b;
  int ea_is_dir = minigit_path_is_dir(ea->name);
  int eb_is_dir = minigit_path_is_dir(eb->name);
  char *real_ea_path = NULL, *real_eb_path = NULL;
  if (ea_is_dir) {
    real_ea_path = malloc(strlen(ea->name) + 2);
    memcpy(real_ea_path, ea->name, strlen(ea->name) + 1);
    real_ea_path[strlen(ea->name)] = '/';
    real_ea_path[strlen(ea->name) + 1] = '\0';
  } else {
    real_ea_path = ea->name;
  }
  if (eb_is_dir) {
    real_eb_path = malloc(strlen(eb->name) + 2);
    memcpy(real_eb_path, eb->name, strlen(eb->name) + 1);
    real_eb_path[strlen(eb->name)] = '/';
    real_eb_path[strlen(eb->name) + 1] = '\0';
  } else {
    real_eb_path = eb->name;
  }

  int result = strcmp(real_ea_path, real_eb_path);
  if (ea_is_dir) free(real_ea_path);
  if (eb_is_dir) free(real_eb_path);
  return result;
}

typedef struct {
  minigit_tree_entry *items;
  size_t count;
  size_t capacity;
} dynamic_arr;
#define da_append(da_arr, item)                                                \
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

/* ---------------------------------------------------------------------
 * TODO(你来实现)
 *
 * serialize 提示：先算出总长度（每个 entry 的长度是
 * strlen(mode_str) + 1(空格) + strlen(name) + 1('\0') + 20），分配一次
 * 缓冲区，再逐条 entry 拷贝进去，避免反复 realloc。
 *
 * parse 提示：用一个游标 (cursor/offset) 从 data 开头开始扫描，每轮：
 *   1. 从当前位置找下一个空格，中间就是 mode 的 ASCII，用 strtoul(...,
 *      NULL, 8) 转成整数（注意进制是 8，因为 mode 是八进制表示）；
 *   2. 空格之后到下一个 '\0' 之间就是 name；
 *   3. '\0' 之后紧跟的 20 字节是 oid，直接 memcpy；
 *   4. 游标前进到这条 entry 结束的位置，继续下一轮，直到游标走到 size。
 * ------------------------------------------------------------------- */

int minigit_tree_serialize(const minigit_tree *tree, unsigned char **out_data,
                           size_t *out_size) {
  *out_size = 0;
  for (size_t i = 0; i < tree->count; i++) {
    minigit_tree_entry entry = tree->entries[i];
    const char *mode_str = mode_to_str(entry.mode);
    size_t entry_len = strlen(mode_str) + 1 + strlen(entry.name) + 1 + 20;
    *out_size += entry_len;
  }
  *out_data = malloc(*out_size);
  unsigned char *p = *out_data;
  for (size_t i = 0; i < tree->count; i++) {
    minigit_tree_entry entry = tree->entries[i];
    const char *mode_str = mode_to_str(entry.mode);
    int mode_name_len =
        snprintf((char *)p, strlen(mode_str) + 1 + strlen(entry.name) + 1,
                 "%s %s", mode_str, entry.name);
    p += mode_name_len;
    *p = '\0';
    p++;
    memcpy(p, entry.oid.id, 20);
    p += 20;
  }
  return MINIGIT_OK;
}

int minigit_tree_parse(const unsigned char *data, size_t size,
                       minigit_tree *out_tree) {
  enum tree_parse_status {
    LOOKING_FOR_MODE,
    LOOKING_FOR_NAME,
    LOOKING_FOR_OID,
  };
  dynamic_arr arr = {};
  enum tree_parse_status status = LOOKING_FOR_MODE;
  unsigned int cur_mode;
  char *cur_name = NULL;
  size_t idx = 0;
  while (idx < size) {
    switch (status) {
    case LOOKING_FOR_MODE: {
      char mode_str[7] = {};
      size_t j = 0;
      while (data[idx] != ' ') {
        mode_str[j++] = data[idx++];
        if (j >= 7)
          return MINIGIT_ERR_INVALID;
      }
      mode_str[j] = '\0';
      cur_mode = str_to_mode(mode_str);
      if (cur_mode == 0) {
        fprintf(stderr, "invalid mode_str: %s\n", mode_str);
        return MINIGIT_ERR_INVALID;
      }
      idx++;
      status = LOOKING_FOR_NAME;
      break;
    }
    case LOOKING_FOR_NAME: {
      size_t name_start_idx = idx;
      while (data[idx] != '\0')
        idx++;
      cur_name = malloc(idx - name_start_idx + 1);
      for (size_t i = 0; data[i + name_start_idx] != '\0'; i++) {
        cur_name[i] = data[i + name_start_idx];
      }
      cur_name[idx - name_start_idx] = '\0';
      idx++;
      status = LOOKING_FOR_OID;
      break;
    }
    case LOOKING_FOR_OID: {
      minigit_oid cur_oid;
      memcpy(cur_oid.id, data + idx, 20);
      minigit_tree_entry cur_entry = {
          .mode = cur_mode,
          .name = cur_name,
          .oid = cur_oid,
      };
      da_append(arr, cur_entry);
      idx += 20;
      status = LOOKING_FOR_MODE;
      break;
    }
    };
  }

  // sort
  qsort(arr.items, arr.count, sizeof(*arr.items), entry_cmp);

  out_tree->count = arr.count;
  out_tree->entries = arr.items;
  return MINIGIT_OK;
}

void minigit_tree_free(minigit_tree *tree) {
  if (tree == NULL) {
    return;
  }
  for (size_t i = 0; i < tree->count; i++) {
    free(tree->entries[i].name);
  }
  free(tree->entries);
  tree->entries = NULL;
  tree->count = 0;
}
