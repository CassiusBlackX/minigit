#include "minigit/refs.h"

#include <stdlib.h>
#include <string.h>

/* TODO(你来实现)：见 refs.h 顶部的说明 */

int minigit_ref_resolve(const minigit_repo *repo, const char *ref_name, minigit_oid *out_oid) {
    (void)repo;
    (void)ref_name;
    (void)out_oid;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_ref_update(const minigit_repo *repo, const char *ref_name, const minigit_oid *oid) {
    (void)repo;
    (void)ref_name;
    (void)oid;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_ref_current_branch(const minigit_repo *repo, char **out_name) {
    (void)repo;
    (void)out_name;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_ref_set_head_symbolic(const minigit_repo *repo, const char *branch_name) {
    (void)repo;
    (void)branch_name;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_ref_create_branch(const minigit_repo *repo, const char *name, const minigit_oid *oid) {
    (void)repo;
    (void)name;
    (void)oid;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_ref_list_branches(const minigit_repo *repo, char ***out_names, size_t *out_count) {
    (void)repo;
    (void)out_names;
    (void)out_count;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
