#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include <stdint.h>
#include <common.h>

#define __EXPORT __attribute__((visibility("default")))
// 编写大型程序时，可用-fvisibility=hidden设置符号默认隐藏，针对特定变量和函数，
// 在代码中使用__attribute__ ((visibility("default")))另该符号外部可见，这种方
//法可用有效避免so之间的符号冲突。
enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

#define RISCV_GPR_TYPE MUXDEF(CONFIG_RV64, uint64_t, uint32_t)
#define RISCV_GPR_NUM  MUXDEF(CONFIG_RVE , 16, 32)
#define DIFFTEST_REG_SIZE (sizeof(RISCV_GPR_TYPE) * (RISCV_GPR_NUM + 1)) // GPRs + pc

//前面是difftest-def.h的内容
//后面增加difftest.h的内容


#endif