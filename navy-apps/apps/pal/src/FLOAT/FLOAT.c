#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  if (a == 0 || b == 0) {
    return 0;
  }
  // 分别打印a和b的整数和小数部分
  printf("F_mul_F: %d.%04d * %d.%04d = ", 
         a >> 16, 
         (int)((a & 0xFFFF) * 10000LL >> 16),
         b >> 16, 
         (int)((b & 0xFFFF) * 10000LL >> 16));
  int64_t temp_prod = (int64_t)a * b;

  const int64_t min_val_scaled = (int64_t)INT32_MIN << 16;
  const int64_t max_val_scaled = (int64_t)INT32_MAX << 16;

  if (temp_prod < min_val_scaled) {
    return INT32_MIN;
  }
  if (temp_prod > max_val_scaled) {
    return INT32_MAX;
  }

  FLOAT result = (FLOAT)(temp_prod >> 16);
  // 打印结果的整数和小数部分
  printf("%d.%04d\n", 
         result >> 16,
         (int)((result & 0xFFFF) * 10000LL >> 16));
  return result;
}


FLOAT F_div_F(FLOAT a, FLOAT b) {
    assert(b != 0);
    // 分别打印a和b的整数和小数部分
    printf("F_div_F: %d.%04d / %d.%04d = ", 
           a >> 16, 
           (int)((a & 0xFFFF) * 10000LL >> 16),
           b >> 16, 
           (int)((b & 0xFFFF) * 10000LL >> 16));
    int64_t temp_a = (int64_t)a << 16;
    int64_t quotient = temp_a / b;    

    if (quotient < INT32_MIN) {
        return INT32_MIN;
    }
    if (quotient > INT32_MAX) {
        return INT32_MAX;
    }

    FLOAT result = (FLOAT)quotient;
    // 打印结果的整数和小数部分
    printf("%d.%04d\n", 
           result >> 16,
           (int)((result & 0xFFFF) * 10000LL >> 16));
    return result;
}

FLOAT f2F(float ft) {
  // 1. 初始的浮点比较：
  // 这个比较仍然是浮点操作。
  // 如果想完全避免，可以先获取位表示 val，然后判断 val 是否为0的位表示。
  if (ft == 0.0f) { 
    return 0;
  }

  uint32_t val;
  
  // 2. 获取 float ft 的32位整数表示：
  // 不使用 union，改用指针转换的方式
  val = *(uint32_t *)&ft; 
  // 或者使用 memcpy (更安全，避免严格别名违规，尽管对于简单类型转换通常编译器会优化好):
  // memcpy(&val, &ft, sizeof(val));

  // 3. 从这里开始，后续的转换逻辑完全基于整数 val 的位操作：
  // 这部分代码和你原来的一样，它不依赖C语言的浮点算术运算。
  int sign_bit = (val >> 31) & 1;
  int exponent_bits = (val >> 23) & 0xFF;
  uint32_t fraction_bits = val & 0x7FFFFF;
  FLOAT result_unsigned;

  if (exponent_bits == 0xFF) {
    assert(0 && "Error: Cannot convert Inf/NaN to FLOAT");
  } 
  else if (exponent_bits == 0) {
    if (fraction_bits == 0) {
        result_unsigned = 0;
    } else {
        result_unsigned = 0; // 对于非规格化数，这里也处理为0
    }
  } 
  else {
    int actual_exponent = exponent_bits - 127;
    uint32_t actual_mantissa = (1 << 23) | fraction_bits;
    int shift = actual_exponent - 7; // (23 - 16)

    if (shift >= 0) {
      if (shift < 31) { // 防止溢出到符号位或完全移出
          result_unsigned = actual_mantissa << shift;
      } else {
          // 如果左移位数过多，导致结果超出FLOAT能表达的正数范围（或变为0，如果符号也移没了）
          // 根据你的FLOAT定义，可能需要一个饱和值或就设为0
          result_unsigned = (actual_mantissa == 0) ? 0 : 0x7FFFFFFF;
          if (shift >= 31 && actual_mantissa != 0) { /* 可能表示溢出 */ } else { result_unsigned = 0; }
      }
    } else {
      int rshift = -shift;
      if (rshift < 32) { // 考虑实际尾数是24位，如果右移过多也会变0
          result_unsigned = actual_mantissa >> rshift;
      } else {
          result_unsigned = 0;
      }
    }
  }

  FLOAT final_result = sign_bit ? -result_unsigned : result_unsigned;
  return final_result;
}

FLOAT Fabs(FLOAT a) {
  // FLOAT 为 int 类型，取绝对值
  return (a < 0) ? -a : a;
}

//以下实验中没要求，为扩展实现
FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);
  int iteration_count = 0;
  const int max_iterations = 1000;

  FLOAT loop_limit = f2F(1e-4); // Expected to be 6 for 1e-4

  do {
    FLOAT prev_t = t;
    FLOAT term1 = F_div_F(x, t);
    FLOAT term_diff = term1 - t;
    dt = F_div_int(term_diff, 2);

    t += dt;
    iteration_count++;

    if (iteration_count > max_iterations) {
        break;
    }
    if (dt == 0 && prev_t == t) { //找到精确解
        break;
    }

  } while(Fabs(dt) > loop_limit);//差距/2小于1e-4时，停止迭代
  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  // 处理特殊情况
  if (x == 0) return 0;
  if (x < 0) return 0;  // 暂时不处理负数
  
  FLOAT t2, dt, t = int2F(2);
  int iteration_count = 0;
  const int max_iterations = 1000; // Safety break

  FLOAT loop_limit = f2F(1e-4); // Expected to be 6
  do {
    FLOAT prev_t = t;
    t2 = F_mul_F(t, t);
    if (t2 == 0) {  // 如果t2变得太小，停止迭代
      return prev_t;
    }
    FLOAT term1 = F_div_F(x, t2);
    FLOAT term_diff = term1 - t;
    dt = F_div_int(term_diff, 3);
    // 如果dt太小，提前结束
    if (dt == 0) {
      return t;
    }
    
    t += dt;
    iteration_count++;
    if (iteration_count > max_iterations) {
      break;
    }
    if (prev_t == t) {
      break;
    }

  } while(Fabs(dt) > loop_limit);
  return t;
}
