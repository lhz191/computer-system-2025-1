#include "cpu/exec.h"
#include "all-instr.h"

typedef struct {
  DHelper decode;
  EHelper execute;
  int width;
} opcode_entry;

#define IDEXW(id, ex, w)   {concat(decode_, id), concat(exec_, ex), w}
#define IDEX(id, ex)       IDEXW(id, ex, 0)
#define EXW(ex, w)         {NULL, concat(exec_, ex), w}
#define EX(ex)             EXW(ex, 0)
#define EMPTY              EX(inv)

static inline void set_width(int width) {
  if (width == 0) {
    width = decoding.is_operand_size_16 ? 2 : 4;
  }
  decoding.src.width = decoding.dest.width = decoding.src2.width = width;
}

/* Instruction Decode and EXecute */
static inline void idex(vaddr_t *eip, opcode_entry *e) {
  /* eip is pointing to the byte next to opcode */
  if (e->decode)
    e->decode(eip);
  e->execute(eip);
}

static make_EHelper(2byte_esc);

#define make_group(name, item0, item1, item2, item3, item4, item5, item6, item7) \
  static opcode_entry concat(opcode_table_, name) [8] = { \
    /* 0x00 */	item0, item1, item2, item3, \
    /* 0x04 */	item4, item5, item6, item7  \
  }; \
static make_EHelper(name) { \
  idex(eip, &concat(opcode_table_, name)[decoding.ext_opcode]); \
}

/* 0x80, 0x81, 0x83 */
make_group(gp1,
    EX(add),  /* 000 */ // ADD Eb/Ev, Ib
    EX(or),   /* 001 */ // OR  Eb/Ev, Ib
    EMPTY,    /* 010 */ // ADC Eb/Ev, Ib
    EX(sbb),  /* 011 */ // SBB Eb/Ev, Ib
    EX(and),  /* 100 */ // AND Eb/Ev, Ib
    EX(sub),  /* 101 */ // SUB Eb/Ev, Ib
    EX(xor),  /* 110 */ // XOR Eb/Ev, Ib
    EX(cmp))  /* 111 */ // CMP Eb/Ev, Ib

  /* 0xc0, 0xc1, 0xd0, 0xd1, 0xd2, 0xd3 */
make_group(gp2,
    EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY)

  /* 0xf6, 0xf7 */
make_group(gp3,
    EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY)

  /* 0xfe */
make_group(gp4,
    EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY)

/*
IDEX用于那些需要从指令中提取操作数信息的指令
EX用于那些不需要额外解码就能执行的指令
*/



/*Pa2.1 Begin*/
/* 0xff */
/*0xff操作码使用了分组（group）的概念，
具体执行哪个指令由ModR/M字节的reg字段（3位）决定。*/
/*gp5组的8个位置对应reg字段的8种可能值（000~111）*/
/*当CPU遇到0xff操作码时，首先会执行一个初始解码步骤，通常使用IDEX(E, gp5)，这一步会解析ModR/M字节。
在这个初始解码之后，CPU会根据ModR/M字节中的reg字段（3位）确定要执行gp5组中的哪一条指令。
对于gp5组中的各个指令，操作数信息已经在初始解码阶段由E解码器提取出来了，所以在组内的实现应该使用EX*/
make_group(gp5,
    EX(inc),        /* 000: inc指令 */
    EX(dec),        /* 001: dec指令 */
    EX(call_rm),    /* 010: call rm指令 */
    EX(call),       /* 011: call指令 */
    EX(jmp_rm),     /* 100: jmp rm指令 */
    EX(jmp),        /* 101: jmp指令 */
    EX(push),       /* 110: push指令 */
    EMPTY           /* 111: 未使用 */
)
/*Pa2.1 End*/

  /* 0x0f 0x01*/
make_group(gp7,
    EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY)

/* TODO: Add more instructions!!! */

