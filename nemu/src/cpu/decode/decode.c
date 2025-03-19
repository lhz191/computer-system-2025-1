#include "cpu/exec.h"
#include "cpu/rtl.h"

/* shared by all helper functions */
DecodeInfo decoding;
rtlreg_t t0, t1, t2, t3;
const rtlreg_t tzero = 0;

#define make_DopHelper(name) void concat(decode_op_, name) (vaddr_t *eip, Operand *op, bool load_val)

/* Refer to Appendix A in i386 manual for the explanations of these abbreviations */

/* Ib, Iv */
static inline make_DopHelper(I) {
  /* eip here is pointing to the immediate */
  op->type = OP_TYPE_IMM;
  op->imm = instr_fetch(eip, op->width);
  rtl_li(&op->val, op->imm);

#ifdef DEBUG
  snprintf(op->str, OP_STR_SIZE, "$0x%x", op->imm);
#endif
}

/* I386 manual does not contain this abbreviation, but it is different from
 * the one above from the view of implementation. So we use another helper
 * function to decode it.
 */
/* sign immediate */
static inline make_DopHelper(SI) {
  assert(op->width == 1 || op->width == 4);
  //首先确保操作数宽度是合法的（1或4字节）
  op->type = OP_TYPE_IMM;

  /* TODO: Use instr_fetch() to read `op->width' bytes of memory
   * pointed by `eip'. Interpret the result as a signed immediate,
   * and assign it to op->simm.
   *
   op->simm = ???
   */
  // TODO();
  //使用instr_fetch()从eip指向的内存读取指定宽度的数据
  //将读取的数据作为有符号立即数存储在op->simm中
  //simm表示signed immediate，有符号立即数
  uint32_t imm = instr_fetch(eip, op->width);
    // printf("指令原始偏移量: 0x%x\n", imm);
  // if (op->width == 1) {
  //   // 对8位立即数进行符号扩展
  //   op->simm = (int8_t)imm;
  // } else {
  //   op->simm = (int32_t)imm;
  // }
  rtl_sext(&imm, &imm, op->width);
  // printf("指令原始偏移量: 0x%x\n", imm);

  op->simm = (int32_t)imm;
  // 根据操作数宽度进行符号扩展，使用RTL指令将立即数加载到操作数的val字段中
  rtl_li(&op->val, op->simm);
  // printf("指令大小: %d字节\n", op->width);
  // printf("指令原始偏移量: 0x%x\n", imm);
#ifdef DEBUG
  snprintf(op->str, OP_STR_SIZE, "$0x%x", op->simm);
#endif
}

/* I386 manual does not contain this abbreviation.
 * It is convenient to merge them into a single helper function.
 */
/* AL/eAX */
static inline make_DopHelper(a) {
  op->type = OP_TYPE_REG;
  op->reg = R_EAX;
  if (load_val) {
    rtl_lr(&op->val, R_EAX, op->width);
  }

#ifdef DEBUG
  snprintf(op->str, OP_STR_SIZE, "%%%s", reg_name(R_EAX, op->width));
#endif
}

/* This helper function is use to decode register encoded in the opcode. */
/* XX: AL, AH, BL, BH, CL, CH, DL, DH
 * eXX: eAX, eCX, eDX, eBX, eSP, eBP, eSI, eDI
 */
static inline make_DopHelper(r) {
  op->type = OP_TYPE_REG;
  op->reg = decoding.opcode & 0x7;
  if (load_val) {
    rtl_lr(&op->val, op->reg, op->width);
  }

#ifdef DEBUG
  snprintf(op->str, OP_STR_SIZE, "%%%s", reg_name(op->reg, op->width));
#endif
}

/* I386 manual does not contain this abbreviation.
 * We decode everything of modR/M byte by one time.
 */
/* Eb, Ew, Ev
 * Gb, Gv
 * Cd,
 * M
 * Rd
 * Sw
 */
static inline void decode_op_rm(vaddr_t *eip, Operand *rm, bool load_rm_val, Operand *reg, bool load_reg_val) {
  read_ModR_M(eip, rm, load_rm_val, reg, load_reg_val);
}

/* Ob, Ov */
static inline make_DopHelper(O) {
  op->type = OP_TYPE_MEM;
  op->addr = instr_fetch(eip, 4);
  if (load_val) {
    rtl_lm(&op->val, &op->addr, op->width);
  }

#ifdef DEBUG
  snprintf(op->str, OP_STR_SIZE, "0x%x", op->addr);
#endif
}

/* Eb <- Gb
 * Ev <- Gv
 */
make_DHelper(G2E) {
  decode_op_rm(eip, id_dest, true, id_src, true);
}

make_DHelper(mov_G2E) {
  decode_op_rm(eip, id_dest, false, id_src, true);
}

/* Gb <- Eb
 * Gv <- Ev
 */
make_DHelper(E2G) {
  decode_op_rm(eip, id_src, true, id_dest, true);
}

make_DHelper(mov_E2G) {
  decode_op_rm(eip, id_src, true, id_dest, false);
}

make_DHelper(lea_M2G) {
  decode_op_rm(eip, id_src, false, id_dest, false);
}

/* AL <- Ib
 * eAX <- Iv
 */
make_DHelper(I2a) {
  decode_op_a(eip, id_dest, true);
  decode_op_I(eip, id_src, true);
}

/* Gv <- EvIb
 * Gv <- EvIv
 * use for imul */
make_DHelper(I_E2G) {
  decode_op_rm(eip, id_src2, true, id_dest, false);
  decode_op_I(eip, id_src, true);
}

