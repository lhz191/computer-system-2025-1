#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;
  
  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage.
  rtl_push(&decoding.seq_eip);
  //rtl_push(&cpu.eip);
  cpu.eip=decoding.jmp_eip;
  decoding.is_jmp = 1;
  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  // 从栈中弹出返回地址
  rtl_pop(&t0);
  
  // 设置下一条指令的地址
  decoding.jmp_eip = t0;
  decoding.is_jmp = 1;

  print_asm("ret");
}

make_EHelper(call_rm) {
  // 1. 保存下一条指令的地址到栈中
  rtl_push(&decoding.seq_eip);
  
  // 2. 设置跳转目标地址（从操作数中获取）
  decoding.jmp_eip = id_dest->val;
  
  // 3. 设置跳转标志
  decoding.is_jmp = 1;

  print_asm("call *%s", id_dest->str);
}

/*Pa2.1 在special.c文件中已被定义*/
// make_EHelper(nop) {
//   // nop指令不做任何操作
//   print_asm("nop");
// }