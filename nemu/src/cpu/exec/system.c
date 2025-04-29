#include "cpu/exec.h"
#include "common.h"

// 函数声明，放在文件开头
void raise_intr(uint8_t NO, vaddr_t save_addr);
void diff_test_skip_qemu();
void diff_test_skip_nemu();

//Pa3.2 实现lidt指令
make_EHelper(lidt) {
  rtl_li(&t0, id_dest->addr);
  
  // 读取limit和base
  cpu.idtr.limit = vaddr_read(t0, 2);
  cpu.idtr.base = vaddr_read(t0 + 2, 4);
  
  print_asm_template1(lidt);
}

/* PA4.1: 实现CR寄存器的操作指令 */
make_EHelper(mov_r2cr) {
  // 将通用寄存器的值移动到控制寄存器
  switch (id_dest->reg) {
    case 0: cpu.cr0 = id_src->val; break;
    case 3: cpu.cr3 = id_src->val; break;
    default: assert(0);
  }
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  // 将控制寄存器的值移动到通用寄存器
  switch (id_src->reg) {
    case 0: operand_write(id_dest, &cpu.cr0); break;
    case 3: operand_write(id_dest, &cpu.cr3); break;
    default: assert(0);
  }
  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));
}


//Pa3.2 实现中断函数
make_EHelper(int) {
  raise_intr(id_dest->val, decoding.seq_eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}
//Pa3.1 实现iret函数
make_EHelper(iret) {
  // 从栈中弹出 EIP, CS, EFLAGS
  rtl_pop(&decoding.jmp_eip);        // 恢复 EIP
  rtl_pop(&cpu.cs);         // 恢复 CS
  rtl_pop(&cpu.eflags.val); // 恢复 EFLAGS

  // 设置 decoding.is_jmp 使 CPU 跳转到弹出的 EIP
  decoding.is_jmp = 1;

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  // TODO();
  // 从设备端口读取数据
    // 从设备端口读取数据
  // uint32_t port_value = pio_read(id_src->val, id_dest->width);
  // printf("从端口 0x%x 读取值: 0x%x (宽度: %d)\n", id_src->val, port_value, id_dest->width);
  rtl_li(&t0, pio_read(id_src->val, id_dest->width));
  operand_write(id_dest, &t0);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  // printf("向端口 0x%x 写入值: 0x%x (宽度: %d)\n", id_dest->val, id_src->val, id_src->width);
  // 向设备端口写入数据
  pio_write(id_dest->val, id_src->width, id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
