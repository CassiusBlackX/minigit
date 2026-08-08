#include "minigit/object.h"
#include "minigit/compress.h"
#include "minigit/fsutil.h"
#include "minigit/sha1.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* 这部分是纯字符串表格查找，已经帮你写好，不是本项目要学习的内容。 */
const char *minigit_obj_type_name(minigit_obj_type type) {
    switch (type) {
        case MINIGIT_OBJ_BLOB: return "blob";
        case MINIGIT_OBJ_TREE: return "tree";
        case MINIGIT_OBJ_COMMIT: return "commit";
    }
    return "unknown";
}

int minigit_obj_type_from_name(const char *name, minigit_obj_type *out_type) {
    if (strcmp(name, "blob") == 0) { *out_type = MINIGIT_OBJ_BLOB; return MINIGIT_OK; }
    if (strcmp(name, "tree") == 0) { *out_type = MINIGIT_OBJ_TREE; return MINIGIT_OK; }
    if (strcmp(name, "commit") == 0) { *out_type = MINIGIT_OBJ_COMMIT; return MINIGIT_OK; }
    return MINIGIT_ERR_INVALID;
}

/* ---------------------------------------------------------------------
 * TODO(你来实现)
 *
 * 提示：header 的构造可以用 snprintf(buf, sizeof(buf), "%s %zu",
 * minigit_obj_type_name(type), size) + 手动补一个 '\0'，注意 snprintf
 * 不会帮你把 '\0' 算进"写入了多少字节"里，你需要自己在 store 里显式放
 * 一个 '\0' 字节（而不是依赖字符串结尾的隐式 '\0'，因为 store 后面还
 * 跟着 content，你需要精确控制 header 的长度）。
 * ------------------------------------------------------------------- */

int minigit_object_hash(minigit_obj_type type, const void *data, size_t size,
                         minigit_oid *out_oid) {
    (void)type;
    (void)data;
    (void)size;
    (void)out_oid;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_object_write(const minigit_repo *repo, minigit_obj_type type,
                          const void *data, size_t size, minigit_oid *out_oid) {
  char *header = NULL, *target_path = NULL;
  void *store = NULL;
  unsigned char *compressed = NULL;

  // step 1: generate 'header'
  int header_len = snprintf(NULL, 0, "%s %zu", minigit_obj_type_name(type), size);
  header = malloc(header_len + 1);
  snprintf(header, header_len, "%s %zu", minigit_obj_type_name(type), size);
  header[header_len - 1] = '\0';
  // step 2: generate 'store'
  size_t store_len = header_len + size;
  store = realloc(header, store_len);
  memcpy(store + header_len, data, size);
  // step 3: sha1 for 'store'
  if (!out_oid)  // FIXME: dont know if `out_oid` is a valid pointer
    out_oid = malloc(sizeof(*out_oid));
  minigit_sha1_buffer(store, store_len, out_oid);
  // step 4: compress 'store' using zlib
  size_t compressed_len = 0;
  int result = minigit_compress(store, store_len, &compressed, &compressed_len);
  if (result != MINIGIT_OK) goto cleanup;
  // step 5: write compressed to a file
  int target_path_len = snprintf(NULL, 0, "%s/objects/%x/%s",
        repo->git_dir,
        out_oid->id[0],
        out_oid->id + 1
      );
  target_path = malloc(target_path_len);
  snprintf(target_path, target_path_len, "%s/objects/%x/%s",
        repo->git_dir,
        out_oid->id[0],
        out_oid->id + 1);
  if (minigit_path_exists(target_path)) return MINIGIT_OK;
  result = minigit_write_file(target_path, compressed, compressed_len);

cleanup:
  if (store) free(store);
  if (compressed) free(compressed);
  if (target_path) free(target_path);
  return result;
}


int minigit_object_read(const minigit_repo *repo, const minigit_oid *oid,
                         minigit_object *out) {
    (void)repo;
    (void)oid;
    (void)out;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

void minigit_object_free(minigit_object *obj) {
    if (obj == NULL) {
        return;
    }
    free(obj->data);
    obj->data = NULL;
    obj->size = 0;
}
