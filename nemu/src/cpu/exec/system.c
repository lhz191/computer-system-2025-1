#include "cpu/exec.h"
#include "common.h"

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

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}


//Pa3.2 实现中断函数
make_EHelper(int) {
  raise_intr(id_dest->val, decoding.seq_eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  TODO();

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
