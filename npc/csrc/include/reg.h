#ifndef __REG_H__
#define __REG_H__

#include <common.h>
#include "isa.h"

static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < 32));
  return idx;
}

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

static inline const char* reg_name(int idx) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}

word_t isa_reg_str2val(const char *s, bool *success);

//void get_reg_value(int data);

word_t get_gpr(int i);

void update_reg(void);

#endif