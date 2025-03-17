#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);



/*Pa2.1 Pa2.2 begin*/
// 添加sub指令的声明
// Group 1 instructions
make_EHelper(add);
make_EHelper(or);
make_EHelper(sbb);
make_EHelper(and);
make_EHelper(sub);
make_EHelper(xor);
make_EHelper(cmp);


make_EHelper(lea);
make_EHelper(push);
make_EHelper(pop);  // 添加pop指令声明
make_EHelper(xor);  // 添加xor指令声明
make_EHelper(movsx);  // 添加movsx指令声明
make_EHelper(test);  // 添加test指令声明
make_EHelper(jcc);  // 添加jcc指令声明
make_EHelper(cmp);  // 添加cmp指令声明
make_EHelper(inc);  // 添加inc指令声明
make_EHelper(dec);  // 添加dec指令声明
make_EHelper(neg);  // 添加neg指令声明
make_EHelper(ret);  // 添加ret指令声明
make_EHelper(jmp);  // 添加jmp指令声明

make_EHelper(jmp_rm);  // 添加jmp_rm声明
make_EHelper(call_rm); // 添加call_rm声明

make_EHelper(leave);  // 添加leave指令声明
make_EHelper(nop);  // 添加nop指令声明

make_EHelper(setcc);  // 添加setcc指令声明

make_EHelper(movzx);  // 添加movzx指令声明

make_EHelper(adc);  // 添加adc指令声明

make_EHelper(rol);  // 循环左移
make_EHelper(shl);  // 逻辑左移
make_EHelper(shr);  // 逻辑右移
make_EHelper(sar);  // 算术右移
/*Pa2.1 Pa2.2 end*/



