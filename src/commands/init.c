/*
 * cmd_init — 对应 `git init`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit init [path]`（path 省略时默认是当前目录 "."）。
 *
 * 和其他子命令不同，init 【不能】调用 minigit_cli_require_repo /
 * minigit_repo_discover——因为这条命令的目的就是"创建" .git，此时它还
 * 不存在，也不需要向上查找已存在的仓库。
 *
 * 需要创建的磁盘结构（相对 path）：
 *   .git/                      仓库根目录
 *   .git/objects/              对象数据库（先创建空目录即可，子目录
 *                               "xx/" 会在第一次写对象时按需创建——参考
 *                               fsutil.h 里 minigit_write_file 会自动
 *                               创建父目录，所以这里不需要提前建好所有
 *                               两位前缀子目录）
 *   .git/refs/heads/           分支存放位置（先创建空目录，还没有任何
 *                               提交，所以此时不应该创建 refs/heads/main
 *                               文件——它要等第一次 commit 时才出现，
 *                               这正是真实 git 里"没有提交的空仓库,
 *                               HEAD 指向一个还不存在的分支"这个初看
 *                               有点反直觉的状态）
 *   .git/HEAD                  内容固定为 "ref: refs/heads/main\n"
 *
 * 建议直接用 minigit_mkdir_p（fsutil.h，已实现）创建目录，
 * minigit_write_file 写 HEAD 文件。
 *
 * 成功后建议打印一行提示，例如：
 *   "Initialized empty minigit repository in <绝对路径>/.git/"
 * （可以用 realpath() 得到绝对路径，或者直接打印相对路径，测试不会
 * 严格校验这行文字，只会校验磁盘上的文件结构）。
 *
 * 如果 .git 已经存在，参考真实 git 的行为：不报错，视为幂等操作即可
 * （比如打印 "Reinitialized existing minigit repository in ..."，或者
 * 你觉得没必要区分也可以，测试不强制要求这个区分）。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/fsutil.h"

int cmd_init(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
