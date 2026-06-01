AM_SRCS := riscv/npc/start.S \
           riscv/npc/trm.c \
           riscv/npc/ioe.c \
           riscv/npc/timer.c \
           riscv/npc/input.c \
           riscv/npc/cte.c \
           riscv/npc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

NPC_HOME ?= $(abspath $(AM_HOME)/../npc)
NEMU_HOME ?= $(abspath $(AM_HOME)/../nemu)

CFLAGS    += -g -fdata-sections -ffunction-sections
LDSCRIPTS += $(AM_HOME)/scripts/linker.ld
LDFLAGS   += --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
NPCFLAGS  += -l $(shell dirname $(IMAGE).elf)/npc-log.txt
NPCFLAGS  += -b
NPCFLAGS  += -d --diff=$(NEMU_HOME)/build/riscv64-nemu-interpreter-so

MAINARGS_MAX_LEN = 64
MAINARGS_PLACEHOLDER = the_insert-arg_rule_in_Makefile_will_insert_mainargs_here
CFLAGS += -DMAINARGS_MAX_LEN=$(MAINARGS_MAX_LEN) -DMAINARGS_PLACEHOLDER=$(MAINARGS_PLACEHOLDER)

insert-arg: image
	@python $(AM_HOME)/tools/insert-arg.py $(IMAGE).bin $(MAINARGS_MAX_LEN) $(MAINARGS_PLACEHOLDER) "$(mainargs)"

image: image-dep
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

check-npc-home:
	@if [ ! -f "$(NPC_HOME)/Makefile" ]; then \
		echo "Error: NPC project not found under $(NPC_HOME)"; \
		exit 1; \
	fi

run: insert-arg check-npc-home
	$(MAKE) -C $(NPC_HOME) run NPC_IMG=$(IMAGE).bin NPC_ELF=$(IMAGE).elf NPC_FLAGS="$(NPCFLAGS)"

gdb: insert-arg check-npc-home
	$(MAKE) -C $(NPC_HOME) gdb NPC_IMG=$(IMAGE).bin NPC_ELF=$(IMAGE).elf NPC_FLAGS="$(NPCFLAGS)"

.PHONY: insert-arg check-npc-home
