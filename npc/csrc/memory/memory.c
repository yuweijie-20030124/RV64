#include "memory.h"
#include "common.h"
#include "reg.h"
#include "svdpi.h"
#include <stdio.h>

void mmio_write(paddr_t addr, int len, word_t data);
word_t mmio_read(paddr_t addr, int len);

paddr_t host_read(void *addr, int len) {
  switch (len) {
    case 1: return *(uint8_t  *)addr;
    case 2: return *(uint16_t *)addr;
    case 4: return *(uint32_t *)addr;
    IFDEF(CONFIG_ISA64, case 8: return *(paddr_t *)addr);
    default: MUXDEF(CONFIG_RT_CHECK, assert(0), return 0);
  }
}

void host_write(void *addr, int len, paddr_t data) {
  //printf("%lx\n",(uint64_t *)addr);
  switch (len) {
    case 1: *(uint8_t  *)addr = data; return;
    case 2: *(uint16_t *)addr = data; return;
    case 4: *(uint32_t *)addr = data; return;
    IFDEF(CONFIG_ISA64, case 8: *(paddr_t *)addr = data; return);
    IFDEF(CONFIG_RT_CHECK, default: assert(0));
  }
}


//读0x8000000内存
word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

//写内存0x80000000
void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

//检查内存0x80000000是否越界
void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

//读物理地址
word_t paddr_read(paddr_t addr, int len) {
  //printf("进来了\n"); 
  if (likely(in_pmem(addr))) {
    IFDEF(CONFIG_MTRACE, Log("read in address = " FMT_PADDR ", len = %d\n", addr, len));
    return pmem_read(addr, len);
  }

  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));

  out_of_bound(addr);
  return 0;
}

//写物理地址
void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) {
    pmem_write(addr, len, data);
    IFDEF(CONFIG_MTRACE, Log("write in address = " FMT_PADDR ", len = %d, data = " FMT_WORD "\n", addr, len, data));
    return;
  }

  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);

  out_of_bound(addr);
}

extern "C" void sim_sram_read(paddr_t raddr, int *rdata){
    assert(rdata != NULL);
    *rdata = (int)paddr_read(raddr, 4);
}

extern "C" void sim_sram_write(paddr_t waddr, paddr_t wdata, uint8_t wmask){
    for (int i = 0; i < 4; i++) {
      if ((wmask >> i) & 0x1) {
        paddr_write(waddr + i, 1, (wdata >> (i * 8)) & 0xff);
      }
    }
}

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

//读地址 == 取指
word_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}
//写地址
void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
}

#ifdef CONFIG_MTRACE

extern "C" void Log_mem_read(paddr_t addr){
    if ((addr < CONFIG_MRACE_START) || (addr >= CONFIG_MRACE_END))
        return;
    word_t val;
    pmem_read(addr,&val);
    Log_mem("PC is " FMT_WORD ", Read  Addr: " FMT_PADDR " Data: " FMT_WORD "\n", get_gpr(32), addr, val);
}

extern "C" void Log_mem_wirte(paddr_t addr,word_t data,uint8_t wmask){
    if ((addr < CONFIG_MRACE_START) || (addr >= CONFIG_MRACE_END))
        return;
    if(wmask==0xff)     Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr    , (uint64_t)data                  , wmask);
    else if(wmask==0x0f)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 0, (uint64_t)(uint32_t)(data >> 0 ), wmask);
    else if(wmask==0xf0)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 4, (uint64_t)(uint32_t)(data >> 32), wmask);
    else if(wmask==0x03)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 0, (uint64_t)(uint16_t)(data >> 0 ), wmask);
    else if(wmask==0x0c)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 2, (uint64_t)(uint16_t)(data >> 16), wmask);
    else if(wmask==0x30)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 4, (uint64_t)(uint16_t)(data >> 32), wmask);
    else if(wmask==0xc0)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 6, (uint64_t)(uint16_t)(data >> 48), wmask);
    else if(wmask==0x01)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 0, (uint64_t)(uint8_t )(data >> 0 ), wmask);
    else if(wmask==0x02)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 1, (uint64_t)(uint8_t )(data >> 8 ), wmask);
    else if(wmask==0x04)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 2, (uint64_t)(uint8_t )(data >> 16), wmask);
    else if(wmask==0x08)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 3, (uint64_t)(uint8_t )(data >> 24), wmask);
    else if(wmask==0x10)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 4, (uint64_t)(uint8_t )(data >> 32), wmask);
    else if(wmask==0x20)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 5, (uint64_t)(uint8_t )(data >> 40), wmask);
    else if(wmask==0x40)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 6, (uint64_t)(uint8_t )(data >> 48), wmask);
    else if(wmask==0x80)Log_mem("PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr + 7, (uint64_t)(uint8_t )(data >> 56), wmask);
    else panic("error size: PC is " FMT_WORD ", Write Addr: " FMT_PADDR " Data: " FMT_WORD " wmask is 0x%02x\n", get_gpr(32), addr, data, wmask);
}

#else

extern "C" void Log_mem_read(paddr_t addr) {}
extern "C" void Log_mem_wirte(paddr_t addr, word_t data, char wmask) {}

#endif


uint8_t mem[CONFIG_MSIZE] = {0};

// Memory transfer
uint8_t* guest_to_host(paddr_t addr) { return mem + (addr - CONFIG_MBASE); }

const static uint32_t img [] = {
  // 0x00100073,   // ebreak (used as nemu_trap)     0x8000_0018
  0x00130393,   // addi t2, t1, 1    t2 = t1 + 1  0x8000_0000
  0x00c000ef,   // jal ra ,80000010               0x8000_0004 
  0x00240493,   // addi s1, s0, 2    s1 = s0 + 2  0x8000_0008 这个一定不执行
  0x00350593,   // addi a1, a0, 3    a1 = a0 + 3  0x8000_000C 这个一定不执行
  0x00460693,   // addi a3, a2, 4    a3 = a2 + 4  0x8000_0010 跳到这里来
  0x00570793,	  // addi a5, a4, 5    a5 = a4 + 5  0x8000_0014
  0x00100073,   // ebreak (used as nemu_trap)     0x8000_0018
  0x0000006f,   // j self*/
};

void init_mem() {
  /* Load built-in image. */
  //pmem 0x80000000~0x8ffffff
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));
} 
