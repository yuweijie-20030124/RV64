include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/npc.mk

CROSS_COMPILE := riscv64-unknown-elf-
COMMON_CFLAGS := -fno-pic -march=rv64ima_zicsr -mabi=lp64 -mcmodel=medany -mstrict-align

CFLAGS  += $(COMMON_CFLAGS) -DISA_H=\"riscv/riscv.h\"
ASFLAGS += $(COMMON_CFLAGS)
LDFLAGS += -melf64lriscv
