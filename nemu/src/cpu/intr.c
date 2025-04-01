#include "cpu/exec.h"
#include "memory/mmu.h"

//Pa3.1
void raise_intr(uint8_t NO, vaddr_t save_addr) {
  /* 根据i386中断机制实现中断触发：
   * 1. 将EFLAGS, CS, EIP压入栈
   * 2. 从IDTR中获取IDT地址和长度
   * 3. 根据中断号在IDT中索引门描述符
   * 4. 跳转到目标地址
   */
  
  // 1. 将EFLAGS, CS, EIP保存到栈上
  rtl_push(&cpu.eflags.val);  // 保存EFLAGS
  rtl_push(&cpu.cs);          // 保存CS
  rtl_push(&save_addr);       // 保存EIP (下一条指令的地址)
  
  // 2. 根据中断号从IDT中查找门描述符
  uint32_t idt_addr = cpu.idtr.base;
  uint32_t gate_addr = idt_addr + NO * 8; // 每个门描述符8字节
  
  // 3. 读取门描述符中的offset域
  rtl_li(&t0, vaddr_read(gate_addr,2));
  rtl_li(&t1, vaddr_read(gate_addr + 4, 4));

  // 4. 跳转到目标地址
  uint32_t target = (t1 & 0xffff0000) | (t0& 0xffff);
  decoding.jmp_eip = target;
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
}
