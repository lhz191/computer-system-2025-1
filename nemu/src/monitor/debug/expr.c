#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
/*PA1 part2 Begin*/
enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_AND, TK_OR, TK_NOT, TK_NUM, TK_HEX, TK_REG, TK_PLUS, TK_MINUS, TK_MUL, TK_DIV, TK_LPAREN, TK_RPAREN, TK_DEREF, TK_NEG
  /* TODO: Add more token types */
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces,这里的+是一个量词！，在<<正则表达式>>中，+ 是一个量词，表示前面的字符或子表达式可以出现一次或多次。
  {"\\+", TK_PLUS},     // plus
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"-", TK_MINUS},      // minus，不需要转义: 减号 - 在正则表达式中没有特殊的含义
  {"&&", TK_AND},       // and
  {"\\|\\|", TK_OR},    // or
  {"!", TK_NOT},        // not
  {"\\*", TK_MUL},      // multiply
  {"/", TK_DIV},        // divide
  {"\\(", TK_LPAREN},   // left parenthesis
  {"\\)", TK_RPAREN},   // right parenthesis
  {"0[xX][0-9a-fA-F]+", TK_HEX},  // hexadecimal numbers
  {"\\$[a-zA-Z]+", TK_REG},       // registers
  {"[0-9]+", TK_NUM}    // numbers
};
/*PA1 part2 End*/

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;


/*PA1 part2 Begin*/
static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;  // 忽略空格
          case TK_NUM:
          case TK_HEX:
          case TK_REG:
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';  // 确保字符串以 '\0' 结尾
            nr_token++;
            break;
          // case TK_MUL://按照指导书的意思，make_token先不对乘法和解引用进行区分，后续expr阶段进行区分
          //   if (nr_token == 0 || (tokens[nr_token - 1].type != TK_NUM && tokens[nr_token - 1].type != TK_HEX && tokens[nr_token - 1].type != TK_REG && tokens[nr_token - 1].type != TK_RPAREN)) {
          //     tokens[nr_token].type = TK_DEREF;  // 视为解引用
          //   } else {
          //     tokens[nr_token].type = TK_MUL;  // 视为乘法
          //   }
          //   nr_token++;
          //   break;
          default:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
/*PA1 part2 End*/
/*PA1 part2 Begin*/
bool check_parentheses(int p, int q) {
  if (tokens[p].type != TK_LPAREN || tokens[q].type != TK_RPAREN) {
    return false;
  }

  int balance = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_LPAREN) balance++;
    if (tokens[i].type == TK_RPAREN) balance--;
    if (balance == 0 && i < q) return false; // 中途平衡，说明不是包围
  }

  return balance == 0;
}

/*PA1 part2 End*/
/*PA1 part2 Begin*/

int find_dominant_operator(int p, int q) {
  int op = -1;
  int min_priority = 10; // 假设一个较大的初始优先级
  int balance = 0;
  printf("find_dominant_operator called with p: %d, q: %d\n", p, q); // 打印 p 和 q 的值
  for (int i = p; i <= q; i++) {
      if (tokens[i].type == TK_LPAREN) balance++;
      if (tokens[i].type == TK_RPAREN) balance--;

      if (balance == 0) {
        int priority = 0;
        switch (tokens[i].type) {
          case TK_DEREF:
          case TK_NEG: priority = 7; break; // 解引用和负号
          case TK_NOT: priority = 6; break; // 逻辑非
          case TK_MUL:
          case TK_DIV: priority = 5; break; // 乘法和除法
          case TK_PLUS:
          case TK_MINUS: priority = 4; break; // 加法和减法
          case TK_EQ:
          case TK_NEQ: priority = 3; break; // 比较运算符
          case TK_AND: priority = 2; break; // 逻辑与
          case TK_OR: priority = 1; break; // 逻辑或
          // 其他运算符的优先级
          default: priority = 11; break;
      }



      if (priority <= min_priority) {
        //<=非常重要，当有多个运算符的优先级都是最低时, 根据结合性, 最后被结合的运算符才是dominant operator. 
      //不加等于的话，会导致计算出错
        min_priority = priority;
        op = i;
      }
    }
  }

  return op;
}
/*PA1 part2 End*/
/*PA1 part2 Begin*/

uint32_t get_reg_val(const char *reg_name) {
  if (strcmp(reg_name, "$eax") == 0) return cpu.eax;
  if (strcmp(reg_name, "$ecx") == 0) return cpu.ecx;
  if (strcmp(reg_name, "$edx") == 0) return cpu.edx;
  if (strcmp(reg_name, "$ebx") == 0) return cpu.ebx;
  if (strcmp(reg_name, "$esp") == 0) return cpu.esp;
  if (strcmp(reg_name, "$ebp") == 0) return cpu.ebp;
  if (strcmp(reg_name, "$esi") == 0) return cpu.esi;
  if (strcmp(reg_name, "$edi") == 0) return cpu.edi;
  if (strcmp(reg_name, "$eip") == 0) return cpu.eip; 
  // 如果寄存器名称无效，处理错误
  assert(0 && "Invalid register name");
  return 0; // 仅用于消除编译器警告
}
uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    *success = false;
    return 0;
  }
  else if (p == q) {
    // 单个token，应该是一个数字或寄存器
    if (tokens[p].type == TK_NUM) {
      return strtoul(tokens[p].str, NULL, 10);
    }
    else if (tokens[p].type == TK_HEX) {
      return strtoul(tokens[p].str, NULL, 16);
    }
    else if (tokens[p].type == TK_REG) {
      // 处理寄存器值
      // 函数get_reg_val来获取寄存器的值
      return get_reg_val(tokens[p].str);
    }
    else {
      *success = false;
      return 0;
    }
  }
  else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1, success);
  }
  else {
    int op = find_dominant_operator(p, q);
    printf("Dominant operator position: %d\n", op);
    if (op == -1) {
      *success = false;
      return 0;
    }

    uint32_t val1 = 0;
    if (tokens[op].type != TK_NEG && tokens[op].type != TK_DEREF && tokens[op].type != TK_NOT) {
      val1 = eval(p, op - 1, success);
    }
    uint32_t val2 = eval(op + 1, q, success);
    printf("tokens[op].type: %d\n", tokens[op].type);
    // printf
    switch (tokens[op].type) {
      case TK_PLUS: return val1 + val2;
      case TK_MINUS: return val1 - val2;
      case TK_MUL: return val1 * val2;
      case TK_DIV: return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      case TK_OR: return val1 || val2;
      case TK_NOT: return !val2;
      case TK_DEREF: return vaddr_read(val2, 4); 
      case TK_NEG: return -val2;
      default: assert(0);
    }
  }
}

/*PA1 part2 End*/
/*PA1 part2 Begin*/
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();

  // return 0;
  // 识别解引用和负号
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == TK_MUL && (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_REG && tokens[i - 1].type != TK_RPAREN))) {
      tokens[i].type = TK_DEREF;
    }
    if (tokens[i].type == TK_MINUS && (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_REG && tokens[i - 1].type != TK_RPAREN))) {
      tokens[i].type = TK_NEG;
    }
  }

  return eval(0, nr_token - 1, success);
}

/*PA1 part2 End*/