opcode_entry opcode_table [512] = {
  /*Pa2.1 0x00 add指令*/
  /* 0x00 */	IDEXW(G2E, add, 1), EMPTY, EMPTY, IDEX(G2E, add),
  /* 0x04 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x08 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /*Pa2.1 0x0f 0xbe：两字节操作码的转义，去两字节操作码表查找 */
  /* 0x0c */	EMPTY, EMPTY, EMPTY, EX(2byte_esc),
  /* 0x10 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x14 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x18 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x1c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x20 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x24 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x28 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x2c */	EMPTY, EMPTY, EMPTY, EMPTY,
  //Pa2.1 0x31: xor r32 to r/m32
  /* 0x30 */	EMPTY, IDEX(G2E, xor), EMPTY, EMPTY, 
  /* 0x34 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /*Pa2.1 0x3b: cmp r/m32 to r32*/
  /* 0x38 */	EMPTY, EMPTY, EMPTY, IDEX(G2E, cmp),
  /*Pa2.1 0x3c cmp imm8 to r/m8*/
  /* 0x3c */	IDEXW(I2a, cmp, 1), IDEX(I2a, cmp), EMPTY, EMPTY,
  /* 0x40 */	IDEX(r, inc), IDEX(r, inc), IDEX(r, inc), IDEX(r, inc),
  /*Pa2.1 0x46: inc %esi*/
  /* 0x44 */	IDEX(r, inc), IDEX(r, inc), IDEX(r, inc), IDEX(r, inc),
  /* 0x48 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x4c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /*Pa2.1
  通过查看x86指令集手册得知的。在x86架构中：
  0x50-0x57这个范围的操作码专门用于push寄存器指令：
  0x50: push %eax
  0x51: push %ecx
  0x52: push %edx
  0x53: push %ebx
  0x54: push %esp
  0x55: push %ebp
  0x56: push %esi
  0x57: push %edi
  Pa2.1*/
  /* 0x50 */	IDEX(r, push), IDEX(r, push), IDEX(r, push), IDEX(r, push),
  /*Pa2.1 Begin*/
  // 0x55 是 push ebp
  /* 0x54 */	IDEX(r, push), IDEX(r, push), IDEX(r, push), IDEX(r, push),
  /*Pa2.1 End*/
  /* 0x58 */	IDEX(r, pop), IDEX(r, pop), IDEX(r, pop), IDEX(r, pop),
  
  //Pa2.1 0x58-5f pop %ebp指令。
  /* 0x5c */	IDEX(r, pop), IDEX(r, pop), IDEX(r, pop), IDEX(r, pop),  
  /* 0x60 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x64 */	EMPTY, EMPTY, EX(operand_size), EMPTY,
  /*Pa2.1 0x68:push imm32*/
  /*Pa2.1 0x6a：push imm8*/
  /*I是一个解码器（decoder），用于处理立即数（Immediate）操作数。make_DopHelper(I) */
  /*IDEX(I, push)：解码32位立即数，IDEXW(SI, push, 1)：解码8位立即数*/
  // 0x6a: push imm8 with sign extension
  /* 0x68 */	IDEX(I, push), EMPTY, IDEXW(SI, push, 1), EMPTY, 
  /* 0x6c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x70 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /*Pa2.1 0x74，0x75 jcc指令*/
  /* 0x74 */	IDEXW(J, jcc, 1), IDEXW(J, jcc, 1), EMPTY, EMPTY,
  /* 0x78 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x7c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x80 */	IDEXW(I2E, gp1, 1), IDEX(I2E, gp1), EMPTY, IDEX(SI2E, gp1),
  /*Pa2.1 0x84 c0 test %al,%al*/
  /* 0x84 */	IDEXW(G2E, test, 1), IDEX(G2E, test), EMPTY, EMPTY,
  /* 0x88 */	IDEXW(mov_G2E, mov, 1), IDEX(mov_G2E, mov), IDEXW(mov_E2G, mov, 1), IDEX(mov_E2G, mov),
  /*Pa2.1 Begin*/
  /* 0x8c */	EMPTY, IDEX(lea_M2G, lea), EMPTY, EMPTY,  // 0x8d是lea指令
  /*Pa2.1 End*/
  /*Pa2.1 0x90 nop指令*/
  /* 0x90 */	EX(nop), EMPTY, EMPTY, EMPTY,
  /* 0x94 */	IDEXW(gp7_E, setcc, 1), EMPTY, EMPTY, EMPTY,
  /* 0x98 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x9c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa0 */	IDEXW(O2a, mov, 1), IDEX(O2a, mov), IDEXW(a2O, mov, 1), IDEX(a2O, mov),
  /* 0xa4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xac */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xb0 */	IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1),
  /* 0xb4 */	IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1),
  /* 0xb8 */	IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov),
  /* 0xbc */	IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov),
  /*Pa2.1 0xc0: ret*/
  /* 0xc0 */	IDEXW(gp2_Ib2E, gp2, 1), IDEX(gp2_Ib2E, gp2), EMPTY, EX(ret),
  /* 0xc4 */	EMPTY, EMPTY, IDEXW(mov_I2E, mov, 1), IDEX(mov_I2E, mov),
  /* 0xc8 */	EMPTY, EX(leave), EMPTY, EMPTY,
  /* 0xcc */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xd0 */	IDEXW(gp2_1_E, gp2, 1), IDEX(gp2_1_E, gp2), IDEXW(gp2_cl2E, gp2, 1), IDEX(gp2_cl2E, gp2),
  /* 0xd4 */	EMPTY, EMPTY, EX(nemu_trap), EMPTY,
  /* 0xd8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xdc */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /*Pa2.1 Begin*/
  /*查阅手册得知，0xe9和0xec都是jmp指令*/
  /* 0xe8 */	IDEX(J, call), IDEX(J, jmp), EMPTY, IDEX(J, jmp),
  /*Pa2.1 End*/
  /* 0xec */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf4 */	EMPTY, EMPTY, IDEXW(E, gp3, 1), IDEX(E, gp3),
  /* 0xf8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xfc */	EMPTY, EMPTY, IDEXW(E, gp4, 1), IDEX(E, gp5),

  /*2 byte_opcode_table */

  /* 0x00 */	EMPTY, IDEX(gp7_E, gp7), EMPTY, EMPTY,
  /* 0x04 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x08 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x0c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x10 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x14 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x18 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x1c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x20 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x24 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x28 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x2c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x30 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x34 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x38 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x3c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x40 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x44 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x48 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x4c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x50 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x54 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x58 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x5c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x60 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x64 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x68 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x6c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x70 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x74 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x78 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x7c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x80 */	EMPTY, EMPTY, EMPTY, EMPTY,
  //Pa2.1 0x84: je rel32 (jump if equal)
  /* 0x84 */	IDEX(J, jcc), EMPTY, EMPTY, EMPTY, 
  /* 0x88 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x8c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x90 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x94 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x98 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x9c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xac */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xb0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xb4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xb8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xbc */	EMPTY, EMPTY, IDEXW(E2G, movsx, 1),  EMPTY,
  /* 0xc0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xc4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xc8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xcc */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xd0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xd4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xd8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xdc */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xec */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xfc */	EMPTY, EMPTY, EMPTY, EMPTY
};

