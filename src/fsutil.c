/*
 * fsutil.c — 已完整实现，直接使用。
 */
#include "minigit/fsutil.h"
#include "minigit/common.h"
#include "minigit/refs.h"

#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int minigit_path_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

int minigit_path_is_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

int minigit_mkdir_p(const char *path) {
    char buf[4096];
    size_t len = strlen(path);
    if (len == 0 || len >= sizeof(buf)) {
        return MINIGIT_ERR_INVALID;
    }
    memcpy(buf, path, len + 1);

    for (char *p = buf + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(buf, 0755) != 0 && errno != EEXIST) {
                return MINIGIT_ERR_IO;
            }
            *p = '/';
        }
    }
    if (mkdir(buf, 0755) != 0 && errno != EEXIST) {
        return MINIGIT_ERR_IO;
    }
    return MINIGIT_OK;
}

int minigit_read_file(const char *path, unsigned char **buf, size_t *len) {
    if (path == NULL || buf == NULL || len == NULL) {
        return MINIGIT_ERR_INVALID;
    }

    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return MINIGIT_ERR_NOT_FOUND;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return MINIGIT_ERR_IO;
    }
    long size = ftell(f);
    if (size < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return MINIGIT_ERR_IO;
    }

    unsigned char *data = malloc((size_t)size > 0 ? (size_t)size : 1);
    if (data == NULL) {
        fclose(f);
        return MINIGIT_ERR;
    }

    size_t read_bytes = size > 0 ? fread(data, 1, (size_t)size, f) : 0;
    fclose(f);
    if (read_bytes != (size_t)size) {
        free(data);
        return MINIGIT_ERR_IO;
    }

    *buf = data;
    *len = (size_t)size;
    return MINIGIT_OK;
}

/* 从完整路径中截取出父目录部分并 mkdir -p，例如
 * "/repo/.git/objects/ab/cdef..." -> mkdir -p "/repo/.git/objects/ab" */
static int ensure_parent_dir(const char *path) {
    const char *slash = strrchr(path, '/');
    if (slash == NULL) {
        return MINIGIT_OK; /* 没有父目录部分，比如相对路径 "HEAD" */
    }
    size_t dir_len = (size_t)(slash - path);
    if (dir_len == 0) {
        return MINIGIT_OK;
    }
    char *dir = malloc(dir_len + 1);
    if (dir == NULL) {
        return MINIGIT_ERR;
    }
    memcpy(dir, path, dir_len);
    dir[dir_len] = '\0';
    int rc = minigit_mkdir_p(dir);
    free(dir);
    return rc;
}

int minigit_write_file(const char *path, const void *buf, size_t len) {
    if (path == NULL || (buf == NULL && len != 0)) {
        return MINIGIT_ERR_INVALID;
    }

    int rc = ensure_parent_dir(path);
    if (rc != MINIGIT_OK) {
        return rc;
    }

    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        return MINIGIT_ERR_IO;
    }
    size_t written = len > 0 ? fwrite(buf, 1, len, f) : 0;
    int close_rc = fclose(f);
    if (written != len || close_rc != 0) {
        return MINIGIT_ERR_IO;
    }
    return MINIGIT_OK;
}

char *minigit_path_join(const char *a, const char *b) {
    if (a == NULL || b == NULL) {
        return NULL;
    }
    size_t alen = strlen(a);
    size_t blen = strlen(b);
    int need_sep = (alen > 0 && a[alen - 1] != '/');

    char *out = malloc(alen + (need_sep ? 1 : 0) + blen + 1);
    if (out == NULL) {
        return NULL;
    }
    memcpy(out, a, alen);
    size_t pos = alen;
    if (need_sep) {
        out[pos++] = '/';
    }
    memcpy(out + pos, b, blen);
    pos += blen;
    out[pos] = '\0';
    return out;
}

/* rel_prefix == "" 表示当前在 root 本身；否则是相对 root 的子目录路径 */
static int walk_recursive(const char *root, const char *rel_prefix, minigit_walk_fn cb,
                           void *userdata) {
    char *dir_full = (rel_prefix[0] == '\0') ? strdup(root) : minigit_path_join(root, rel_prefix);
    if (dir_full == NULL) {
        return MINIGIT_ERR;
    }

    DIR *dir = opendir(dir_full);
    free(dir_full);
    if (dir == NULL) {
        return MINIGIT_ERR_IO;
    }

    int rc = MINIGIT_OK;
    struct dirent *ent;
    while (rc == MINIGIT_OK && (ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        if (strcmp(ent->d_name, ".git") == 0) {
            continue;
        }

        char *entry_rel = (rel_prefix[0] == '\0') ? strdup(ent->d_name)
                                                    : minigit_path_join(rel_prefix, ent->d_name);
        char *entry_full = minigit_path_join(root, entry_rel);
        if (entry_rel == NULL || entry_full == NULL) {
            free(entry_rel);
            free(entry_full);
            rc = MINIGIT_ERR;
            break;
        }

        if (minigit_path_is_dir(entry_full)) {
            rc = walk_recursive(root, entry_rel, cb, userdata);
        } else {
            rc = cb(entry_rel, userdata);
        }

        free(entry_rel);
        free(entry_full);
    }

    closedir(dir);
    return rc;
}

int minigit_walk_files(const char *root, minigit_walk_fn cb, void *userdata) {
    if (root == NULL || cb == NULL) {
        return MINIGIT_ERR_INVALID;
    }
    return walk_recursive(root, "", cb, userdata);
}

int minigit_list_files(const char *dir_path, char ***out_names, size_t *out_count) {
  if (dir_path == NULL || out_names == NULL || out_count == NULL) return MINIGIT_ERR_INVALID;

  DIR *dir = opendir(dir_path);
  if (dir == NULL) return MINIGIT_ERR_NOT_FOUND;

  size_t count = 0;
  size_t capacity = 16;
  char **names = malloc(capacity * sizeof(char*));
  int rc = MINIGIT_OK;
  struct dirent *ent;

  while ((ent = readdir(dir)) != NULL) {
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

    /* only files, skip subdirectory */
    char *full_path = minigit_path_join(dir_path, ent->d_name);
    if (full_path == NULL) {
      rc = MINIGIT_ERR;
      break;
    }
    int is_dir = minigit_path_is_dir(full_path);
    free(full_path);
    if (is_dir) continue;

    if (count >= capacity) {
      capacity *= 2;
      char **new_names = realloc(names, capacity * sizeof(char*));
      if (new_names == NULL) {
        rc = MINIGIT_ERR;
        break;
      }
      names = new_names;
    }

    names[count] = strdup(ent->d_name);
    if (names[count] == NULL) {
      rc = MINIGIT_ERR;
      break;
    }
    count++;
  }
  closedir(dir);

  if (rc != MINIGIT_OK) {
    for (size_t i = 0; i < count; i++) {
      free(names[i]);
    }
    free(names);
    return rc;
  }

  *out_names = names;
  *out_count = count;
  return MINIGIT_OK;
}
