#include "cpu/rtl.h"

/* Condition Code */
//jcc指令通过rtl_setcc来根据条件码决定是否跳转
void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  // subcode的最低位用来表示是否需要反转结果
  bool invert = subcode & 0x1;
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // TODO: Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
  // 使用subcode的高3位来判断具体的条件类型
  switch (subcode & 0xe) {
    case CC_O:// 溢出
      rtl_get_OF(dest);// 获取溢出标志位
      break;
    case CC_B:// 低于（无符号）
      rtl_get_CF(dest);// 获取进位标志位
      break;
    case CC_E:// 等于
      rtl_get_ZF(dest);// 获取零标志位
      break;
    case CC_BE:// 低于或等于（无符号）
      rtl_get_CF(&t0);    // 获取进位标志位，存入临时变量t0
      rtl_get_ZF(&t1);    // 获取零标志位，存入临时变量t1
      rtl_or(dest, &t0, &t1);  // 合并两个条件
      break;
    case CC_S:
      rtl_get_SF(dest);// 获取符号标志位
      break;
    case CC_L:// 低于（有符号）
      rtl_get_SF(&t0);    // 获取符号标志位，存入临时变量t0
      rtl_get_OF(&t1);    // 获取溢出标志位，存入临时变量t1
      rtl_xor(dest, &t0, &t1);  // 合并两个条件
      break;
    case CC_LE:// 低于或等于（有符号）
      rtl_get_SF(&t0);    // 获取符号标志位，存入临时变量t0
      rtl_get_OF(&t1);    // 获取溢出标志位，存入临时变量t1
      rtl_xor(&t2, &t0, &t1);  // 合并两个条件
      rtl_get_ZF(&t3);    // 获取零标志位，存入临时变量t3
      rtl_or(dest, &t2, &t3);  // 合并两个条件
      break;
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
  }
  // 如果需要反转结果（比如jne是je的反转）
  if (invert) {
    rtl_xori(dest, dest, 0x1);
  }
}
