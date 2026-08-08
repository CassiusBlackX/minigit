/*
 * cmd_add — 对应 `git add`
 *
 * ============================================================================
 * TODO(你来实现)
 * ============================================================================
 * 用法：`minigit add <file>...`（本项目要求：至少支持显式列出的普通
 * 文件路径；支持 "." 递归添加整个工作区是加分项，可以用 fsutil.h 里
 * 已经实现好的 minigit_walk_files 来遍历，回调里对每个文件做和下面
 * 一样的处理）
 *
 * 对每个文件路径，需要做的事：
 *   1. minigit_read_file 读出文件内容。
 *   2. 判断文件是不是可执行（用 <sys/stat.h> 的 stat()，检查
 *      st_mode & S_IXUSR），决定用 MINIGIT_MODE_EXEC 还是
 *      MINIGIT_MODE_BLOB（都定义在 tree.h）。
 *   3. minigit_object_write(repo, MINIGIT_OBJ_BLOB, data, size, &oid)
 *      —— 注意：无论这个文件的用途是 tree 里的普通文件还是可执行文件，
 *      对象类型都是 blob，可执行位是记录在 tree entry 的 mode 里的，
 *      不是对象本身的属性——这是很多初学者会搞混的地方：git 的对象
 *      存储层不知道"权限"这个概念，权限是 tree 这一层的元数据。
 *   4. minigit_index_load 读出当前 index。
 *   5. minigit_index_add(&index, path, mode, &oid) 登记这条记录（注意
 *      path 应该是【相对工作区根目录】的路径，不是相对当前工作目录的
 *      路径——如果用户在子目录里执行 minigit add，你可能需要用
 *      repo->work_tree 和 realpath 换算出真正的相对路径；如果暂时不想
 *      处理这个复杂度，可以先约定"minigit 必须在仓库根目录下执行"，这
 *      是一个可以接受的简化）。
 *   6. minigit_index_save 写回磁盘。
 *
 * 注意 4-6 步如果对多个文件循环调用，应该【只 load 一次、add 多次、
 * 最后 save 一次】，而不是每个文件都重新 load/save 一遍——否则如果一
 * 次 add 多个文件，后面的 save 会用旧数据覆盖前面文件的修改。
 * ============================================================================
 */
#include "minigit/commands.h"
#include "minigit/fsutil.h"
#include "minigit/index.h"
#include "minigit/object.h"
#include "minigit/tree.h"

int cmd_add(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return MINIGIT_ERR_NOT_IMPLEMENTED;
}
