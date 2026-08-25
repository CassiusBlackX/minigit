#include "minigit/refs.h"
#include "minigit/common.h"
#include "minigit/fsutil.h"
#include "minigit/sha1.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* TODO(你来实现)：见 refs.h 顶部的说明 */

int minigit_ref_resolve(const minigit_repo *repo, const char *ref_name,
                        minigit_oid *out_oid) {
  int status = MINIGIT_OK;
  unsigned char *buf = NULL;
  // read ref file content
  const char *ref_file_path = minigit_path_join(repo->git_dir, ref_name);
  if (!minigit_path_exists(ref_file_path)) {
    status = MINIGIT_ERR_NOT_FOUND;
    goto cleanup;
  }
  size_t buf_size = 0;
  status = minigit_read_file(ref_file_path, &buf, &buf_size);
  if (status != MINIGIT_OK)
    goto cleanup;

  // parse file
  char startup[5] = {};
  memcpy(startup, buf, 4);
  startup[4] = '\0';
  if (strcmp(startup, "ref:") == 0) {
    // symbol reference
    const size_t new_ref_file_start_pos = 5;
    size_t index = new_ref_file_start_pos; // new_ref_file_path start pos
    while (buf[index] != '\n' && index < buf_size)
      index++;
    if (index >= buf_size) {
      status = MINIGIT_ERR_INVALID;
      goto cleanup;
    }
    char *new_ref_name = malloc(index - new_ref_file_start_pos + 1);
    memcpy(new_ref_name, buf + new_ref_file_start_pos,
           index - new_ref_file_start_pos);
    new_ref_name[index - new_ref_file_start_pos] = '\0';
    status = minigit_ref_resolve(repo, new_ref_name, out_oid);
    free(new_ref_name);
  } else if (buf_size == 41) {
    // hash file
    buf[buf_size - 1] = '\0'; // transform '\n' into '\0'
    status = minigit_oid_from_hex((const char *)buf, out_oid);
  } else
    status = MINIGIT_ERR_INVALID;

cleanup:
  free((char *)ref_file_path);
  free(buf);
  return status;
}

int minigit_ref_update(const minigit_repo *repo, const char *ref_name,
                       const minigit_oid *oid) {
  const char *ref_file_path = minigit_path_join(repo->git_dir, ref_name);
  char oid_hex[MINIGIT_OID_HEXSZ + 1] = {};
  minigit_oid_to_hex(oid, oid_hex);
  oid_hex[MINIGIT_OID_HEXSZ] = '\n';
  int status = minigit_write_file(ref_file_path, oid_hex, MINIGIT_OID_HEXSZ + 1);
  free((char*)ref_file_path);
  return status;
}

int minigit_ref_current_branch(const minigit_repo *repo, char **out_name) {
  int status = MINIGIT_OK;
  const char *ref_file_path = minigit_path_join(repo->git_dir, "HEAD");
  unsigned char *buf = NULL;
  if (!minigit_path_exists(ref_file_path)) {
    status = MINIGIT_ERR_INVALID;
    goto cleanup;
  }
  size_t buf_size = 0;
  status = minigit_read_file(ref_file_path, &buf, &buf_size);
  if (status != MINIGIT_OK) goto cleanup;

  // parse file
  char startup[17] = {};
  memcpy(startup, buf, 16);
  startup[16] = '\0';
  if (strcmp(startup, "ref: refs/heads/") != 0) {
    status = MINIGIT_ERR_INVALID;
    goto cleanup;
  }

  // symbol reference
  const size_t new_ref_file_start_pos = 16;
  size_t index = new_ref_file_start_pos; // new_ref_file_path start pos
  while (buf[index] != '\n' && index < buf_size)
    index++;
  if (index >= buf_size) {
    status = MINIGIT_ERR_INVALID;
    goto cleanup;
  }
  char *new_ref_name = malloc(index - new_ref_file_start_pos + 1);
  memcpy(new_ref_name, buf + new_ref_file_start_pos,
         index - new_ref_file_start_pos);
  new_ref_name[index - new_ref_file_start_pos] = '\0';
  *out_name = new_ref_name;

cleanup:
  free((char*)ref_file_path);
  free(buf);
  return status;
}

int minigit_ref_set_head_symbolic(const minigit_repo *repo,
                                  const char *branch_name) {
  int status = MINIGIT_OK;
  const char *ref_file_path = minigit_path_join(repo->git_dir, "HEAD");
  int ref_name_len = snprintf(NULL, 0, "ref: refs/heads/%s\n", branch_name);
  char *buf = malloc(ref_name_len + 1);
  snprintf(buf, ref_name_len + 1, "ref: refs/heads/%s\n", branch_name);
  status = minigit_write_file(ref_file_path, buf, ref_name_len);
  free(buf);
  free((char*)ref_file_path);
  return status;
}

int minigit_ref_create_branch(const minigit_repo *repo, const char *name,
                              const minigit_oid *oid) {
  int status = MINIGIT_OK;
  int ref_file_path_name_len = snprintf(NULL, 0, "%s/refs/heads/%s", repo->git_dir, name);
  char *ref_file_path = malloc(ref_file_path_name_len + 1);
  snprintf(ref_file_path, ref_file_path_name_len + 1, "%s/refs/heads/%s", repo->git_dir, name);
  if (minigit_path_exists(ref_file_path)) {
    status = MINIGIT_ERR_EXISTS;
    goto cleanup;
  }

  char oid_hex[MINIGIT_OID_HEXSZ + 1] = {};
  minigit_oid_to_hex(oid, oid_hex);
  oid_hex[MINIGIT_OID_HEXSZ] = '\n';
  status = minigit_write_file(ref_file_path, oid_hex, MINIGIT_OID_HEXSZ + 1);

cleanup:
  free(ref_file_path);
  return status;
}

int minigit_ref_list_branches(const minigit_repo *repo, char ***out_names,
                              size_t *out_count) {
  int status = MINIGIT_OK;
  const char *ref_heads_dir = minigit_path_join(repo->git_dir, "refs/heads");
  if (!minigit_path_is_dir(ref_heads_dir)) {
    status = MINIGIT_ERR_NOT_FOUND;
    goto cleanup;
  }
  
  status = minigit_list_files(ref_heads_dir, out_names, out_count);

cleanup:
  free((char*)ref_heads_dir);
  return status;
}
