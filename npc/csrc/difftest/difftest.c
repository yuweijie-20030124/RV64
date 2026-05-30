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

#include <dlfcn.h>

#include <isa.h>
#include <common.h>
#include <memory.h>
#include <difftest.h>
#include <reg.h>
#include "svdpi.h"

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

void isa_reg_display();

const char *nemu_regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};
    
// const char *csrs_regs[] = {
//   "mcause","mstatus","mepc","mtvec","mhartid","mscratch"
// };

#ifdef CONFIG_DIFFTEST

static bool is_skip_ref = false;
static int skip_dut_nr_inst = 0;

// this is used to let ref skip instructions which
// can not produce consistent behavior with NEMU
void difftest_skip_ref() {
  is_skip_ref = true;
  // If such an instruction is one of the instruction packing in QEMU
  // (see below), we end the process of catching up with QEMU's pc to
  // keep the consistent behavior in our best.
  // Note that this is still not perfect: if the packed instructions
  // already write some memory, and the incoming instruction in NEMU
  // will load that memory, we will encounter false negative. But such
  // situation is infrequent.
  skip_dut_nr_inst = 0;
}

// this is used to deal with instruction packing in QEMU.
// Sometimes letting QEMU step once will execute multiple instructions.
// We should skip checking until NEMU's pc catches up with QEMU's pc.
// The semantic is
//   Let REF run `nr_ref` instructions first.
//   We expect that DUT will catch up with REF within `nr_dut` instructions.
void difftest_skip_dut(int nr_ref, int nr_dut) {
  skip_dut_nr_inst += nr_dut;

  while (nr_ref -- > 0) {
    ref_difftest_exec(1);
  }
}

void init_difftest(char *ref_so_file, long img_size, int port) {
  // printf("diff_so_file = %s\n",ref_so_file);
  // printf("img_size = %ld\n",img_size);
  assert(ref_so_file != NULL);
  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY);//打开传入的动态库文件 ref_so_file
  assert(handle);
  //打开动态链接对动态库中API符号进行符号解析和重定位，返回他们的地址，
  ref_difftest_memcpy = (void (*)(paddr_t, void*, size_t, bool))dlsym(handle, "difftest_memcpy");
  assert(ref_difftest_memcpy);

  ref_difftest_regcpy = (void (*)(void*, bool))dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

  ref_difftest_exec = (void (*)(uint64_t))dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  ref_difftest_raise_intr = (void (*)(uint64_t))dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

  void (*ref_difftest_init)(int) =  (void (*)(int))dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);

  ref_difftest_init(port);
  ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  // ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
}

bool isa_difftest_checkregs(NPC_reg *ref_r, vaddr_t pc) {
  update_reg();
  int reg_num = (int)(sizeof(cpu.gpr) / sizeof(cpu.gpr[0]));
  // int csr_num = (int)(sizeof(cpu.csr) / sizeof(cpu.csr[0]));
  // printf("pc = 0x%08x!!!!!\n",cpu.pc);
  // printf("进来啦!!!!!!!!!\n");
  for (int i = 0; i < reg_num; i++) {
    if (cpu.gpr[i] != ref_r->gpr[i]) {
        //  printf("进来啦!!!!!!!!!\n");
        //  printf("i = %d\n",i);
        //  printf("cpu.gpr = %d\n",cpu.gpr[i]);
    printf("PC get wrong at 0x%08x\n",cpu.pc);
    printf("reg wrong!!!!!!!!!\n");
    printf("different reg is %s\n",nemu_regs[i]);
    printf("error register is 0x%08x\t%d\n",cpu.gpr[i],cpu.gpr[i]);
    printf("right register is 0x%08x\t%d\n",ref_r->gpr[i],ref_r->gpr[i]);
    

    printf("\n");
    printf("nemu reg:\n");
    // isa_reg_display();
    printf("PC     : 0x%08x %d\n", ref_r->pc, ref_r->pc);
    for (int i = 0; i < 32; i++) {
        printf("%-4s   : 0x%08x %-11d  ", nemu_regs[i], ref_r->gpr[i], ref_r->gpr[i]);
        if ((i + 1) % 4 == 0) printf("\n");
    }

  
    return false;
    }
  }


  // printf("difftest cpu.pc    = 0x%08x\n",cpu.pc);
  // printf("difftest ref_r->pc = 0x%08x\n",ref_r->pc);
  if (cpu.pc != ref_r->pc) {
    printf("pc wrong!!!!!!!!!\n");
    printf("wrong pc = 0x%08x\n",cpu.pc);
    printf("right pc = 0x%08x\n",ref_r->pc);

    printf("\n");
    printf("nemu reg:\n"); 
    isa_reg_display();
    return false;
  }

  return true;
}

static void checkregs(NPC_reg *ref, vaddr_t pc) {
    //printf("进来啦!!!!!!!!!\n");
  if (!isa_difftest_checkregs(ref, pc)) {
    npc_state.state = NPC_ABORT;
    npc_state.halt_pc = pc;
    printf("\n");
    printf("npc reg:\n");
    
    isa_reg_display();
    printf("\n");
  }
}

void difftest_step(vaddr_t pc, vaddr_t npc) {
  NPC_reg ref_r;
  
if (skip_dut_nr_inst > 0) {
    ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
    if (ref_r.pc == npc) {
      skip_dut_nr_inst = 0;
      checkregs(&ref_r, npc);
      return;
    }
    skip_dut_nr_inst --;
    if (skip_dut_nr_inst == 0)
      panic("can not catch up with ref.pc = " FMT_WORD " at pc = " FMT_WORD, ref_r.pc, pc);
    return;
  }

  if (is_skip_ref) {
    update_reg();
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    is_skip_ref = false;
    return;
  }

  ref_difftest_exec(1);
  ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
  // printf("ref_r.pc = 0x%08x\n",ref_r.pc);
  // printf("npc.pc   = 0x%08x\n",pc);
  checkregs(&ref_r, pc);
}
#else
void init_difftest(char *ref_so_file, long img_size, int port) { }
#endif
