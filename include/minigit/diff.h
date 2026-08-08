/*
 * diff.h — 行级 diff：比较两段文本，找出改动的最小行集合。
 *
 * ============================================================================
 * 【思路：最长公共子序列 (LCS)】
 * ============================================================================
 * "两段文本的 diff"这个问题，标准做法是转化成"两个序列的最长公共子
 * 序列 (Longest Common Subsequence)"问题：
 *
 *   把文本 a 按行切成序列 A = [A1, A2, ..., Am]，文本 b 切成
 *   B = [B1, B2, ..., Bn]（这里"行"是比较的最小单位，两行只要内容
 *   完全相同就算相等，不关心行内字符级别的差异——真实 git 默认也是
 *   按行 diff，字符级高亮是终端展示层面的锦上添花，本项目不需要）。
 *
 *   LCS(A, B) 是"在 A 和 B 中都按原相对顺序出现的最长公共行序列"。
 *   直觉上：LCS 里的行就是"双方都没变的行"，不在 LCS 里但在 A 中的行
 *   就是"被删除的行"，不在 LCS 里但在 B 中的行就是"被新增的行"。
 *
 * 【标准动态规划解法】
 *   定义 dp[i][j] = LCS(A[0..i), B[0..j)) 的长度（i, j 从 0 开始表示
 *   "前 i / j 行"）。转移方程：
 *
 *       dp[0][j] = dp[i][0] = 0
 *       dp[i][j] = dp[i-1][j-1] + 1                    若 A[i-1] == B[j-1]
 *       dp[i][j] = max(dp[i-1][j], dp[i][j-1])          否则
 *
 *   这是一个 O(m*n) 时间、O(m*n) 空间的表。填完表后，从 dp[m][n] 开始
 *   【倒着回溯】：
 *       - 若 A[i-1] == B[j-1]：这一行是"未变"（EQUAL），i--, j-- 同时
 *         往回走；
 *       - 否则若 dp[i-1][j] >= dp[i][j-1]：说明这一步是从"少一行 A"
 *         转移来的，也就是 A[i-1] 是被删除的行（DEL），i--；
 *       - 否则：B[j-1] 是被新增的行（ADD），j--。
 *   回溯到 i==0 且 j==0 结束。把回溯过程中产生的 op 逆序排列，就是从
 *   头到尾的 diff 结果。
 *
 *   注意复杂度：文件很大时 O(m*n) 会很慢（真实 git 用的是更快的 Myers
 *   diff 算法，O((m+n)*D)，D 是编辑距离）。本项目定位是学习原理，普通
 *   DP 版 LCS 完全够用，不需要实现 Myers 算法。
 *
 * ============================================================================
 */
#ifndef MINIGIT_DIFF_H
#define MINIGIT_DIFF_H

#include "minigit/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MINIGIT_DIFF_EQUAL,
    MINIGIT_DIFF_ADD,   /* 这一行只存在于 b（新增） */
    MINIGIT_DIFF_DEL,   /* 这一行只存在于 a（删除） */
} minigit_diff_op;

typedef struct {
    minigit_diff_op op;
    const char *line; /* 指向 a 或 b 原始缓冲区内部，不拥有内存，不要 free */
    size_t len;        /* 行内容长度，不含换行符 */
} minigit_diff_line;

typedef struct {
    minigit_diff_line *lines; /* 调用方通过 minigit_diff_free 释放这个数组本身 */
    size_t count;
} minigit_diff_result;

/* ----------------------------------------------------------------------
 * TODO(你来实现): src/diff.c
 * -------------------------------------------------------------------- */

/* 对 a[0..a_len) 和 b[0..b_len) 按行做 diff。
 * 提示：
 *   1. 先写一个小的内部 helper，把一段 buffer 按 '\n' 切分成
 *      (指针, 长度) 的行数组（不用拷贝字符串，直接记录偏移量），
 *      分别用于 a 和 b。
 *   2. 按上面注释描述的 DP 建表 + 回溯，生成 minigit_diff_line 数组。
 *   3. out->lines / out->count 是最终结果，注意 line 指针要指向【原始
 *      a/b 缓冲区】里的位置，调用方（比如 diff_cmd.c）在 out 使用完之前
 *      不能释放 a/b。 */
int minigit_diff_lines(const char *a, size_t a_len, const char *b, size_t b_len,
                        minigit_diff_result *out);

/* 释放 minigit_diff_lines 分配的 out->lines 数组（不释放 a/b，那是调
 * 用方的） */
void minigit_diff_free(minigit_diff_result *out);

#ifdef __cplusplus
}
#endif

#endif /* MINIGIT_DIFF_H */
