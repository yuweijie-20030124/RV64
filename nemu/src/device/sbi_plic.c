#include <utils.h>
#include <device/map.h>
#include "isa.h"

static uint32_t *sbi_plic_base = NULL;

#define PLIC_PENDING_BASE 0x1000
#define PLIC_ENABLE_BASE 0x2000
#define PLIC_ENABLE_STRIDE 0x80
#define PLIC_CONTEXT_BASE 0x200000
#define PLIC_CONTEXT_STRIDE 0x1000
#define PLIC_CONTEXT_CLAIM 0x4
#define S_MODE_EXTERNAL_INTERRUPT 9

static bool sbi_plic_irq_pending(uint32_t irq)
{
    uint32_t word = irq / 32;
    uint32_t bit = irq % 32;
    return (sbi_plic_base[(PLIC_PENDING_BASE / 4) + word] & (1u << bit)) != 0;
}

static bool sbi_plic_irq_enabled(uint32_t irq)
{
    uint32_t word = irq / 32;
    uint32_t bit = irq % 32;
    for (int context = 0; context < CONFIG_SBI_PLIC_CONTEXT_COUNT; context++) {
        uint32_t enable_base = PLIC_ENABLE_BASE + context * PLIC_ENABLE_STRIDE;
        if (sbi_plic_base[(enable_base / 4) + word] & (1u << bit)) {
            return true;
        }
    }
    return false;
}

static uint32_t sbi_plic_next_irq()
{
    for (uint32_t irq = 1; irq <= CONFIG_SBI_PLIC_INT_SOURCE_COUNT; irq++) {
        if (sbi_plic_irq_pending(irq) && sbi_plic_irq_enabled(irq)) {
            return irq;
        }
    }
    return 0;
}

static void sbi_plic_update_mip()
{
    if (sbi_plic_next_irq() != 0) {
        cpu.mip |= (1UL << S_MODE_EXTERNAL_INTERRUPT);
    } else {
        cpu.mip &= ~(1UL << S_MODE_EXTERNAL_INTERRUPT);
    }
}

void sbi_plic_set_pending(uint32_t irq)
{
    assert(irq <= CONFIG_SBI_PLIC_INT_SOURCE_COUNT);
    uint32_t word = irq / 32;
    uint32_t bit = irq % 32;
    sbi_plic_base[(PLIC_PENDING_BASE / 4) + word] |= (1u << bit);
    sbi_plic_update_mip();
}

void sbi_plic_clear_pending(uint32_t irq)
{
    assert(irq <= CONFIG_SBI_PLIC_INT_SOURCE_COUNT);
    uint32_t word = irq / 32;
    uint32_t bit = irq % 32;
    sbi_plic_base[(PLIC_PENDING_BASE / 4) + word] &= ~(1u << bit);
    sbi_plic_update_mip();
}

static void sbi_plic_io_handler(uint32_t offset, int len, bool is_write)
{
    uint32_t local_offset = offset;
    assert(len <= 4);
    assert(offset <= (2 * 1024 + 4 * CONFIG_SBI_PLIC_CONTEXT_COUNT) * 1024);
    if (offset >= PLIC_CONTEXT_BASE) {
        uint32_t context_offset = (offset - PLIC_CONTEXT_BASE) & (PLIC_CONTEXT_STRIDE - 1);
        if (context_offset == PLIC_CONTEXT_CLAIM) {
            if (is_write) {
                sbi_plic_update_mip();
            } else {
                uint32_t irq = sbi_plic_next_irq();
                sbi_plic_base[local_offset / 4] = irq;
                if (irq != 0) {
                    sbi_plic_clear_pending(irq);
                }
            }
        }
    } else if (is_write) {
        sbi_plic_update_mip();
    }
}

void init_sbi_plic()
{
    sbi_plic_base = (uint32_t *)new_space((2 * 1024 + 64) * 1024);
    add_mmio_map("sbi_plic", CONFIG_SBI_PLIC_MMIO, sbi_plic_base, (2 * 1024 + 64) * 1024, sbi_plic_io_handler);
    memset(sbi_plic_base, '\0', (2 * 1024 + 64) * 1024);
}
