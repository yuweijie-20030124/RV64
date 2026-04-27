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

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
    // PC 单独一行，与其他寄存器保持相同的列宽格式
    printf("%-8s 0x%016lx %12ld\n", "PC", cpu.pc, cpu.pc);
    
    // 每个寄存器占4列：名称(8)、十六进制(18，含0x)、十进制(12)
    for (int i = 0; i < 32; i++) {
        printf("%-8s 0x%016lx %12ld", reg_name(i), gpr(i), gpr(i));
        if ((i + 1) % 4 == 0) {
            printf("\n");   // 每四个换行
        } else {
            printf("  ");   // 两个空格分隔寄存器块（不用制表符）
        }
    }
    
    // 打印 CSR 寄存器，单独一行，保持对齐
    printf("\n");  // 在寄存器组后换行
    printf("%-8s 0x%016lx %12ld  ", "mcause", cpu.mcause, cpu.mcause);
    printf("%-8s 0x%016lx %12ld  ", "mstatus", cpu.mstatus, cpu.mstatus);
    printf("%-8s 0x%016lx %12ld  ", "mepc", cpu.mepc, cpu.mepc);
    printf("%-8s 0x%016lx %12ld\n", "mtvec", cpu.mtvec, cpu.mtvec); 
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int idx=0;
  char str[10];
  strcpy(str,s+1); //去除最左边的$
  if(strcmp(str,"pc")==0) return cpu.pc; //如果是pc那就返回cpu.pc的值
  for(int i=0;i<32;i++){
    if(strcmp(regs[i],str)==0){
      idx=i; //返回索引值
      break;
    }
    if(i==31) *success=false;
  }
  return gpr(idx);
}
