#ifndef __ISA_H__
#define __ISA_H__

#include <common.h>

//csr数组顺序
/*
mcause   0
mstatus  1
mepc     2
mtvec    3
mhartid  4
mscratch 5
*/
typedef struct {
  word_t gpr[32];
  vaddr_t pc;
  // vaddr_t csr[6] ;
} NPC_reg;

// decode
typedef struct Decode {
  word_t pc;
  word_t snpc; // static next pc
  word_t val;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;

extern NPC_reg cpu;
extern Decode s;

#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif