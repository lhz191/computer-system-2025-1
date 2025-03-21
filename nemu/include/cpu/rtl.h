#ifndef __RTL_H__
#define __RTL_H__

#include "nemu.h"

extern rtlreg_t t0, t1, t2, t3;
extern const rtlreg_t tzero;

/* RTL basic instructions */

 void rtl_li(rtlreg_t* dest, uint32_t imm) {
  *dest = imm;
}

#define c_add(a, b) ((a) + (b))
#define c_sub(a, b) ((a) - (b))
#define c_and(a, b) ((a) & (b))
#define c_or(a, b)  ((a) | (b))
#define c_xor(a, b) ((a) ^ (b))
#define c_shl(a, b) ((a) << (b))
#define c_shr(a, b) ((a) >> (b))
#define c_sar(a, b) ((int32_t)(a) >> (b))
#define c_slt(a, b) ((int32_t)(a) < (int32_t)(b))
#define c_sltu(a, b) ((a) < (b))

#define make_rtl_arith_logic(name) \
  static inline void concat(rtl_, name) (rtlreg_t* dest, const rtlreg_t* src1, const rtlreg_t* src2) { \
    *dest = concat(c_, name) (*src1, *src2); \
  } \
  static inline void concat3(rtl_, name, i) (rtlreg_t* dest, const rtlreg_t* src1, int imm) { \
    *dest = concat(c_, name) (*src1, imm); \
  }


make_rtl_arith_logic(add)
make_rtl_arith_logic(sub)
make_rtl_arith_logic(and)
make_rtl_arith_logic(or)
make_rtl_arith_logic(xor)
make_rtl_arith_logic(shl)
make_rtl_arith_logic(shr)
make_rtl_arith_logic(sar)
make_rtl_arith_logic(slt)
make_rtl_arith_logic(sltu)

static inline void rtl_mul(rtlreg_t* dest_hi, rtlreg_t* dest_lo, const rtlreg_t* src1, const rtlreg_t* src2) {
  asm volatile("mul %3" : "=d"(*dest_hi), "=a"(*dest_lo) : "a"(*src1), "r"(*src2));
}

static inline void rtl_imul(rtlreg_t* dest_hi, rtlreg_t* dest_lo, const rtlreg_t* src1, const rtlreg_t* src2) {
  asm volatile("imul %3" : "=d"(*dest_hi), "=a"(*dest_lo) : "a"(*src1), "r"(*src2));
}

static inline void rtl_div(rtlreg_t* q, rtlreg_t* r, const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  asm volatile("div %4" : "=a"(*q), "=d"(*r) : "d"(*src1_hi), "a"(*src1_lo), "r"(*src2));
}

static inline void rtl_idiv(rtlreg_t* q, rtlreg_t* r, const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  asm volatile("idiv %4" : "=a"(*q), "=d"(*r) : "d"(*src1_hi), "a"(*src1_lo), "r"(*src2));
}

static inline void rtl_lm(rtlreg_t *dest, const rtlreg_t* addr, int len) {
  *dest = vaddr_read(*addr, len);
}

static inline void rtl_sm(rtlreg_t* addr, int len, const rtlreg_t* src1) {
  vaddr_write(*addr, len, *src1);
}

static inline void rtl_lr_b(rtlreg_t* dest, int r) {
  *dest = reg_b(r);
}

static inline void rtl_lr_w(rtlreg_t* dest, int r) {
  *dest = reg_w(r);
}

static inline void rtl_lr_l(rtlreg_t* dest, int r) {
  *dest = reg_l(r);
}

static inline void rtl_sr_b(int r, const rtlreg_t* src1) {
  reg_b(r) = *src1;
}

static inline void rtl_sr_w(int r, const rtlreg_t* src1) {
  reg_w(r) = *src1;
}

static inline void rtl_sr_l(int r, const rtlreg_t* src1) {
  reg_l(r) = *src1;
}

/* RTL psuedo instructions */

static inline void rtl_lr(rtlreg_t* dest, int r, int width) {
  switch (width) {
    case 4: rtl_lr_l(dest, r); return;
    case 1: rtl_lr_b(dest, r); return;
    case 2: rtl_lr_w(dest, r); return;
    default: assert(0);
  }
}

static inline void rtl_sr(int r, int width, const rtlreg_t* src1) {
  switch (width) {
    case 4: rtl_sr_l(r, src1); return;
    case 1: rtl_sr_b(r, src1); return;
    case 2: rtl_sr_w(r, src1); return;
    default: assert(0);
  }
}

#define make_rtl_setget_eflags(f) \
  static inline void concat(rtl_set_, f) (const rtlreg_t* src) { \
    cpu.eflags.f = *src; \
  } \
  static inline void concat(rtl_get_, f) (rtlreg_t* dest) { \
    *dest = cpu.eflags.f; \
  }

make_rtl_setget_eflags(CF)
make_rtl_setget_eflags(OF)
make_rtl_setget_eflags(ZF)
make_rtl_setget_eflags(SF)

/*Pa2.1 Begin*/
static inline void rtl_mv(rtlreg_t* dest, const rtlreg_t *src1) {
  // dest <- src1
  *dest = *src1;
}

static inline void rtl_not(rtlreg_t* dest) {
  // dest <- ~dest
  *dest = ~(*dest);
}
/*Pa2.1 End*/
/*对于8位到32位的符号扩展：最高位是1则扩展后高位全为1，否则则高位全为0
如果8位数是0x7f(01111111)，扩展后是0x0000007f
如果8位数是0x81(10000001)，扩展后是0xffffff81
*/
static inline void rtl_sext(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- signext(src1[(width * 8 - 1) .. 0])
  // 根据源操作数的宽度进行符号扩展
  
  // // 首先获取源操作数的有效位
  // *dest = *src1 & ((1u << (width * 8)) - 1);
  
  // // 如果最高位是1，需要进行符号扩展
  // if (*dest & (1u << (width * 8 - 1))) {
  //   // 将高位全部置为1
  //   *dest |= (~0u) << (width * 8);
  // }
  rtl_li(&t1,32-width*8);
  rtl_shl(dest,src1,&t1);
  rtl_sar(dest,dest,&t1);
}

static inline void rtl_push(const rtlreg_t* src1) {
  // esp <- esp - 4
  // M[esp] <- src1
  /*Pa2.1 Begin*/
  cpu.esp -= 4;//将栈指针esp减4（因为是32位系统，每个数据占4字节）
  rtl_sm(&cpu.esp, 4, src1);//将src1指向的数据写入新的栈顶位置
  /*Pa2.1 end*/
}

 //Pa2.1 0x5d pop %ebp指令。
static inline void rtl_pop(rtlreg_t* dest) {
  // dest <- M[esp]
  // esp <- esp + 4
  rtl_lm(dest, &cpu.esp, 4);  // 从栈顶读取4字节数据
  cpu.esp += 4;  // 栈指针加4
}
/*Pa2.1 Begin*/

// 检查值是否为0，结果存入dest
static inline void rtl_eq0(rtlreg_t* dest, const rtlreg_t* src1) {
  *dest = (*src1 == 0) ? 1 : 0;
}

// 检查值是否等于立即数imm
static inline void rtl_eqi(rtlreg_t* dest, const rtlreg_t* src1, int imm) {
  *dest = (*src1 == imm) ? 1 : 0;
}

// 检查值是否不为0
static inline void rtl_neq0(rtlreg_t* dest, const rtlreg_t* src1) {
  *dest = (*src1 != 0) ? 1 : 0;
}

// 获取src1指定宽度的最高位（符号位）
static inline void rtl_msb(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  *dest = (*src1 >> (width * 8 - 1)) & 0x1;
}

// 根据运算结果更新ZF标志位
static inline void rtl_update_ZF(const rtlreg_t* result, int width) {
  rtl_eq0(&t0, result);
  rtl_set_ZF(&t0);
}

// 根据运算结果更新SF标志位
static inline void rtl_update_SF(const rtlreg_t* result, int width) {
  rtl_msb(&t0, result, width);
  rtl_set_SF(&t0);
}
/*Pa2.1 End*/

/*Pa2.1 Begin*/
static inline void rtl_update_ZFSF(const rtlreg_t* result, int width) {
  // 更新ZF
  rtl_eq0(&t0, result);
  rtl_set_ZF(&t0);
  // 更新SF (检查最高位)
  rtl_msb(&t0, result, width);
  rtl_set_SF(&t0);
}

// // CF (Carry Flag): 表示无符号运算是否产生进位
// static inline void rtl_set_CF(const rtlreg_t* src) {
//   cpu.eflags.CF = *src;
// }

// // OF (Overflow Flag): 表示有符号运算是否溢出
// static inline void rtl_set_OF(const rtlreg_t* src) {
//   cpu.eflags.OF = *src;
// }

// // ZF (Zero Flag): 表示结果是否为0
// static inline void rtl_set_ZF(const rtlreg_t* src) {
//   cpu.eflags.ZF = *src;
// }

// // SF (Sign Flag): 表示结果是否为负数
// static inline void rtl_set_SF(const rtlreg_t* src) {
//   cpu.eflags.SF = *src;
// }
/*Pa2.1 End*/
#endif
