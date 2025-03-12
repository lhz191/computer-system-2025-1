#include "cpu/exec.h"

make_EHelper(test) {
  TODO();

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
  TODO();

  print_asm_template2(or);
}

make_EHelper(sar) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  TODO();

  print_asm_template1(not);
}
