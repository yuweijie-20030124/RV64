/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <cpu/ftrace.h>
#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_B, TYPE_R, TYPE_CR, TYPE_CI, TYPE_CSS, TYPE_CIW, TYPE_CL, TYPE_CS, TYPE_CB,TYPE_CA,
  TYPE_ZBA,TYPE_XPERM8,
  TYPE_N, // none 
};

static void etrace() {
  IFDEF(CONFIG_ETRACE, {
      printf(ANSI_FMT("[ETRACE]", ANSI_FG_BLUE)"ecall in mepc = " FMT_WORD ", mcause = " FMT_WORD "\n",cpu.mepc, cpu.mcause);});
}

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = SEXT(((BITS(i, 31, 31) << 19) | BITS(i, 30, 21) | (BITS(i, 20, 20) << 10) | (BITS(i, 19, 12) << 11)) << 1, 21);} while(0)
#define immB() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1); } while(0)
#define c_addi4spn_imm ((BITS(c_inst, 12, 11) << 4) | (BITS(c_inst, 10, 7) << 6) | (BITS(c_inst, 6, 6) << 2) | (BITS(c_inst, 5, 5) << 3))
#define c_lsw_imm ((BITS(c_inst, 12, 10) << 3) | (BITS(c_inst, 6, 6) << 2) | (BITS(c_inst, 5, 5) << 6))
#define c_lsd_imm ((BITS(c_inst, 12, 10) << 3) | (BITS(c_inst, 6, 5) << 6))
#define c_addi_addiw_andi_li_imm (SEXT(((BITS(c_inst, 12, 12) << 5) | (BITS(c_inst, 6, 2))), 6))
#define c_addi16sp_imm (SEXT(((BITS(c_inst, 12, 12) << 9) | (BITS(c_inst, 6, 6) << 4) | (BITS(c_inst, 5, 5) << 6) | (BITS(c_inst, 4, 3) << 7) | (BITS(c_inst, 2, 2) << 5)), 10))
#define c_lui_imm (SEXT(((BITS(c_inst, 12, 12) << 17) | (BITS(c_inst, 6, 2) << 12)), 18))
#define c_j_jal_imm (SEXT(((BITS(c_inst, 12, 12) << 11) | (BITS(c_inst, 11, 11) << 4) | (BITS(c_inst, 10, 9) << 8) | (BITS(c_inst, 8, 8) << 10) | (BITS(c_inst, 7, 7) << 6) | (BITS(c_inst, 6, 6) << 7) | (BITS(c_inst, 5, 3) << 1) | (BITS(c_inst, 2, 2) << 5)), 12))
#define c_b_imm (SEXT(((BITS(c_inst, 12, 12) << 8) | (BITS(c_inst, 11, 10) << 3) | (BITS(c_inst, 6, 5) << 6) | (BITS(c_inst, 4, 3) << 1) | (BITS(c_inst, 2, 2) << 5)), 9))
#define c_srli_srai_slli_imm ((BITS(c_inst, 12, 12) << 5) | (BITS(c_inst, 6, 2)))
#define c_lwsp_imm ((BITS(c_inst, 12, 12) << 5) | (BITS(c_inst, 6, 4) << 2) | (BITS(c_inst, 3, 2) << 6))
#define c_swsp_imm ((BITS(c_inst, 12, 9) << 2) | (BITS(c_inst, 8, 7) << 6))
#define c_ldsp_imm ((BITS(c_inst, 12, 12) << 5) | (BITS(c_inst, 6, 5) << 3) | (BITS(c_inst, 4, 2) << 6))
#define c_sdsp_imm ((BITS(c_inst, 12, 10) << 3) | (BITS(c_inst, 9, 7) << 6))
// #define zba_imm 


#define srcCR_CI_CSS()            \
    do                        \
    {                         \
        rs1 = BITS(i, 11, 7); \
        rs2 = BITS(i, 6, 2);  \
    } while (0)