/* Eb <- Ib
 * Ev <- Iv
 */
make_DHelper(I2E) {
  decode_op_rm(eip, id_dest, true, NULL, false);
  decode_op_I(eip, id_src, true);
}

make_DHelper(mov_I2E) {
  decode_op_rm(eip, id_dest, false, NULL, false);
  decode_op_I(eip, id_src, true);
}

/* XX <- Ib
 * eXX <- Iv
 */
make_DHelper(I2r) {
  decode_op_r(eip, id_dest, true);
  decode_op_I(eip, id_src, true);
}

make_DHelper(mov_I2r) {
  decode_op_r(eip, id_dest, false);
  decode_op_I(eip, id_src, true);
}

/* used by unary operations */
make_DHelper(I) {
  decode_op_I(eip, id_dest, true);
}

make_DHelper(r) {
  decode_op_r(eip, id_dest, true);
}

make_DHelper(E) {
  decode_op_rm(eip, id_dest, true, NULL, false);
}

make_DHelper(gp7_E) {
  decode_op_rm(eip, id_dest, false, NULL, false);
}

/* used by test in group3 */
make_DHelper(test_I) {
  decode_op_I(eip, id_src, true);
}

make_DHelper(SI2E) {
  assert(id_dest->width == 2 || id_dest->width == 4);
  decode_op_rm(eip, id_dest, true, NULL, false);
  id_src->width = 1;
  decode_op_SI(eip, id_src, true);
  if (id_dest->width == 2) {
    id_src->val &= 0xffff;
  }
}

make_DHelper(SI_E2G) {
  assert(id_dest->width == 2 || id_dest->width == 4);
  decode_op_rm(eip, id_src2, true, id_dest, false);
  id_src->width = 1;
  decode_op_SI(eip, id_src, true);
  if (id_dest->width == 2) {
    id_src->val &= 0xffff;
  }
}

make_DHelper(gp2_1_E) {
  decode_op_rm(eip, id_dest, true, NULL, false);
  id_src->type = OP_TYPE_IMM;
  id_src->imm = 1;
  rtl_li(&id_src->val, 1);
#ifdef DEBUG
  sprintf(id_src->str, "$1");
#endif
}

make_DHelper(gp2_cl2E) {
  decode_op_rm(eip, id_dest, true, NULL, false);
  id_src->type = OP_TYPE_REG;
  id_src->reg = R_CL;
  rtl_lr_b(&id_src->val, R_CL);
#ifdef DEBUG
  sprintf(id_src->str, "%%cl");
#endif
}

make_DHelper(gp2_Ib2E) {
  decode_op_rm(eip, id_dest, true, NULL, false);
  id_src->width = 1;
  decode_op_I(eip, id_src, true);
}

/* Ev <- GvIb
 * use for shld/shrd */
make_DHelper(Ib_G2E) {
  decode_op_rm(eip, id_dest, true, id_src2, true);
  id_src->width = 1;
  decode_op_I(eip, id_src, true);
}

make_DHelper(O2a) {
  decode_op_O(eip, id_src, true);
  decode_op_a(eip, id_dest, false);
}

make_DHelper(a2O) {
  decode_op_a(eip, id_src, true);
  decode_op_O(eip, id_dest, false);
}

make_DHelper(J) {
  decode_op_SI(eip, id_dest, false);
  
  // 添加调试信息
  printf("J解码器 - 当前eip: 0x%x\n", *eip);
  printf("J解码器 - 偏移量: 0x%x (%d)\n", id_dest->simm, id_dest->simm);
  
  // the target address can be computed in the decode stage
  decoding.jmp_eip = id_dest->simm + *eip;
  
  printf("J解码器 - 计算出的跳转目标: 0x%x\n", decoding.jmp_eip);
}

make_DHelper(push_SI) {
  decode_op_SI(eip, id_dest, true);
}

make_DHelper(in_I2a) {
  id_src->width = 1;
  decode_op_I(eip, id_src, true);
  decode_op_a(eip, id_dest, false);
}

make_DHelper(in_dx2a) {
  id_src->type = OP_TYPE_REG;
  id_src->reg = R_DX;
  rtl_lr_w(&id_src->val, R_DX);
#ifdef DEBUG
  sprintf(id_src->str, "(%%dx)");
#endif

  decode_op_a(eip, id_dest, false);
}

make_DHelper(out_a2I) {
  decode_op_a(eip, id_src, true);
  id_dest->width = 1;
  decode_op_I(eip, id_dest, true);
}

make_DHelper(out_a2dx) {
  decode_op_a(eip, id_src, true);

  id_dest->type = OP_TYPE_REG;
  id_dest->reg = R_DX;
  rtl_lr_w(&id_dest->val, R_DX);
#ifdef DEBUG
  sprintf(id_dest->str, "(%%dx)");
#endif
}

/* decode SI format instruction */
//Pa2.1 0x6a: push imm8 with sign extension,SI解码器
make_DHelper(SI) {
  //DopHelper是操作数解码器（Decode OPerand Helper）
  //DHelper是指令解码器（Decode Helper）
  decode_op_SI(eip, id_dest, true);
}


void operand_write(Operand *op, rtlreg_t* src) {
  if (op->type == OP_TYPE_REG) { rtl_sr(op->reg, op->width, src); }
  else if (op->type == OP_TYPE_MEM) { rtl_sm(&op->addr, op->width, src); }
  else { assert(0); }
}
