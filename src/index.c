#include "minigit/index.h"
#include "minigit/tree.h"
#include "minigit/object.h"

#include <stdlib.h>
#include <string.h>

/* TODO(你来实现)：见 index.h 顶部的格式说明与 write_tree 的算法提示 */

int minigit_index_load(const minigit_repo *repo, minigit_index *out_index) {
    (void)repo;
    (void)out_index;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_index_save(const minigit_repo *repo, const minigit_index *index) {
    (void)repo;
    (void)index;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_index_add(minigit_index *index, const char *path, unsigned int mode,
                       const minigit_oid *oid) {
    (void)index;
    (void)path;
    (void)mode;
    (void)oid;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_index_remove(minigit_index *index, const char *path) {
    (void)index;
    (void)path;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

minigit_index_entry *minigit_index_find(const minigit_index *index, const char *path) {
    (void)index;
    (void)path;
    return NULL;
}

int minigit_index_write_tree(const minigit_repo *repo, const minigit_index *index,
                              minigit_oid *out_tree_oid) {
    (void)repo;
    (void)index;
    (void)out_tree_oid;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
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
