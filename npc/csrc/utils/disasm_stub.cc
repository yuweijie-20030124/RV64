#include <cstdio>
#include <cstdint>

extern "C" void init_disasm(const char *triple) {
    (void)triple;
}

extern "C" void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte) {
    (void)pc;
    (void)code;
    (void)nbyte;
    snprintf(str, size, "unimp");
}
