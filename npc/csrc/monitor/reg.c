#include <isa.h>
#include "reg.h"
#include <sim_top.h>
// #include "Vtop__Dpi.h"
#include "assert.h"
#include "svdpi.h"

int i = 0;

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
    update_reg();
    printf("PC      : 0x%08x %d\n", cpu.pc, cpu.pc);
    for (int i = 0; i < 32; i++) {
    printf("%-4s    : 0x%08x %-11d  ", reg_name(i), gpr(i), gpr(i));
    if ((i + 1) % 4 == 0) printf("\n");
    }
    // printf("mcause  : 0x%08x %-11d  "  , cpu.csr[0], cpu.csr[0]);
    // printf("mstatus : 0x%08x %-11d  "  , cpu.csr[1], cpu.csr[1]);
    // printf("mepc    : 0x%08x %-11d  "  , cpu.csr[2], cpu.csr[2]);
    // printf("mtvec   : 0x%08x %-11d  \n", cpu.csr[3], cpu.csr[3]);
    // printf("mhartid : 0x%08x %-11d  "  , cpu.csr[4], cpu.csr[4]);
    // printf("mscratch: 0x%08x %-11d  "  , cpu.csr[5], cpu.csr[5]);

    printf("\n");
}

//讲寄存器名字符转换为对应的寄存器值
word_t isa_reg_str2val(const char *s, bool *success) {
  int idx=0;
  char str[10];
  strcpy(str,s+1); //去除最左边的$
  if(strcmp(str,"pc")==0) return cpu.pc; //如果是pc那就返回cpu.pc的值
  for(int i=0;i<MUXDEF(CONFIG_RVE, 16, 32);i++){
    if(strcmp(regs[i],str)==0){
      idx=i; //返回索引值
      break;
    }
    if(i==31) *success=false;
  }
  return gpr(idx);
}

//get gpr value
word_t get_gpr(int i){
  assert((i >= 0) && (i <= 32));
  if(i == 0) return 0;
  else if(i == 32) return cpu.pc;
  else return gpr(i);
}

/**************************** DPI-C *******************************/
//1. difftest regfile register
extern "C" void difftest_ArchIntRegState(word_t *out_io_value_0, word_t *out_io_value_1, word_t *out_io_value_2, word_t *out_io_value_3, word_t *out_io_value_4,
                                         word_t *out_io_value_5, word_t *out_io_value_6, word_t *out_io_value_7, word_t *out_io_value_8, word_t *out_io_value_9,
                                         word_t *out_io_value_10, word_t *out_io_value_11, word_t *out_io_value_12, word_t *out_io_value_13, word_t *out_io_value_14,
                                         word_t *out_io_value_15, word_t *out_io_value_16, word_t *out_io_value_17, word_t *out_io_value_18, word_t *out_io_value_19,
                                         word_t *out_io_value_20, word_t *out_io_value_21, word_t *out_io_value_22, word_t *out_io_value_23, word_t *out_io_value_24,
                                         word_t *out_io_value_25, word_t *out_io_value_26, word_t *out_io_value_27, word_t *out_io_value_28, word_t *out_io_value_29,
                                         word_t *out_io_value_30, word_t *out_io_value_31);

void update_reg(void){
    const svScope scope = svGetScopeFromName(SIM_SCOPE_DIFFTEST_REGFILE);
    assert(scope);
    svSetScope(scope);  // 设置当前 DPI 作用域
    difftest_ArchIntRegState(
      &cpu.gpr[0 ], 
      &cpu.gpr[1 ], 
      &cpu.gpr[2 ], 
      &cpu.gpr[3 ],
      &cpu.gpr[4 ], 
      &cpu.gpr[5 ], 
      &cpu.gpr[6 ], 
      &cpu.gpr[7 ],
      &cpu.gpr[8 ], 
      &cpu.gpr[9 ], 
      &cpu.gpr[10], 
      &cpu.gpr[11],
      &cpu.gpr[12], 
      &cpu.gpr[13], 
      &cpu.gpr[14], 
      &cpu.gpr[15],
      &cpu.gpr[16], 
      &cpu.gpr[17], 
      &cpu.gpr[18], 
      &cpu.gpr[19],
      &cpu.gpr[20], 
      &cpu.gpr[21], 
      &cpu.gpr[22], 
      &cpu.gpr[23],
      &cpu.gpr[24], 
      &cpu.gpr[25], 
      &cpu.gpr[26], 
      &cpu.gpr[27],
      &cpu.gpr[28], 
      &cpu.gpr[29], 
      &cpu.gpr[30], 
      &cpu.gpr[31]
  );
}



