// /***************************************************************************************
// * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
// *
// * NEMU is licensed under Mulan PSL v2.
// * You can use this software according to the terms and conditions of the Mulan PSL v2.
// * You may obtain a copy of Mulan PSL v2 at:
// *          http://license.coscl.org.cn/MulanPSL2
// *
// * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// *
// * See the Mulan PSL v2 for more details.
// ***************************************************************************************/

#include <dlfcn.h>
// 使用 -I 搜索路径找到 capstone 头文件，不再硬编码绝对路径
#include <capstone/capstone.h>
#include <common.h>

static size_t (*cs_disasm_dl)(csh handle, const uint8_t *code,
    size_t code_size, uint64_t address, size_t count, cs_insn **insn);
static void (*cs_free_dl)(cs_insn *insn, size_t count);

static csh handle;

void init_disasm() {
  void *dl_handle;
  // 通过编译期宏 NEMU_HOME_STR 拼接 capstone 动态库路径，避免硬编码
  dl_handle = dlopen(NEMU_HOME_STR "/tools/capstone/repo/libcapstone.so.5", RTLD_LAZY);
  assert(dl_handle);

  // 显式转换 dlsym 返回的 void* 为函数指针类型
  cs_err (*cs_open_dl)(cs_arch, cs_mode, csh*) = 
      (cs_err (*)(cs_arch, cs_mode, csh*))dlsym(dl_handle, "cs_open");
  assert(cs_open_dl);

  cs_disasm_dl = (size_t (*)(csh, const uint8_t*, size_t, uint64_t, size_t, cs_insn**))
      dlsym(dl_handle, "cs_disasm");
  assert(cs_disasm_dl);

  cs_free_dl = (void (*)(cs_insn*, size_t))dlsym(dl_handle, "cs_free");
  assert(cs_free_dl);

  cs_arch arch = CS_ARCH_RISCV;
  

  cs_mode mode = static_cast<cs_mode>(
    MUXDEF(CONFIG_ISA64, CS_MODE_RISCV64, CS_MODE_RISCV32) | 
    CS_MODE_RISCVC
  );

    //printf("Initializing Capstone: arch=%d, mode=%d\n", arch, mode); 

  int ret = cs_open_dl(arch, mode, &handle);
  assert(ret == CS_ERR_OK);
}

//将指令进行反汇编
// str: 存储反汇编结果的字符串
// size: str 的大小
// pc: 指令的地址
// code: 指令的机器码
// nbyte: 指令的字节数

void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte) {
	cs_insn *insn;
  // printf("dis pc = 0x%08lx\n",pc);
  // printf("dis code = 0x%08x\n",*code);
	size_t count = cs_disasm_dl(handle, code, nbyte, pc, 0, &insn);
  // no assert anymore, because some instructions may be unrecognized by Capstone
  // Capstone 可能无法识别某些扩展指令（如 Zba），容错回退到显示原始字节。
  if (count != 1) {
    if (count == 0) {
      snprintf(str, size, "invalid (raw: %02x %02x %02x %02x)",
               code[0], code[1], code[2], code[3]);
    } else {
      snprintf(str, size, "multi-insn (count=%zu)", count);
      cs_free_dl(insn, count);
    }
    return;
  }

  int ret = snprintf(str, size, "%s", insn->mnemonic);
  if (insn->op_str[0] != '\0') {
    snprintf(str + ret, size - ret, "\t%s", insn->op_str);
  }
  cs_free_dl(insn, count);
}

// void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte) {
//   cs_insn *insn;
//   size_t count = cs_disasm_dl(handle, code, nbyte, pc, 0, &insn);
  
//   if (count != 1) {
//     if (count == 0) {
//       snprintf(str, size, "invalid (raw: %02x %02x %02x %02x)", 
//                code[0], code[1], code[2], code[3]);
//     } else {
//       snprintf(str, size, "multi-insn (count=%zu)", count);
//       cs_free_dl(insn, count);
//     }
//     return;
//   }

//   int ret = snprintf(str, size, "%s", insn->mnemonic);
//   if (insn->op_str[0] != '\0') {
//     snprintf(str + ret, size - ret, "\t%s", insn->op_str);
//   }
//   cs_free_dl(insn, count);
// }