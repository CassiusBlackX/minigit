#include "minigit/tree.h"

#include <stdlib.h>
#include <string.h>

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
    (void)tree;
    (void)out_data;
    (void)out_size;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}

int minigit_tree_parse(const unsigned char *data, size_t size, minigit_tree *out_tree) {
    (void)data;
    (void)size;
    (void)out_tree;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
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
