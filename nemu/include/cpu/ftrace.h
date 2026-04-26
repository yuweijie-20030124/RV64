#include <common.h>

void parse_elf(const char *elf_file);
void ret_trace(paddr_t pc);
void call_trace(paddr_t pc, paddr_t target);