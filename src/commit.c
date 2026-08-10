#include "minigit/commit.h"
#include "minigit/common.h"
#include "minigit/sha1.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *arr;
  size_t count;
  size_t capacity;
} buffer;

__attribute__((format(printf, 2, 3))) 
int buf_append_printf(buffer *buf, const char *format, ...);
int buf_append_printf(buffer *buf, const char *format, ...) {
  if (!buf || !format)
    return -1;

  va_list args;
  va_start(args, format);

  int len = vsnprintf(NULL, 0, format, args);
  va_end(args);
  if (len < 0)
    return -1;

  size_t need = buf->count + (size_t)len + 1;
  if (need > buf->capacity) {
    if (buf->capacity == 0)
      buf->capacity = 256;
    while (buf->capacity < need)
      buf->capacity *= 2;
    buf->arr = realloc(buf->arr, sizeof(*buf->arr) * buf->capacity);
  }

  va_start(args, format);
  vsnprintf(buf->arr + buf->count, (size_t)len + 1, format, args);
  va_end(args);

  buf->count += (size_t)len;
  return 0;
}

#define buf_append_printf(buf, ...) buf_append_printf(&(buf), __VA_ARGS__)

typedef struct {
  minigit_oid *items;
  size_t count;
  size_t capacity;
} dynamic_arr;
#define da_append(xs, x)                                                       \
  do {                                                                         \
    if (xs.count >= xs.capacity) {                                             \
      if (xs.capacity == 0)                                                    \
        xs.capacity = 4;                                                       \
      else                                                                     \
        xs.capacity *= 2;                                                      \
      xs.items = realloc(xs.items, sizeof(*xs.items) * xs.capacity);           \
    }                                                                          \
    xs.items[xs.count++] = x;                                                  \
  } while (0)

/* TODO(你来实现)：见 commit.h 顶部的格式说明 */

int minigit_commit_serialize(const minigit_commit *commit,
                             unsigned char **out_data, size_t *out_size) {
  buffer buf = {};

  // tree line
  char tree_oid_hex[MINIGIT_OID_HEXSZ + 1];
  minigit_oid_to_hex(&commit->tree, tree_oid_hex);
  buf_append_printf(buf, "tree %s\n", tree_oid_hex);

  // parent lines
  for (size_t i = 0; i < commit->parent_count; i++) {
    minigit_oid cur_parent = commit->parents[i];
    char cur_parent_tree_oid_hex[MINIGIT_OID_HEXSZ + 1];
    minigit_oid_to_hex(&cur_parent, cur_parent_tree_oid_hex);
    buf_append_printf(buf, "parent %s\n", cur_parent_tree_oid_hex);
  }

  // author line
  buf_append_printf(buf, "author %s\n", commit->author);
  // committer line
  buf_append_printf(buf, "committer %s\n", commit->committer);
  // empty line
  buf_append_printf(buf, "\n");
  // message
  buf_append_printf(buf, "%s", commit->message);

  *out_data = (unsigned char *)buf.arr;
  *out_size = buf.count;
  return MINIGIT_OK;
}

int minigit_commit_parse(const unsigned char *data, size_t size,
                         minigit_commit *out_commit) {
  size_t idx = 0;
  // FIXME: how to check the border effeciently?

  // looking for tree
  while (data[idx] != ' ')
    idx++; // skip 'tree'
  char tree_str[5];
  strncpy(tree_str, (const char *)data, 4);
  tree_str[4] = '\0';
  if (strcmp(tree_str, "tree") != 0)
    return MINIGIT_ERR_INVALID;
  idx++; // skip  ' '
  char tree_oid_hex[MINIGIT_OID_HEXSZ + 1];
  strncpy(tree_oid_hex, (const char *)data + idx, MINIGIT_OID_HEXSZ);
  tree_oid_hex[MINIGIT_OID_HEXSZ] = '\0';
  minigit_oid_from_hex(tree_oid_hex, &out_commit->tree);
  idx += MINIGIT_OID_HEXSZ; // skip oid hex
  idx++;                    // skip '\n'

  // looking for parents
  dynamic_arr parents = {};
  for (;;) {
    char line_title[7]; // strlen("parent") + 1
    strncpy(line_title, (const char *)data + idx, 6);
    line_title[6] = '\0';
    if (strcmp(line_title, "parent") != 0)
      break;
    idx += 7; // skip 'parent '

    char parent_oid_hex[MINIGIT_OID_HEXSZ + 1];
    strncpy(parent_oid_hex, (const char *)data + idx, MINIGIT_OID_HEXSZ);
    parent_oid_hex[MINIGIT_OID_HEXSZ] = '\0';
    minigit_oid parent = {};
    minigit_oid_from_hex(parent_oid_hex, &parent);
    da_append(parents, parent);
    idx += MINIGIT_OID_HEXSZ; // skip oid hex
    idx++;                    // skip '\n'
  }
  out_commit->parents = parents.items;
  out_commit->parent_count = parents.count;

  // looking for author
  char author_title[7]; // strlen("author") + 1
  strncpy(author_title, (const char *)data + idx, 6);
  author_title[6] = '\0';
  if (strcmp(author_title, "author") != 0)
    return MINIGIT_ERR_INVALID;
  idx += 7; // skip 'author '

  size_t author_start_idx = idx;
  while (data[idx] != '\n')
    idx++;
  size_t author_msg_len = idx - author_start_idx + 1;
  out_commit->author = malloc(author_msg_len);
  strncpy(out_commit->author, (const char *)data + author_start_idx,
          author_msg_len - 1);
  out_commit->author[author_msg_len - 1] = '\0';
  idx++; // skip '\n'

  // looking for committer
  char committer_title[10]; // strlen("committer") + 1
  strncpy(committer_title, (const char *)data + idx, 9);
  committer_title[9] = '\0';
  if (strcmp(committer_title, "committer") != 0)
    return MINIGIT_ERR_INVALID;
  idx += 10;

  size_t committer_start_idx = idx;
  while (data[idx] != '\n')
    idx++;
  size_t committer_msg_len = idx - committer_start_idx + 1;
  out_commit->committer = malloc(committer_msg_len);
  strncpy(out_commit->committer, (const char *)data + committer_start_idx,
          committer_msg_len - 1);
  out_commit->committer[committer_msg_len - 1] = '\0';
  idx++;
  idx++;

  // commit msg
  size_t msg_len = size - idx;
  out_commit->message = malloc(msg_len + 1);
  strncpy(out_commit->message, (const char*)data + idx, msg_len);
  out_commit->message[msg_len] = '\0';

  return MINIGIT_OK;
}

void minigit_commit_free(minigit_commit *commit) {
  if (commit == NULL) {
    return;
  }
  free(commit->parents);
  free(commit->author);
  free(commit->committer);
  free(commit->message);
  memset(commit, 0, sizeof(*commit));
}
