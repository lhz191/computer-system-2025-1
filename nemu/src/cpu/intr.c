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
  
  // 在保存EFLAGS后，将IF位置为0，让处理器进入关中断状态
  cpu.eflags.IF = 0;
  
  rtl_push(&cpu.cs);          // 保存CS
  rtl_push(&save_addr);       // 保存EIP (下一条指令的地址)
  
  // 2. 根据中断号从IDT中查找门描述符
  uint32_t idt_addr = cpu.idtr.base;
  uint32_t gate_addr = idt_addr + NO * 8; // 每个门描述符8字节
  
  // 3. 读取门描述符中的offset域
  rtl_li(&t0, vaddr_read(gate_addr,2));//读取当前门低16位偏移量
  rtl_li(&t1, vaddr_read(gate_addr + 4, 4));//高32位
  
  // 4. 跳转到目标地址
  uint32_t target = (t1 & 0xffff0000) | (t0& 0xffff);
  //高16与低16拼接（？暂时存疑，后续进行测试），//测试无误
  decoding.jmp_eip = target;
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
  // 设置INTR引脚为高电平，表示有中断请求
  cpu.INTR = true;
}