#define srcCIW_CL_CS_CB()       \
    do                             \
    {                              \
        rs1 = (BITS(i, 9, 7) + 8); \
        rs2 = (BITS(i, 4, 2) + 8); \
        *rd = rs2;                 \
    } while (0)
#define srcCA()       \
    do                             \
    {                              \
        rs1 = (BITS(i, 9, 7) + 8); \
        rs2 = (BITS(i, 4, 2) + 8); \
        *rd = rs1;                 \
    } while (0)


static bool can_not_diasssemble;
bool isa_can_not_disassemble(){
    return can_not_diasssemble;
}

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I:     src1R();           immI();            break;
    case TYPE_U:                        immU();            break;
    case TYPE_S:     src1R();           src2R(); immS();   break;
		case TYPE_J: 	   				           immJ();            break;
		case TYPE_B:     src1R();           src2R(); immB();   break;
    case TYPE_R:     src1R();           src2R();           break;
    case TYPE_CR:    srcCR_CI_CSS();    src1R(); src2R();  break;
    case TYPE_CI:    srcCR_CI_CSS();    src1R();           break;
    case TYPE_CSS:   srcCR_CI_CSS();             src2R();  break;
    case TYPE_CIW:   srcCIW_CL_CS_CB();                    break;
    case TYPE_CL:    srcCIW_CL_CS_CB(); src1R();           break;
    case TYPE_CS:    srcCIW_CL_CS_CB(); src1R(); src2R();  break;
    case TYPE_CB:    srcCIW_CL_CS_CB(); src1R();           break;
    case TYPE_CA:    srcCA();           src1R(); src2R();  break;
    case TYPE_ZBA:   src1R();           src2R();           break;
    case TYPE_XPERM8:src1R();                              break;
		
 }
}
/*
static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst)
#define INSTPAT_MATCH(s, name, type, ...  execute body  ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}
*/

