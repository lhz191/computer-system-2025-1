#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

/*Pa2.1 begin*/
// 添加sub指令的声明
make_EHelper(sub);
/*Pa2.1 end*/