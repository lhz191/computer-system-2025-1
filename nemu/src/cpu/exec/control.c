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
  /*Pa1 Begin*/
  // the target address is calculated at the decode stage
  // 保存下一条指令的地址到栈中
  rtl_push(&decoding.seq_eip);
  
  // 设置跳转标志和目标地址
  decoding.is_jmp = 1;
  // 注意：decoding.jmp_eip已经在译码阶段设置好了，不需要我们计算

  print_asm("call %x", decoding.jmp_eip);
    /*Pa1 end*/
}

make_EHelper(ret) {
  TODO();

  print_asm("ret");
}

make_EHelper(call_rm) {
  TODO();

  print_asm("call *%s", id_dest->str);
}