static int decode_exec(Decode *s) {
    int rd = 0;
  uint16_t c_inst = 0;
    can_not_diasssemble = false;
    word_t src1 = 0, src2 = 0, imm = 0;
    s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
    __VA_ARGS__ ; \
}

  INSTPAT_START();
  //c extend
  if((INSTPAT_INST(s) & 0x3) != 0x3){
      s->dnpc -= 2;
      INSTPAT_INST(s) &= 0xffff;
      c_inst = INSTPAT_INST(s);
      if((INSTPAT_INST(s) & 0x3) == 0x0){
    INSTPAT("000 ??? ??? ?? ??? 00", C.ADDI4SPN , CIW , R(rd) = R(2) + c_addi4spn_imm);
    INSTPAT("010 ??? ??? ?? ??? 00", c.lw       , CL  , R(rd) = SEXT(Mr(src1 + c_lsw_imm, 4), 32));
    INSTPAT("110 ??? ??? ?? ??? 00", c.sw       , CS  , Mw(src1 + c_lsw_imm, 4, src2));
        }
    else if((INSTPAT_INST(s) & 0x3) == 0x1){
    INSTPAT("000 ?00 000 ?? ??? 01", c.nop,                 N);
    INSTPAT("000 ??? ??? ?? ??? 01", c.addi,                CI, R(rd) = src1 + c_addi_addiw_andi_li_imm);
    INSTPAT("010 ??? ??? ?? ??? 01", c.li,                  CI, R(rd) = c_addi_addiw_andi_li_imm);
    INSTPAT("011 ?00 010 ?? ??? 01", c.addi16sp,            CI, R(rd) = src1 + c_addi16sp_imm);
    INSTPAT("011 ??? ??? ?? ??? 01", c.lui,                 CI, R(rd) = c_lui_imm);
    INSTPAT("100 ?10 ??? ?? ??? 01", c.andi,                CA, R(rd) = src1 & c_addi_addiw_andi_li_imm);
    INSTPAT("100 011 ??? 00 ??? 01", c.sub,                 CA, R(rd) = src1 - src2);
    INSTPAT("100 011 ??? 01 ??? 01", c.xor,                 CA, R(rd) = src1 ^ src2);
    INSTPAT("100 011 ??? 10 ??? 01", c.or,                  CA, R(rd) = src1 | src2);
    INSTPAT("100 011 ??? 11 ??? 01", c.and,                 CA, R(rd) = src1 & src2);
    INSTPAT("101 ??? ??? ?? ??? 01", c.j,                   N, s->dnpc = s->pc + c_j_jal_imm);
    INSTPAT("110 ??? ??? ?? ??? 01", c.beqz,                CB, if (src1 == 0) s->dnpc = s->pc + c_b_imm);
    INSTPAT("111 ??? ??? ?? ??? 01", c.bnez,                CB, if (src1 != 0) s->dnpc = s->pc + c_b_imm);
    INSTPAT("001 ??? ??? ?? ??? 01", c.jal,                 N, R(1) = s->dnpc; s->dnpc = s->pc + c_j_jal_imm);
    INSTPAT("100 000 ??? ?? ??? 01", c.srli,                CA, R(rd) = src1 >> c_srli_srai_slli_imm);
    INSTPAT("100 001 ??? ?? ??? 01", c.srai,                CA, R(rd) = (sword_t)src1 >> c_srli_srai_slli_imm);
    }
    else if((INSTPAT_INST(s) & 0x3) == 0x2){
    INSTPAT("010 ??? ??? ?? ??? 10", c.lwsp,                CI, R(rd) = SEXT(Mr(R(2) + c_lwsp_imm, 4), 32));
    INSTPAT("100 0?? ??? 00 000 10", c.jr,                  CI, s->dnpc = src1; s->dnpc &= ((word_t)-2));
    INSTPAT("100 0?? ??? ?? ??? 10", c.mv,                  CR, R(rd) = src2);
    // c.ebreak must be matched before the wider c.jalr form, otherwise it
    // is mis-decoded as jalr x1, x0 and falls through to PC 0.
    INSTPAT("100 100 000 00 000 10", c.ebreak,              N, NEMUTRAP(s->pc, R(10)));
    INSTPAT("100 1?? ??? 00 000 10", c.jalr,                CR, R(1) = s->dnpc; s->dnpc = src1; s->dnpc &= ((word_t)-2);/*ftrace*/);
    INSTPAT("100 1?? ??? ?? ??? 10", c.add,                 CR, R(rd) = src1 + src2);
    INSTPAT("110 ??? ??? ?? ??? 10", c.swsp,                CSS, Mw(R(2) + c_swsp_imm, 4, src2));
    INSTPAT("000 0?? ??? ?? ??? 10", c.slli,                CI, R(rd) = src1 << c_srli_srai_slli_imm);
  }
  }
  //INSTPAT(模式字符串, 指令名称, 指令类型, 指令执行操作);
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm); 
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);     

  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = src1 >> BITS(imm, 5, 0)); 
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = src1 << BITS(imm, 5, 0));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, R(rd) = (int32_t)src1 >> BITS(imm , 4 , 0) ); 
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1 + imm);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = (src1 < imm) ? 1 : 0); 
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = ((int32_t)src1 < ((int32_t)imm)) ? 1 : 0); 
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(rd) = SEXT(Mr(src1 + imm, 1),8));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = SEXT(Mr(src1 + imm, 2),16));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4)); 
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm); 
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm); 
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = src1 | imm);

  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2)); 
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2)); 
  INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd     , S, Mw(src1 + imm, 8, src2)); 

  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(rd) = s->pc + 4;
   s->dnpc = s->pc + imm;
   IFDEF(CONFIG_FTRACE, {
    if (rd == 1) {
        call_trace(s->pc, s->dnpc);
    }})
   );
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, R(rd) = s->pc + 4;
   s->dnpc = (src1 + imm) & (~1);
   IFDEF(CONFIG_FTRACE,{
    if (s->isa.inst.val == 0x00008067)
        ret_trace(s->pc);
    else if (rd == 1) {call_trace(s->pc, s->dnpc);} 
    else if (rd == 0 && imm == 0) {call_trace(s->pc, s->dnpc);}
   })
   );
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, 
    // if(s->pc == 0x800115c0){
    // printf("src1 =%d\n",src1);
    // printf("src2 =%d\n",src2);
    // printf("pc =0x%08x\n",s->pc);
    // printf("imm =0x%08x\n",imm);
    // printf("dnpc =0x%08x\n",s->dnpc);  
    // }
    // printf("src1 =%d\n",src1);printf("src2 =%d\n",src2);
    // printf("pc =0x%08x\n",s->pc);printf("imm =0x%08x\n",imm);
    // printf("dnpc =0x%08x\n",s->dnpc);
    if(src1 == src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, if(src1 != src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, s->dnpc = ((int32_t)src1< (int32_t)src2) ? s->pc + imm : s->dnpc); 
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, s->dnpc = ((int32_t)src1>=(int32_t)src2) ? s->pc + imm : s->dnpc); 
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, s->dnpc = (src1< src2) ? s->pc + imm : s->dnpc); 
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, s->dnpc = (src1>=src2) ? s->pc + imm : s->dnpc); 
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = src1 >> BITS(src2, 4, 0));
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2); 
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = src1 <<  BITS(src2 , 4 , 0)); 
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = ((int32_t)src1 < (int32_t)src2) ? 1 : 0);
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = src1 < src2 ? 1 : 0); 
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2); 
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2); 
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2); 
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(rd) = (int32_t)src1 >> BITS(src2 , 4 , 0)); 

  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2); 
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = (unsigned)src1 * (unsigned)src2);
  //INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = src1 / src2);
  //INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = src1 % src2);
  //INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(rd) = (unsigned)src1 % (unsigned)src2);
  //INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(rd) = (unsigned)src1 / (unsigned)src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, R(rd) = ((int64_t)(int32_t)src1 * (int64_t)(int32_t)src2) >> 32;);
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, R(rd) = ((int64_t)(int32_t)src1 * (int64_t)(uint32_t)src2) >> 32;);
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, R(rd) = ((int64_t)(uint32_t)src1 * (int64_t)(uint32_t)src2) >> 32;);
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, if (src2 == 0) R(rd) = -1;else if ((int32_t)src1 == INT32_MIN && (int32_t)src2 == -1) R(rd) = INT32_MIN;else R(rd) = (int32_t)src1 / (int32_t)src2;);
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, if (src2 == 0) R(rd) = 0xFFFFFFFF;else R(rd) = (uint32_t)src1 / (uint32_t)src2;);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, if (src2 == 0) R(rd) = (int32_t)src1;else if ((int32_t)src1 == INT32_MIN && (int32_t)src2 == -1) R(rd) = 0;else R(rd) = (int32_t)src1 % (int32_t)src2;);
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, if (src2 == 0) R(rd) = (uint32_t)src1;else R(rd) = (uint32_t)src1 % (uint32_t)src2;);
  
