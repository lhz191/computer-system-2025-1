#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  // (a_real * 2^16) * (b_real * 2^16) = (a_real * b_real) * 2^32
  // 期望结果: (a_real * b_real) * 2^16
  // 因此，(a * b) / 2^16  (即右移16位)
  // 中间乘积可能达到64位
  // printf("DEBUG: F_mul_F called with a=0x%x (%d), b=0x%x (%d)\\n", a, a, b, b);
  
  if (a == 0 || b == 0) {
    return 0;
  }

  int64_t temp_prod = (int64_t)a * b;

  // 检查缩放后的结果是否溢出32位
  // (temp_prod / 2^16) should be within [INT32_MIN, INT32_MAX]
  // So, temp_prod should be within [INT32_MIN * 2^16, INT32_MAX * 2^16]
  
  const int64_t min_val_scaled = (int64_t)INT32_MIN << 16;
  const int64_t max_val_scaled = (int64_t)INT32_MAX << 16;

  if (temp_prod < min_val_scaled) {
    // printf("DEBUG: F_mul_F: Underflow detected. temp_prod=0x%llx, clamping to INT32_MIN\\n", (long long)temp_prod);
    return INT32_MIN;
  }
  if (temp_prod > max_val_scaled) {
    // printf("DEBUG: F_mul_F: Overflow detected. temp_prod=0x%llx, clamping to INT32_MAX\\n", (long long)temp_prod);
    return INT32_MAX;
  }

  FLOAT result = (FLOAT)(temp_prod >> 16);
  
  // printf("DEBUG: F_mul_F: temp_prod=0x%llx (%lld), result=0x%x (%d)\\n", (long long)temp_prod, (long long)temp_prod, result, result);
  return result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    assert(b != 0);
    // printf("DEBUG: F_div_F called with a=0x%x (%d), b=0x%x (%d)\\n", a, a, b, b);

    // (a_real / b_real) * 2^16
    // = ( (a/2^16) / (b/2^16) ) * 2^16
    // = (a/b) * 2^16
    // So, we calculate (a * 2^16) / b, where a and b are already scaled by 2^16
    // This becomes ((a_int * 2^16) * 2^16) / (b_int * 2^16) for actual unscaled values
    // which is (a_int / b_int) * 2^16.
    // Using current a, b (which are FLOATs, i.e. scaled by 2^16):
    // ( (a_input_scaled_by_2_16) << 16 ) / b_input_scaled_by_2_16

    int64_t temp_a = (int64_t)a << 16; // Intermediate value can be 32+16 = 48 bits + sign
    int64_t quotient = temp_a / b;     // b is FLOAT (32-bit int). Result can be 48 bits.

    // Clamp the 64-bit quotient to 32-bit range
    if (quotient < INT32_MIN) {
        // printf("DEBUG: F_div_F: Underflow detected. quotient=0x%llx, clamping to INT32_MIN\\n", (long long)quotient);
        return INT32_MIN;
    }
    if (quotient > INT32_MAX) {
        // printf("DEBUG: F_div_F: Overflow detected. quotient=0x%llx, clamping to INT32_MAX\\n", (long long)quotient);
        return INT32_MAX;
    }

    FLOAT result = (FLOAT)quotient;
    // printf("DEBUG: F_div_F: temp_a(a<<16)=0x%llx, b=0x%x, quotient=0x%llx, result=0x%x\\n", (long long)temp_a, b, (long long)quotient, result);
    return result;
}

