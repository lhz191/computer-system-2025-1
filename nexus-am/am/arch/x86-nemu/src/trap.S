#----|-------entry-------|-errorcode-|---irq id---|---handler---|
.globl vecsys;    vecsys:  pushl $0;  pushl $0x80; jmp asm_trap
.globl vecnull;  vecnull:  pushl $0;  pushl   $-1; jmp asm_trap
.globl vecself;  vecself:  pushl $0;  pushl $0x81; jmp asm_trap

asm_trap:
  # 保存当前上下文到陷阱帧
  pushal

  # 传递陷阱帧指针给irq_handle
  pushl %esp
  call irq_handle

  # irq_handle返回新进程的陷阱帧指针(在eax中)
  # 我们不需要恢复esp，而是直接使用返回的新陷阱帧指针
  movl %eax, %esp

  # 从新进程的陷阱帧恢复上下文
  popal
  # 弹出错误码和中断号
  addl $8, %esp

  # 从中断返回，CPU会自动从栈中弹出EIP、CS和EFLAGS
  iret