static make_EHelper(2byte_esc) {
  uint32_t opcode = instr_fetch(eip, 1) | 0x100;
  decoding.opcode = opcode;
  set_width(opcode_table[opcode].width);
  idex(eip, &opcode_table[opcode]);
}

make_EHelper(real) {
  uint32_t opcode = instr_fetch(eip, 1);
  decoding.opcode = opcode;
  set_width(opcode_table[opcode].width);
  idex(eip, &opcode_table[opcode]);
}

static inline void update_eip(void) {
  cpu.eip = (decoding.is_jmp ? (decoding.is_jmp = 0, decoding.jmp_eip) : decoding.seq_eip);
}

void exec_wrapper(bool print_flag) {
#ifdef DEBUG
  decoding.p = decoding.asm_buf;
  decoding.p += sprintf(decoding.p, "%8x:   ", cpu.eip);
#endif

  decoding.seq_eip = cpu.eip;
  exec_real(&decoding.seq_eip);

#ifdef DEBUG
  int instr_len = decoding.seq_eip - cpu.eip;
  sprintf(decoding.p, "%*.s", 50 - (12 + 3 * instr_len), "");
  strncat(decoding.asm_buf, decoding.assembly, 80);
  Log_write("%s\n", decoding.asm_buf);
  if (print_flag) {
    puts(decoding.asm_buf);
  }
#endif

#ifdef DIFF_TEST
  uint32_t eip = cpu.eip;
#endif

  update_eip();

#ifdef DIFF_TEST
  void difftest_step(uint32_t);
  difftest_step(eip);
#endif
}