//ZBA指令
  INSTPAT("0010000 ????? ????? 010 ????? 01100 11", sh1add , ZBA, R(rd) = ((src1 << 1) + src2););
  INSTPAT("0010000 ????? ????? 100 ????? 01100 11", sh2add , ZBA, R(rd) = ((src1 << 2) + src2););
  INSTPAT("0010000 ????? ????? 110 ????? 01100 11", sh3add , ZBA, R(rd) = ((src1 << 3) + src2););
  INSTPAT("0000100 00000 ????? 100 ????? 01100 11", ZEXT.H , XPERM8, R(rd) = src1 & 0xffff;);
  INSTPAT("0100100 ????? ????? 101 ????? 00100 11", bexti  , I,   R(rd) = (src1 >> BITS(imm, 4, 0)) & 1;);
  INSTPAT("0110000 00101 ????? 001 ????? 00100 11", sext.h , I,   R(rd) = SEXT(BITS(src1, 15, 0), 16););

  // ===== Zbb 补全：单源一元运算 (funct12 固定，op=0010011)，
  // 必须放在 rori 等通配模式之前，确保 rs2 字段被精确匹配，避免误匹配。
  // INSTPAT("0110000 00000 ????? 001 ????? 00100 11", clz    , I,   R(rd) = (src1 == 0) ? 32 : __builtin_clz(src1););
  // INSTPAT("0110000 00001 ????? 001 ????? 00100 11", ctz    , I,   R(rd) = (src1 == 0) ? 32 : __builtin_ctz(src1););
  // INSTPAT("0110000 00010 ????? 001 ????? 00100 11", cpop   , I,   R(rd) = __builtin_popcount(src1););
  INSTPAT("0110000 00100 ????? 001 ????? 00100 11", sext.b , I,   R(rd) = SEXT(BITS(src1, 7, 0), 8););
  INSTPAT("0010100 00111 ????? 101 ????? 00100 11", orc.b  , I, {
    // orc.b: 每个字节内若任一位为 1，则该字节扩展为 0xff，否则为 0x00。
    uint32_t v = src1, r = 0;
    for (int b = 0; b < 4; b++) if ((v >> (b*8)) & 0xff) r |= (uint32_t)0xff << (b*8);
    R(rd) = r;
  });
  INSTPAT("0110100 11000 ????? 101 ????? 00100 11", rev8   , I, {
    // rev8 (RV32): 反转 32 位字的字节序。
    uint32_t v = src1;
    R(rd) = ((v & 0xff) << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) | ((v >> 24) & 0xff);
  });

  // ===== Zbb：rori (立即数循环右移，funct7=0110000, funct3=101, op=0010011)
  // shamt 在 imm 低 5 位；shamt==0 时直接返回 src1，避免 32 位移位 UB。
  INSTPAT("0110000 ????? ????? 101 ????? 00100 11", rori   , I, {
    uint32_t sh = BITS(imm, 4, 0);
    R(rd) = (sh == 0) ? src1 : ((src1 >> sh) | (src1 << (32 - sh)));
  });

  // ===== Zbb：双源 R-type 运算 (op=0110011)
  INSTPAT("0000101 ????? ????? 100 ????? 01100 11", min    , R, R(rd) = ((int32_t)src1 < (int32_t)src2) ? src1 : src2;);
  INSTPAT("0000101 ????? ????? 101 ????? 01100 11", minu   , R, R(rd) = ((uint32_t)src1 < (uint32_t)src2) ? src1 : src2;);
  INSTPAT("0000101 ????? ????? 110 ????? 01100 11", max    , R, R(rd) = ((int32_t)src1 > (int32_t)src2) ? src1 : src2;);
  INSTPAT("0000101 ????? ????? 111 ????? 01100 11", maxu   , R, R(rd) = ((uint32_t)src1 > (uint32_t)src2) ? src1 : src2;);
  INSTPAT("0100000 ????? ????? 111 ????? 01100 11", andn   , R, R(rd) = src1 & ~src2;);
  INSTPAT("0100000 ????? ????? 110 ????? 01100 11", orn    , R, R(rd) = src1 | ~src2;);
  INSTPAT("0100000 ????? ????? 100 ????? 01100 11", xnor   , R, R(rd) = ~(src1 ^ src2););
  INSTPAT("0110000 ????? ????? 001 ????? 01100 11", rol    , R, {
    uint32_t sh = src2 & 31;
    R(rd) = (sh == 0) ? src1 : ((src1 << sh) | (src1 >> (32 - sh)));
  });
  INSTPAT("0110000 ????? ????? 101 ????? 01100 11", ror    , R, {
    uint32_t sh = src2 & 31;
    R(rd) = (sh == 0) ? src1 : ((src1 >> sh) | (src1 << (32 - sh)));
  });

  // ===== Zbs 补全：bclr/bset/binv/bext (R-type) + bclri/bseti/binvi (I-type)
  INSTPAT("0100100 ????? ????? 001 ????? 01100 11", bclr   , R, R(rd) = src1 & ~((uint32_t)1 << (src2 & 31)););
  INSTPAT("0010100 ????? ????? 001 ????? 01100 11", bset   , R, R(rd) = src1 |  ((uint32_t)1 << (src2 & 31)););
  INSTPAT("0110100 ????? ????? 001 ????? 01100 11", binv   , R, R(rd) = src1 ^  ((uint32_t)1 << (src2 & 31)););
  INSTPAT("0100100 ????? ????? 101 ????? 01100 11", bext   , R, R(rd) = (src1 >> (src2 & 31)) & 1;);
  INSTPAT("0100100 ????? ????? 001 ????? 00100 11", bclri  , I, R(rd) = src1 & ~((uint32_t)1 << BITS(imm, 4, 0)););
  INSTPAT("0010100 ????? ????? 001 ????? 00100 11", bseti  , I, R(rd) = src1 |  ((uint32_t)1 << BITS(imm, 4, 0)););
  INSTPAT("0110100 ????? ????? 001 ????? 00100 11", binvi  , I, R(rd) = src1 ^  ((uint32_t)1 << BITS(imm, 4, 0)););

  INSTPAT("0100000 00000 00000 000 00000 01100 11", sub    , R, magic_instruction()); 

  INSTPAT("0011000 00010 00000 000 0000 011100 11", mret   , R, s->dnpc = cpu.mepc);
  //div注释：
  //匹配 div 指令（有符号除法）。
  //如果除数 src2 为 0，结果规定为 -1。
  //如果被除数是最小负数（INT32_MIN），除数为 -1，结果规定为 INT32_MIN（防止溢出）。
  //否则正常做有符号除法。
  
  //printf("mulh:%lx\n", (int64_t)(~src1+1) * (int64_t)src2));
  //正确的a5:0001 1001 1101 0010 1001 1010 1011 1001
  //INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, R(rd) = SEXT(src1 * src2, 32));
  //把寄存器 x[rs2]乘到寄存器 x[rs1]上，都视为 2 的补码，将乘积的高位写入 x[rd]。
  
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , I, s->dnpc = isa_raise_intr(11,s->pc);etrace());
  
    //CSR寄存器
  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , I,  
  if(imm == 0x305){  //mtvec
    R(rd) = cpu.mtvec;
    cpu.mtvec =  src1;
  };
  if(imm == 0x300){ //mstatus
    R(rd) = cpu.mstatus;
    cpu.mstatus =  src1;
  };
  if(imm == 0x341){ //mepc
    R(rd) = cpu.mepc;
    cpu.mepc =  src1;
  };
  if(imm == 0x342){ //mcause
    R(rd) = cpu.mcause;
    cpu.mcause =  src1;
  };
);
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  , I, 
  if(imm == 0x305){  //mtvec
    R(rd) = cpu.mtvec;
    cpu.mtvec |=  src1;
  };
  if(imm == 0x300){ //mstatus
    R(rd) = cpu.mstatus;
    cpu.mstatus |=  src1;
  };
  if(imm == 0x341){ //mepc
    R(rd) = cpu.mepc;
    cpu.mepc |=  src1;
  };
  if(imm == 0x342){ //mcause
    R(rd) = cpu.mcause;
    cpu.mcause |=  src1;
  };
);
	INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
	INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  //return一个0回去，但是现在并没有使用这个返回值，可以忽略他。
  return decode_exec(s);
}


/*
7'b0000011: begin // lw
                regS = 1;
                RegW = 1;
            end
    
            7'b0100011: begin // sw
                MemWr = 1;
                ALUop = 0;
            end

jal-判断该指令是否为jal；
branch-判断指令是否为beq；
brlt-判断指令是否为blt；
regS-写回数据的选择信号，0-来源于ALU，1-来源于DataMem，2-来源于PC+4；
ALUop-ALU控制信号，0对应+，1对应-；
MemWr-DataMem写使能；
ALUsrc-操作数选择信号，0-选择寄存器，1-选择立即数；
RegW-寄存器堆写使能；
PCx1-jalr处理信号，若为jalr则将x1+offset的值写入PC；

*/
