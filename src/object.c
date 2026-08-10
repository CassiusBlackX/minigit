#include "minigit/object.h"
#include "minigit/common.h"
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
  char *header = NULL;
  unsigned char *store = NULL;
  int result = MINIGIT_OK;

  // step 1: generate `header`
  int header_len = snprintf(NULL, 0, "%s %zu", minigit_obj_type_name(type), size);
  header_len++;  // add the '\0' into the len count
  header = malloc(header_len);
  snprintf(header, header_len, "%s %zu", minigit_obj_type_name(type), size);
  header[header_len - 1] = '\0';
  // step 2: generate 'store'
  size_t store_len = header_len + size;
  store = realloc(header, store_len);
  memcpy(store + header_len, data, size);
  // step 3: sha1 for 'store'
  if (!out_oid) {
    result = MINIGIT_ERR;
    goto cleanup;
  }
  minigit_sha1_buffer(store, store_len, out_oid);

cleanup:
  free(store);
  return result;
}

int minigit_object_write(const minigit_repo *repo, minigit_obj_type type,
                          const void *data, size_t size, minigit_oid *out_oid) {
  char *header = NULL, *target_path = NULL;
  void *store = NULL;
  unsigned char *compressed = NULL;
  int result = MINIGIT_OK;

  // step 1: generate 'header'
  int header_len = snprintf(NULL, 0, "%s %zu", minigit_obj_type_name(type), size);
  header_len++;  // add the '\0' into the len count
  header = malloc(header_len);
  snprintf(header, header_len, "%s %zu", minigit_obj_type_name(type), size);
  header[header_len - 1] = '\0';
  // step 2: generate 'store'
  size_t store_len = header_len + size;
  store = realloc(header, store_len);
  memcpy(store + header_len, data, size);
  // step 3: sha1 for 'store'
  if (!out_oid) {
    result = MINIGIT_ERR;
    goto cleanup;
  }
  minigit_sha1_buffer(store, store_len, out_oid);
  // step 4: compress 'store' using zlib
  size_t compressed_len = 0;
  result = minigit_compress(store, store_len, &compressed, &compressed_len);
  if (result != MINIGIT_OK) goto cleanup;
  // step 5: write compressed to a file
  char oid_hex[MINIGIT_OID_HEXSZ + 1];
  minigit_oid_to_hex(out_oid, oid_hex);
  int target_path_len = snprintf(NULL, 0, "%s/objects/%.2s/%.38s",
        repo->git_dir,
        oid_hex,
        oid_hex + 2);
  target_path_len++;
  target_path = malloc(target_path_len);
  snprintf(target_path, target_path_len, "%s/objects/%.2s/%.38s",
        repo->git_dir,
        oid_hex,
        oid_hex + 2);  
  if (!minigit_path_exists(target_path)) 
    result = minigit_write_file(target_path, compressed, compressed_len);
  else
    result = MINIGIT_OK;  // file exists, skip write

cleanup:
  if (store) free(store);
  if (compressed) free(compressed);
  if (target_path) free(target_path);
  return result;
}


int minigit_object_read(const minigit_repo *repo, const minigit_oid *oid,
                         minigit_object *out) {
  char *target_path = NULL, *header = NULL;
  unsigned char *compressed = NULL, *decompressed = NULL;
  int result = MINIGIT_ERR;
  if (!out) return result;

  // step 1 : generate target_path
  char oid_hex[MINIGIT_OID_HEXSZ + 1];
  minigit_oid_to_hex(oid, oid_hex);
  int target_path_len = snprintf(NULL, 0, "%s/objects/%.2s/%.38s",
        repo->git_dir,
        oid_hex,
        oid_hex + 2);
  target_path_len++;
  target_path = malloc(target_path_len);
  snprintf(target_path, target_path_len, "%s/objects/%.2s/%.38s",
        repo->git_dir,
        oid_hex,
        oid_hex + 2);
  // step 2 :read compressed data
  if (!minigit_path_exists(target_path) || 
      minigit_path_is_dir(target_path)) {  // file path should not be a directory
    result = MINIGIT_ERR_NOT_FOUND;
    goto cleanup;
  }
  size_t compressed_len;
  result = minigit_read_file(target_path, &compressed, &compressed_len);
  if (result != MINIGIT_OK) goto cleanup;
  // step 3 : decompress to get 'store'
  size_t decompressed_len;
  result = minigit_decompress(compressed, compressed_len, &decompressed, &decompressed_len);
  if (result != MINIGIT_OK) goto cleanup;
  // step 4: restore header
  //   restore type
  char type_name[7] = {};
  size_t header_idx;
  for (header_idx = 0; header_idx <= 6; header_idx++) {
    if (header_idx == decompressed_len) {
      result = MINIGIT_ERR_INVALID;
      goto cleanup;
    }
    if (decompressed[header_idx] == ' ') {
      type_name[header_idx] = '\0';
      header_idx++;  // pass the ' '
      break;
    }
    type_name[header_idx] = decompressed[header_idx];
  }
  result = minigit_obj_type_from_name(type_name, &(out->type));
  if (result != MINIGIT_OK) return result;
  //   size
  char size_char[64] = {};  // FIXME: how long should the array be?
  int i = 0;
  while (decompressed[header_idx] != '\0' && header_idx < decompressed_len) {
    size_char[i++] = decompressed[header_idx++];
  }
  size_char[i] = '\0';
  out->size = atoi(size_char);
  if (out->size != decompressed_len - header_idx - 1) {
    result = MINIGIT_ERR_INVALID;
    goto cleanup;
  }
  // step 5: copy content to out->data
  unsigned char * p = decompressed + header_idx + 1;
  out->data = malloc(out->size);
  memcpy(out->data, p, out->size);
  result = MINIGIT_OK;

cleanup:
  if (target_path) free(target_path);
  if (header) free(header);
  if (compressed) free(compressed);
  if (decompressed) free(decompressed);
  return result;
}

void minigit_object_free(minigit_object *obj) {
    if (obj == NULL) {
        return;
    }
    free(obj->data);
    obj->data = NULL;
    obj->size = 0;
}