FLOAT f2F(float ft) {
  printf("DEBUG: f2F called with float_val=%.8f\n", ft);
  if (ft == 0.0f) {
    printf("DEBUG: f2F: input 0.0f, returning 0\n");
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
  printf("DEBUG: f2F: float_val=%.8f, bit_repr=0x%08x\n", ft, val);

  int sign_bit = (val >> 31) & 1;         // 符号位
  int exponent_bits = (val >> 23) & 0xFF; // 指数部分 (8位)
  uint32_t fraction_bits = val & 0x7FFFFF; // 尾数部分 (23位)

  FLOAT result_unsigned;

  if (exponent_bits == 0xFF) { // 无穷大或NaN
    printf("DEBUG: f2F: Inf/NaN detected, returning 0\n");
    result_unsigned = 0;
  } else if (exponent_bits == 0) { // 非规格化数或零
    if (fraction_bits == 0) {
        printf("DEBUG: f2F: Denorm/Zero (fraction is 0), returning 0\n");
        result_unsigned = 0;
    } else {
        printf("DEBUG: f2F: Denormalized number, returning 0 (as per current logic)\n");
        result_unsigned = 0;
    }
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
    printf("DEBUG: f2F: norm: sign=%d, exp_bits=0x%x (act_exp=%d), frac_bits=0x%x, act_mant=0x%x, shift=%d\n",
           sign_bit, exponent_bits, actual_exponent, fraction_bits, actual_mantissa, shift);

    if (shift >= 0) {
      if (shift < 31) {
          result_unsigned = actual_mantissa << shift;
      } else {
          printf("DEBUG: f2F: WARN: Large positive shift (%d), result_unsigned potentially too large. Setting to 0 for safety, but this indicates 'ft' was out of FLOAT range.\n", shift);
          result_unsigned = 0;
      }
    } else {
      int rshift = -shift;
      if (rshift < 32) {
          result_unsigned = actual_mantissa >> rshift;
      } else {
          printf("DEBUG: f2F: WARN: Large negative shift (rshift %d), result_unsigned will be 0.\n", rshift);
          result_unsigned = 0;
      }
    }
    printf("DEBUG: f2F: norm: result_unsigned=0x%x (%u)\n", result_unsigned, result_unsigned);
  }

  FLOAT final_result = sign_bit ? -result_unsigned : result_unsigned;
  printf("DEBUG: f2F: final_result=0x%x (%d) for float_val=%.8f\n", final_result, final_result, ft);
  return final_result;
}

FLOAT Fabs(FLOAT a) {
  // FLOAT 为 int 类型，取绝对值
  // printf("DEBUG: Fabs called with a=0x%x (%d)\n", a, a); // Enable if needed
  return (a < 0) ? -a : a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  printf("DEBUG: Fsqrt called with x=0x%x (%d)\n", x, x);
  FLOAT dt, t = int2F(2);
  int iteration_count = 0;
  const int max_iterations = 1000; // Safety break for very long loops

  FLOAT loop_limit = f2F(1e-4); // Expected to be 6 for 1e-4
  printf("DEBUG: Fsqrt: loop_limit for Fabs(dt) > is 0x%x (%d)\n", loop_limit, loop_limit);

  do {
    FLOAT prev_t = t;
    FLOAT term1 = F_div_F(x, t);
    FLOAT term_diff = term1 - t;
    dt = F_div_int(term_diff, 2); // Using F_div_int as it's FLOAT / int
    // dt = term_diff / 2; // Or direct integer division if F_div_int is just that

    t += dt;
    iteration_count++;

    printf("DEBUG: Fsqrt iter %d: x=0x%x, t_prev=0x%x, term1(x/t)=0x%x, term_diff=0x%x, dt=0x%x, t_new=0x%x, Fabs(dt)=0x%x\n",
           iteration_count, x, prev_t, term1, term_diff, dt, t, Fabs(dt));

    if (iteration_count > max_iterations) {
        printf("DEBUG: Fsqrt: MAX ITERATIONS REACHED for x=0x%x. Returning current t=0x%x\n", x, t);
        break;
    }
    if (dt == 0 && prev_t == t) { // dt is 0, no change, break to avoid infinite loop if limit is not met.
                                  // This check helps if Fabs(dt) is small but non-zero and dt keeps flipping sign around zero.
        printf("DEBUG: Fsqrt: dt is 0 and t unchanged for x=0x%x. Loop terminating. Fabs(dt)=0x%x\n", x, Fabs(dt));
        break;
    }

  } while(Fabs(dt) > loop_limit);

  printf("DEBUG: Fsqrt: Loop finished for x=0x%x after %d iterations. Result t=0x%x (%d)\n", x, iteration_count, t, t);
  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  printf("DEBUG: Fpow called with x=0x%x (%d), y=0x%x (%d) (y is unused by this version)\n", x, x, y, y);
  
  // 处理特殊情况
  if (x == 0) return 0;
  if (x < 0) return 0;  // 暂时不处理负数
  
  FLOAT t2, dt, t = int2F(2);
  int iteration_count = 0;
  const int max_iterations = 1000; // Safety break

  FLOAT loop_limit = f2F(1e-4); // Expected to be 6
  printf("DEBUG: Fpow: loop_limit for Fabs(dt) > is 0x%x (%d)\n", loop_limit, loop_limit);

  do {
    FLOAT prev_t = t;
    t2 = F_mul_F(t, t);
    if (t2 == 0) {  // 如果t2变得太小，停止迭代
      printf("DEBUG: Fpow: t2 became 0, stopping iteration\n");
      return prev_t;
    }
    
    FLOAT term1 = F_div_F(x, t2);
    FLOAT term_diff = term1 - t;
    dt = F_div_int(term_diff, 3);
    
    // 如果dt太小，提前结束
    if (dt == 0) {
      printf("DEBUG: Fpow: dt became 0, stopping iteration\n");
      return t;
    }
    
    t += dt;
    iteration_count++;

    printf("DEBUG: Fpow iter %d: x=0x%x, t_prev=0x%x, t2(t*t)=0x%x, term1(x/t2)=0x%x, term_diff=0x%x, dt=0x%x, t_new=0x%x, Fabs(dt)=0x%x\n",
           iteration_count, x, prev_t, t2, term1, term_diff, dt, t, Fabs(dt));

    if (iteration_count > max_iterations) {
      printf("DEBUG: Fpow: MAX ITERATIONS REACHED for x=0x%x. Returning current t=0x%x\n", x, t);
      break;
    }
    
    if (prev_t == t) {
      printf("DEBUG: Fpow: t unchanged, stopping iteration\n");
      break;
    }

  } while(Fabs(dt) > loop_limit);

  printf("DEBUG: Fpow: Loop finished for x=0x%x after %d iterations. Result t=0x%x (%d)\n", x, iteration_count, t, t);
  return t;
}
