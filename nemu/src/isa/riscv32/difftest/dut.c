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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

extern const char *regs[];



bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  int reg_num = (int)(sizeof(cpu.gpr) / sizeof(cpu.gpr[0]));
  if(ref_r->mcause != cpu.mcause){
      printf("mcause is wrong\n");
      return false;
  }
  for (int i = 0; i < reg_num; i++) {
    if (ref_r->gpr[i] != cpu.gpr[i]) {
      printf("reg %s is wrong\n",regs[i]);
      printf("wrong reg value is 0x%08lx\n",cpu.gpr[i]);
      printf("right reg value is 0x%08lx\n",ref_r->gpr[i]);
      return false;
    }
  }

  // if (ref_r->mtvec != cpu.mtvec) {
  //   printf("csr reg mtvec is wrong\n");
  //   printf("wrong reg value is 0x%08lx\n",cpu.mtvec);
  //   printf("right reg value is 0x%08lx\n",ref_r->mtvec);
  //   return false;
  // }
  // if (ref_r->mepc != cpu.mepc) {
  //   printf("csr reg mepc is wrong\n");
  //   printf("wrong reg value is 0x%08lx\n",cpu.mepc);
  //   printf("right reg value is 0x%08lx\n",ref_r->mepc);
  //   return false;
  // }
  // if (ref_r->mcause != cpu.mcause) {
  //   printf("csr reg mcause is wrong\n");
  //   printf("wrong reg value is 0x%08lx\n",cpu.mcause);
  //   printf("right reg value is 0x%08lx\n",ref_r->mcause);
  //   return false;
  // }
  // if (ref_r->mstatus != cpu.mstatus) {
  //   printf("csr reg mstatus is wrong\n");
  //   printf("wrong reg value is 0x%08lx\n",cpu.mstatus);
  //   printf("right reg value is 0x%08lx\n",ref_r->mstatus);
  //   return false;
  // }

  if (ref_r->pc != cpu.pc) {
    printf("pc is wrong\n");
    printf("wrong pc is 0x%08lx\n",cpu.pc);
    printf("right pc is 0x%08lx\n",ref_r->pc);
    return false;
  }
  return true;
}

void isa_difftest_attach() {
}
