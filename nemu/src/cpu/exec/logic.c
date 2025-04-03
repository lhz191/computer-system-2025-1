#include "cpu/exec.h"


/*
test指令的主要用途：
它执行两个操作数的按位与运算，但不保存结果
只更新标志位，常用于条件判断
最常见的用法是test %reg, %reg，用来检查寄存器是否为0
*/
make_EHelper(test) {
  // 1. 执行按位与操作，结果存储在t2中
  rtl_and(&t2, &id_dest->val, &id_src->val);
  
  // 2. 更新ZF和SF标志位
  // ZF(Zero Flag): 如果结果为0，则ZF=1；否则ZF=0
  // SF(Sign Flag): 如果结果为负数（最高位为1），则SF=1；否则SF=0
  rtl_update_ZFSF(&t2, id_dest->width);
  
  // 3. 清除CF和OF标志位
  // CF(Carry Flag): 设置为0，因为逻辑运算不产生进位
  // OF(Overflow Flag): 设置为0，因为逻辑运算不会溢出
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(test);
}

make_EHelper(and) {
  /*Pa2.1 Begin*/
  // 执行与运算,使用rtl_and执行按位与操作
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);//将结果写回目标操作数

  // 更新标志位,ZF和SF根据结果更新，
  //零标志和符号标志都需要根据操作数宽度进行
  rtl_update_ZFSF(&t2, id_dest->width);
  
  // AND指令会清除OF和CF标志位（AND指令的特性），逻辑运算不存在进位，也不存在溢出
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  /*Pa2.1 End*/

  print_asm_template2(and);
}
/*Pa2.1 xor运算*/
make_EHelper(xor) {
  // 执行异或运算
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  // 更新标志位，这里与之前的and基本类似，原理也相同，都是逻辑运算
  rtl_update_ZFSF(&t2, id_dest->width);
  
  // XOR指令会清除OF和CF标志位
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(xor);
}

make_EHelper(or) {
  // 执行或运算
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  // 更新标志位
  rtl_update_ZFSF(&t2, id_dest->width);
  
  // OR指令会清除OF和CF标志位
  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(or);
}

make_EHelper(sar) {
  // 1. 先对目标操作数进行符号扩展
  rtl_sext(&t0, &id_dest->val, id_dest->width);
  
  // 2. 执行算术右移操作
  // rtl_sar(dest, src1, src2) 其中src2是移位数量
  rtl_sar(&t0, &t0, &id_src->val);
  
  // 3. 将结果写回目标操作数
  operand_write(id_dest, &t0);
  
  // 4. 更新标志位
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  
  // CF和OF在NEMU中不需要更新，注释已说明
  
  print_asm_template2(sar);
}

make_EHelper(shl) {
  // 1. 获取目标操作数
  rtl_shl(&t0, &id_dest->val, &id_src->val);
  
  // 2. 将结果写回目标操作数
  operand_write(id_dest, &t0);
  
  // 3. 更新标志位
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  
  // CF和OF在NEMU中不需要更新，注释已说明
  
  print_asm_template2(shl);
}


make_EHelper(rol) {
  // 1. 获取移位数量（对32取模，因为我们只关心低5位）
  rtl_andi(&t0, &id_src->val, 0x1f);
  
  if (t0 != 0) {  // 只有当移位数不为0时才进行操作
    // 2. 执行循环左移操作
    rtl_shl(&t2, &id_dest->val, &t0);  // 左移部分
    rtl_li(&t3, 32);
    rtl_sub(&t3, &t3, &t0);  // 32 - count
    rtl_shr(&t3, &id_dest->val, &t3);  // 右移部分（循环回来的位）
    rtl_or(&t0, &t2, &t3);  // 合并结果
    
    // 3. 将结果写回目标操作数
    operand_write(id_dest, &t0);
    
    // 4. 更新标志位
    rtl_update_ZFSF(&t0, id_dest->width);
  }
  
  // CF和OF在NEMU中不需要更新，注释已说明
  
  print_asm_template2(rol);
}


make_EHelper(shr) {
  // 1. 执行逻辑右移操作
  rtl_shr(&t0, &id_dest->val, &id_src->val);
  
  // 2. 将结果写回目标操作数
  operand_write(id_dest, &t0);
  
  // 3. 更新标志位
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  
  // CF和OF在NEMU中不需要更新，注释已说明
  
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  // 1. 先将源操作数复制到临时寄存器
  rtl_mv(&t0, &id_dest->val);
  
  // 2. 执行按位取反操作
  rtl_not(&t0);
  
  // 3. 将结果写回目标操作数
  operand_write(id_dest, &t0);
  
  // NOT 指令不影响任何标志位
  
  print_asm_template1(not);
}