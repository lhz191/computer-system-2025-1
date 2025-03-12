#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct {
  union {
/*union 允许多个数据类型共享同一块内存。这意味着在任何时刻，只能使用其中一个成员。比如，在寄存器的情况下，只需要在某个时刻使用 32 位、16 位或 8 位中的一个，而不是同时使用它们。*/
    union {
      uint32_t _32; // 32-bit register
      uint16_t _16; // 16-bit register
      uint8_t _8[2]; // 8-bit registers
  } gpr[8];

  /* Do NOT change the order of the GPRs' definitions. */

  /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
   * in PA2 able to directly access these registers.
   */
  struct {
/*struct 允许同时存储多个不同类型的数据。对于寄存器来说，使用 struct 可以直接通过名称访问特定的寄存器（如 eax、ecx 等），而不需要记住它们在数组中的位置。*/
      rtlreg_t eax; // 32-bit register
      rtlreg_t ecx; // 32-bit register
      rtlreg_t edx; // 32-bit register
      rtlreg_t ebx; // 32-bit register
      rtlreg_t esp; // 32-bit register
      rtlreg_t ebp; // 32-bit register
      rtlreg_t esi; // 32-bit register
      rtlreg_t edi; // 32-bit register
   };
 };

  vaddr_t eip;

  /* PA2：添加EFLAGS寄存器 */
  /* x86架构中EFLAGS寄存器的位分布：
 * CF在第0位
 * OF在第11位  
 * IF在第9位
 * SF在第7位
 * ZF在第6位
 * 一共32位
 */
  union {
    struct {
      uint32_t CF:1;      // Carry Flag
      uint32_t always1:1; // 保留位，总是1
      uint32_t pad0:4;    // 保留位
      uint32_t ZF:1;      // Zero Flag
      uint32_t SF:1;      // Sign Flag
      uint32_t pad1:1;    // 保留位
      uint32_t IF:1;      // Interrupt Flag
      uint32_t pad2:1;    // 保留位
      uint32_t OF:1;      // Overflow Flag
      uint32_t pad3:20;   // 保留位
    };
    uint32_t val;         // EFLAGS的32位值
  } eflags;
  
} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

static inline const char* reg_name(int index, int width) {
  assert(index >= 0 && index < 8);
  switch (width) {
    case 4: return regsl[index];
    case 1: return regsb[index];
    case 2: return regsw[index];
    default: assert(0);
  }
}

#endif