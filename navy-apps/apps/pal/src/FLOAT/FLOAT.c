#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  printf("F_mul_F input: a = %d (0x%x), b = %d (0x%x)\\n", a, a, b, b);

  if (a == 0 || b == 0) {
    return 0;
  }
  //为了防止这个中间乘积溢出32位，所以用了 int64_t
  int64_t temp_prod = (int64_t)a * b;
  FLOAT result = (FLOAT)(temp_prod >> 16);
  printf("F_mul_F: result (temp_prod >> 16) = %d (0x%x)\\n", result, result);
  return result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    assert(b != 0);

    int64_t temp_a = (int64_t)a << 16;
    int64_t quotient = temp_a / b;    

    if (quotient < INT32_MIN) {
        return INT32_MIN;
    }
    if (quotient > INT32_MAX) {
        return INT32_MAX;
    }

    FLOAT result = (FLOAT)quotient;
    return result;
}

FLOAT f2F(float ft) {
  if (ft == 0.0f) {
    return 0;
  }

  uint32_t val;
  // 使用 union 获取 float 的位表示，避免直接浮点运算
  union {
    float f_val;
    uint32_t u_val;
  } float_converter;

  float_converter.f_val = ft;
  val = float_converter.u_val;

  int sign_bit = (val >> 31) & 1;         // 符号位
  int exponent_bits = (val >> 23) & 0xFF; // 指数部分 (8位)
  uint32_t fraction_bits = val & 0x7FFFFF; // 尾数部分 (23位)

  FLOAT result_unsigned;

  if (exponent_bits == 0xFF) { // 无穷大或NaN，指数为255-127=128
    //如果尾数全0：±∞ (正无穷或负无穷，取决于符号位)
    //如果尾数不全0：NaN (Not a Number)特殊规定
    // result_unsigned = 0;
    assert(0 && "Error: Cannot convert Inf/NaN to FLOAT");
  } 
  else if (exponent_bits == 0) { // 非规格化数或零。指数为0-127=-127
    if (fraction_bits == 0) {
        result_unsigned = 0;
    } else {
        result_unsigned = 0;
    }
  } 
  else { // 规格化数
    // 实际指数 = exponent_bits - 127
    int actual_exponent = exponent_bits - 127;
    // 实际尾数 (包含隐含的1) = (1 << 23) | fraction_bits (24位整数)
    uint32_t actual_mantissa = (1 << 23) | fraction_bits;
    // IEEE-754:
    // 4.0 = 1.0 × 2^2
    // actual_exponent = 2
    // actual_mantissa = 2^23

    // FLOAT需要:
    // 4.0 × 2^16 = 2^18

    // 需要的移位 = 18 - 23 = -5
    // = actual_exponent(2) - 7
    //这个7是float的尾数位数23-Float的偏移位数16=7，本来要左移7位，但有指数，所以左移指数-7位，指数每大一，便少移一位
    int shift = actual_exponent - 7;

    if (shift >= 0) {
      if (shift < 31) {
          result_unsigned = actual_mantissa << shift;
      } else {
          result_unsigned = 0;
      }
    } else {
      int rshift = -shift;
      if (rshift < 32) {
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
// FLOAT Fsqrt(FLOAT x) {
//   FLOAT dt, t = int2F(2);
//   int iteration_count = 0;
//   const int max_iterations = 1000;

//   FLOAT loop_limit = f2F(1e-4); // Expected to be 6 for 1e-4

//   do {
//     FLOAT prev_t = t;
//     FLOAT term1 = F_div_F(x, t);
//     FLOAT term_diff = term1 - t;
//     dt = F_div_int(term_diff, 2);

//     t += dt;
//     iteration_count++;

//     if (iteration_count > max_iterations) {
//         break;
//     }
//     if (dt == 0 && prev_t == t) { //找到精确解
//         break;
//     }

//   } while(Fabs(dt) > loop_limit);//差距/2小于1e-4时，停止迭代
//   return t;
// }

// FLOAT Fpow(FLOAT x, FLOAT y) {
//   // 处理特殊情况
//   if (x == 0) return 0;
//   if (x < 0) return 0;  // 暂时不处理负数
  
//   FLOAT t2, dt, t = int2F(2);
//   int iteration_count = 0;
//   const int max_iterations = 1000; // Safety break

//   FLOAT loop_limit = f2F(1e-4); // Expected to be 6
//   do {
//     FLOAT prev_t = t;
//     t2 = F_mul_F(t, t);
//     if (t2 == 0) {  // 如果t2变得太小，停止迭代
//       return prev_t;
//     }
//     FLOAT term1 = F_div_F(x, t2);
//     FLOAT term_diff = term1 - t;
//     dt = F_div_int(term_diff, 3);
//     // 如果dt太小，提前结束
//     if (dt == 0) {
//       return t;
//     }
    
//     t += dt;
//     iteration_count++;
//     if (iteration_count > max_iterations) {
//       break;
//     }
//     if (prev_t == t) {
//       break;
//     }

//   } while(Fabs(dt) > loop_limit);
//   return t;
// }
