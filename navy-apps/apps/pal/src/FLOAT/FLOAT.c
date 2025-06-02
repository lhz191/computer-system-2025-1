#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  // (a_real * 2^16) * (b_real * 2^16) = (a_real * b_real) * 2^32
  // 期望结果: (a_real * b_real) * 2^16
  // 因此，(a * b) / 2^16  (即右移16位)
  // 中间乘积可能达到64位
  int64_t temp_prod = (int64_t)a * b;
  return (FLOAT)(temp_prod >> 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  assert(b != 0); // 防止除零
  // 期望结果: (a_real / b_real) * 2^16
  // 计算: (a * 2^16) / b = ((a_real * 2^16) * 2^16) / (b_real * 2^16)
  //                    = (a_real / b_real) * 2^16
  // 被除数先左移16位以保持精度
  int64_t temp_a_scaled = (int64_t)a << 16;
  return (FLOAT)(temp_a_scaled / b);
}

FLOAT f2F(float a) {
  if (a == 0.0f) {
    return 0;
  }

  uint32_t val;
  // 使用 union 获取 float 的位表示，避免直接浮点运算
  union {
    float f_val;
    uint32_t u_val;
  } float_converter;

  float_converter.f_val = a;
  val = float_converter.u_val;

  int sign_bit = (val >> 31) & 1;         // 符号位
  int exponent_bits = (val >> 23) & 0xFF; // 指数部分 (8位)
  uint32_t fraction_bits = val & 0x7FFFFF; // 尾数部分 (23位)

  FLOAT result;

  if (exponent_bits == 0xFF) { // 无穷大或NaN
    return 0; // 按题目说明，此类数不出现或不需特别处理
  } else if (exponent_bits == 0) { // 非规格化数或零
    if (fraction_bits == 0) return 0; // 零
    // 非规格化数极小，转换为FLOAT后通常为0
    result = 0;
  } else { // 规格化数
    // 实际指数 = exponent_bits - 127
    int actual_exponent = exponent_bits - 127;
    // 实际尾数 (包含隐含的1) = (1 << 23) | fraction_bits (24位整数)
    uint32_t actual_mantissa = (1 << 23) | fraction_bits;

    // 转换公式: actual_mantissa * 2^(actual_exponent - 7)
    // (推导: float值 V = M_norm * 2^E_act, 其中 M_norm = actual_mantissa / 2^23)
    // (FLOAT = V * 2^16 = (actual_mantissa / 2^23) * 2^E_act * 2^16)
    // (        = actual_mantissa * 2^(E_act + 16 - 23) = actual_mantissa * 2^(E_act - 7))
    int shift = actual_exponent - 7;

    if (shift >= 0) { // 左移
      // 根据题目说明，结果总能用FLOAT表示，不会发生最终溢出
      // shift 最大约为7 (对应float值 ~2^14)，actual_mantissa (24位) << 7 为31位，可容纳
      if (shift < 31) { 
          result = actual_mantissa << shift;
      } else {
          // 此情况理论上不应发生，若发生则表示原float值过大
          result = 0; 
      }
    } else { // 右移
      int rshift = -shift;
      if (rshift < 32) {
          result = actual_mantissa >> rshift;
      } else {
          result = 0; // 右移位数过多，结果为0
      }
    }
  }

  return sign_bit ? -result : result;
}

FLOAT Fabs(FLOAT a) {
  // FLOAT 为 int 类型，取绝对值
  return (a < 0) ? -a : a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
