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
#include <cpu/cpu.h>
#include <difftest-def.h>
#include <memory/paddr.h>
#include <assert.h>
#include <cpu/decode.h>
#include "isa-def.h"

typedef enum {
  DIFFTEST_PRV_U = 0,
  DIFFTEST_PRV_S = 1,
  DIFFTEST_PRV_M = 3,
} DifftestPrivileged;

typedef struct {
  word_t gpr[32];
  word_t pc;
  DifftestPrivileged privilege;
  word_t mtvec, mstatus, mcause, mepc, mtval;
  word_t stvec, sepc, scause, stval;
  word_t medeleg, mideleg;
  word_t mip, mie;
  word_t mcycle, minstret, mhpmcounter[29], mhpmevent[29];
  uint32_t mcountinhibit;
  word_t mscratch;
  word_t sscratch;
  word_t satp;
#ifndef CONFIG_RV64
  word_t mcycleh, minstreth, mhpmcounterh[29], mhpmeventh[29];
#endif
  word_t misa, menvcfg, mseccfg;
  word_t senvcfg;
  uint32_t mcounteren;
  uint32_t scounteren;
#ifndef CONFIG_RV64
  word_t mstatush;
  word_t menvcfgh, mseccfgh;
#endif
  word_t mvendorid, marchid, mimpid, mhartid, mconfigptr;
} DifftestCPUState;

static DifftestCPUState ref_shadow;

__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
    if (direction == DIFFTEST_TO_REF){
        memcpy(guest_to_host(addr), buf, n);
    }
}


__EXPORT void difftest_regcpy(void *dut, bool direction) {
    DifftestCPUState *dut_regs = (DifftestCPUState *)dut;
    if (direction == DIFFTEST_TO_REF){
        memcpy(&ref_shadow, dut_regs, sizeof(ref_shadow));
        for (int i = 0; i < 32;i++){
            cpu.gpr[i] = dut_regs->gpr[i];
        }
        cpu.pc = dut_regs->pc;
        cpu.mtvec = dut_regs->mtvec;
        cpu.mstatus = dut_regs->mstatus;
        cpu.mcause = dut_regs->mcause;
        cpu.mepc = dut_regs->mepc;
    }
    else if (direction == DIFFTEST_TO_DUT)
    {
        memcpy(dut_regs, &ref_shadow, sizeof(ref_shadow));
        for (int i = 0; i < 32; i++)
        {
            dut_regs->gpr[i] = cpu.gpr[i];
        }
        dut_regs->pc = cpu.pc;
        dut_regs->privilege = DIFFTEST_PRV_M;
        dut_regs->mtvec = cpu.mtvec;
        dut_regs->mstatus = cpu.mstatus;
        dut_regs->mcause = cpu.mcause;
        dut_regs->mepc = cpu.mepc;
    }
    else{
        assert(0);
    }
}

__EXPORT void difftest_exec(uint64_t n) {
    cpu_exec(n);
    //   assert(0);
}

__EXPORT void difftest_raise_intr(word_t NO) {
  cpu.pc = isa_raise_intr(NO, cpu.pc);
}

__EXPORT void difftest_init(int port) {
  void init_mem();
  init_mem();
  /* Perform ISA dependent initialization. */
  init_isa();
}
