#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  // (a_real * 2^16) * (b_real * 2^16) = (a_real * b_real) * 2^32
  // 期望结果: (a_real * b_real) * 2^16
  // 因此，(a * b) / 2^16  (即右移16位)
  // 中间乘积可能达到64位
  printf("DEBUG: F_mul_F called with a=0x%x (%d), b=0x%x (%d)\n", a, a, b, b);
  int64_t temp_prod = (int64_t)a * b;
  FLOAT result = (FLOAT)(temp_prod >> 16);
  printf("DEBUG: F_mul_F: temp_prod=0x%llx (%lld), result=0x%x (%d)\n", (long long)temp_prod, (long long)temp_prod, result, result);
  return result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    assert(b != 0);
    // 使用32位运算实现定点数除法
    // 为了保持精度，我们需要：
    // 1. 检查溢出风险
    // 2. 根据需要调整移位
    
    int sign = 1;
    if (a < 0) {
        sign = -sign;
        a = -a;
    }
    if (b < 0) {
        sign = -sign;
        b = -b;
    }

    // 计算需要移位的位数，避免溢出
    // 找到a的最高位
    int a_shift = 0;
    FLOAT a_temp = a;
    while (a_temp > 0 && a_shift < 16) {
        a_temp >>= 1;
        a_shift++;
    }

    // 根据a的值调整移位
    // 如果a比较大，我们减少左移的位数以防溢出
    int shift = 16;
    if (a_shift > 16) {
        shift = 32 - a_shift;  // 确保不会溢出
    }

    // 执行除法，结果需要左移以达到定点数的精度
    FLOAT result;
    if (shift > 0) {
        result = ((a << shift) / b) << (16 - shift);
    } else {
        result = (a / b) << 16;
    }

    return sign * result;
}

FLOAT f2F(float ft) {
  // 不使用浮点数运算，直接使用位操作
  union {
    float f;
    uint32_t i;
  } u;
  u.f = ft;
  uint32_t i = u.i;

  // 提取符号位、指数和尾数
  int sign = (i >> 31) & 1;
  int exp = ((i >> 23) & 0xff) - 127;  // 去掉偏移量
  uint32_t frac = (i & 0x7fffff) | 0x800000;  // 加上隐含的1

  // 根据指数调整尾数
  if (exp >= -16) {
    if (exp >= 15) {
      // 溢出检查
      return sign ? -0x7fffffff : 0x7fffffff;
    }
    frac <<= (exp + 16);
  } else {
    // 对于非常小的数，直接返回0
    return 0;
  }

  // 应用符号位
  return sign ? -(int32_t)frac : (int32_t)frac;
}

FLOAT Fabs(FLOAT a) {
  // FLOAT 为 int 类型，取绝对值
  // printf("DEBUG: Fabs called with a=0x%x (%d)\n", a, a); // Enable if needed
  return (a < 0) ? -a : a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  if (x <= 0) return 0;
  
  FLOAT t = x >> 1;  // 初始猜测值
  FLOAT last_t;
  
  do {
    last_t = t;
    t = (t + F_div_F(x, t)) >> 1;  // 牛顿迭代法
  } while(Fabs(t - last_t) > 0x100);  // 精度阈值
  
  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  // 我们只计算 x^(1/3)，忽略y参数
  if (x <= 0) return 0;
  
  FLOAT t = x >> 1;  // 初始猜测值
  FLOAT last_t;
  
  do {
    last_t = t;
    // t = t - (t*t*t - x)/(3*t*t)
    FLOAT t2 = F_mul_F(t, t);
    FLOAT t3 = F_mul_F(t2, t);
    FLOAT diff = t3 - x;
    FLOAT denom = F_mul_F(3 << 16, t2);  // 3*t^2
    t = t - F_div_F(diff, denom);
  } while(Fabs(t - last_t) > 0x100);  // 精度阈值
  
  return t;
}
