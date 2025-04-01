#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  /*Pa2.1 Begin*/
  // 将操作数压入栈中
  rtl_push(&id_dest->val);
  /*Pa2.1 End*/
  print_asm_template1(push);
}

/*Pa2.1 0x5d pop %ebp指令*/
make_EHelper(pop) {
  // 从栈中弹出数据到目标操作数
  rtl_pop(&t2);
  operand_write(id_dest, &t2);

  print_asm_template1(pop);
}
//Pa3.1 push和pop指令的实现
make_EHelper(pusha) {
  // 保存原始ESP值
  t0 = cpu.esp;
  
  // 按照顺序压栈：EAX, ECX, EDX, EBX, ESP(原始值), EBP, ESI, EDI
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t0);  // 压入原始ESP值
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}
//根据 i386 手册，popa 指令的弹出顺序为：EDI, ESI, EBP, 
//ESP(丢弃), EBX, EDX, ECX, EAX。
make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  
  rtl_pop(&t0); // 丢弃ESP的值
  
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {
  // leave指令相当于：
  // mov esp, ebp
  rtl_mv(&cpu.esp, &cpu.ebp);
  
  // pop ebp
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

/*Pa2.2 cltd和cwtl的实现存疑，没有程序进行测试*/
make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    // cwd: 将AX符号扩展到DX:AX
    // 从AX的最高位(bit 15)获取符号位
    rtl_sari(&t0, &reg_l(R_EAX), 15);
    // 将AX的符号位复制到DX的所有位
    rtl_sr(R_DX, 2, &t0);
  }
  else {
    // cdq: 将EAX符号扩展到EDX:EAX
    // 从EAX的最高位(bit 31)获取符号位
    rtl_sari(&t0, &reg_l(R_EAX), 31);
    // 将EAX的符号位复制到EDX的所有位
    rtl_sr(R_EDX, 4, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cwd" : "cdq");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    // cbw: 将AL符号扩展到AX
    // 提取AL(8位)
    rtl_lr(&t0, R_EAX, 1);
    // 符号扩展到16位
    rtl_sext(&t0, &t0, 1);
    // 写回AX
    rtl_sr(R_AX, 2, &t0);
  }
  else {
    // cwde: 将AX符号扩展到EAX
    // 提取AX(16位)
    rtl_lr(&t0, R_EAX, 2);
    // 符号扩展到32位
    rtl_sext(&t0, &t0, 2);
    // 写回EAX
    rtl_sr(R_EAX, 4, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cbw" : "cwde");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
