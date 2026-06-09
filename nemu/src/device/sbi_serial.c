#include <utils.h>
#include <device/map.h>
#include <SDL2/SDL.h>
#include <sys/select.h>
#include <unistd.h>

#define _NUM(f) \
    f(0) f(1) f(2) f(3) f(4) f(5) f(6) f(7) f(8) f(9)
#define _A_Z(f) \
    f(A) f(B) f(C) f(D) f(E) f(F) f(G) f(H) f(I) f(J) f(K) f(L) f(M) f(N) f(O) f(P) f(Q) f(R) f(S) f(T) f(U) f(V) f(W) f(X) f(Y) f(Z)


// It's conflicted on macos with sys/_types/_key_t.h
#ifdef __APPLE__
  #undef _KEY_T 
#endif

#define SDL_KEYMAP(k) keymap[concat(SDL_SCANCODE_, k)] = *str(k);
static uint8_t keymap[256] = {};
static bool key_status;
static void sbi_serial_update_irq();

static void init_keymap() {
    MAP(_NUM, SDL_KEYMAP)
    MAP(_A_Z, SDL_KEYMAP)
    keymap[SDL_SCANCODE_TAB] = 0x9;
    keymap[SDL_SCANCODE_SPACE] = ' ';
    keymap[SDL_SCANCODE_RETURN] = 0xD;
    keymap[SDL_SCANCODE_BACKSPACE] = 0x8;
}

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0;

static void key_enqueue(uint8_t am_scancode){
    key_queue[key_r] = am_scancode;
    key_r = (key_r + 1) % KEY_QUEUE_LEN;
    Assert(key_r != key_f, "key queue overflow!");
}

static void poll_stdin_key()
{
    while (true) {
        fd_set readfds;
        struct timeval timeout = {0, 0};
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
        if (ret <= 0 || !FD_ISSET(STDIN_FILENO, &readfds)) {
            break;
        }

        uint8_t ch = 0;
        if (read(STDIN_FILENO, &ch, 1) != 1) {
            break;
        }
        key_enqueue(ch);
    }
}

static bool key_available()
{
    poll_stdin_key();
    return key_f != key_r;
}

static uint8_t key_dequeue() {
  poll_stdin_key();
  uint8_t key = 0;
  if (key_f != key_r) {
    key = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
    if((key >= 'A') && (key <= 'Z')){
        key = (key_status) ? (key - 'A' + 'a') : key;
    }
  }
  return key;
}

void send_key_sbi_serial(uint8_t scancode, bool is_keydown){
    if (nemu_state.state == NEMU_RUNNING && (keymap[scancode] != 0) && (!is_keydown)){
        uint8_t am_scancode = keymap[scancode];
        key_enqueue(am_scancode);
        sbi_serial_update_irq();
    }
    if ((scancode == SDL_SCANCODE_LSHIFT) || (scancode == SDL_SCANCODE_AC_REFRESH)){
        if(is_keydown){
            key_status = 0;
        }else{
            key_status = 1;
        }
    }
}

#define DR_OFFSET  0
#define IER_OFFSET 1
#define IIR_OFFSET 2
#define LCR_OFFSET 3
#define MCR_OFFSET 4
#define LSR_OFFSET 5
#define MSR_OFFSET 6
#define SCR_OFFSET 7

#define UART_IRQ 10
#define UART_IER_RDI  0x01
#define UART_IER_THRI 0x02
#define UART_IIR_NO_INT 0x01
#define UART_IIR_THRI   0x02
#define UART_IIR_RDI    0x04
#define UART_IIR_FIFO_BITS 0xc0
#define UART_LSR_DR   0x01
#define UART_LSR_THRE 0x20
#define UART_LSR_TEMT 0x40

static uint8_t *sbi_serial_base = NULL;
static uint8_t sbi_serial_IER  = 0;
static uint8_t sbi_serial_divh = 0;
static uint8_t sbi_serial_divl = 0;
static bool sbi_serial_tx_irq_pending = false;

void sbi_plic_set_pending(uint32_t irq);
void sbi_plic_clear_pending(uint32_t irq);

static void sbi_serial_update_irq()
{
    bool rx_irq = (key_f != key_r) && (sbi_serial_IER & UART_IER_RDI);
    bool tx_irq = sbi_serial_tx_irq_pending && (sbi_serial_IER & UART_IER_THRI);
    if (rx_irq || tx_irq) {
        sbi_plic_set_pending(UART_IRQ);
    } else {
        sbi_plic_clear_pending(UART_IRQ);
    }
}

void update_sbi_serial()
{
    poll_stdin_key();
    sbi_serial_update_irq();
}

static void sbi_serial_putc(char ch)
{
    MUXDEF(CONFIG_TARGET_AM, putch(ch), putc(ch, stderr));
}

static void sbi_serial_io_handler(uint32_t offset, int len, bool is_write)
{
    assert(len == 1);
    if (is_write){
        switch (offset){
            case DR_OFFSET:
                if (BITS(sbi_serial_base[3], 7, 7))
                    sbi_serial_divl = sbi_serial_base[0];
                else {
                    sbi_serial_putc(sbi_serial_base[0]);
                    sbi_serial_tx_irq_pending = true;
                    sbi_serial_update_irq();
                }
                break;
            case IER_OFFSET:
                if (BITS(sbi_serial_base[3], 7, 7))
                    sbi_serial_divh = sbi_serial_base[1];
                else {
                    sbi_serial_IER = sbi_serial_base[1];
                    if (sbi_serial_IER & UART_IER_THRI) {
                        sbi_serial_tx_irq_pending = true;
                    } else {
                        sbi_serial_tx_irq_pending = false;
                    }
                    sbi_serial_update_irq();
                }
                break;
            case IIR_OFFSET:
                // This register is FCR on writes. The FIFO controls do not
                // need state in this minimal 16550 model.
                break;
            case LCR_OFFSET:
                //? pass
                break;
            case MCR_OFFSET:
                sbi_serial_base[4] = 0x0;
                break;
            case LSR_OFFSET:
                //? pass
                break;
            case MSR_OFFSET:
                sbi_serial_base[6] = 0x0;
                break;
            case SCR_OFFSET:
                //? pass
                break;
            default:
                panic("do not support offset = %d", offset);
        }
    }else{
        switch (offset){
            case DR_OFFSET:
                if (BITS(sbi_serial_base[3], 7, 7))
                    sbi_serial_base[0] = sbi_serial_divl;
                else {
                    sbi_serial_base[0] = key_dequeue();
                    sbi_serial_update_irq();
                }
                break;
            case IER_OFFSET:
                if (BITS(sbi_serial_base[3], 7, 7))
                    sbi_serial_base[1] = sbi_serial_divh;
                else
                    sbi_serial_base[1] = sbi_serial_IER;
                break;
            case IIR_OFFSET:
                if (key_available() && (sbi_serial_IER & UART_IER_RDI)) {
                    sbi_serial_base[2] = UART_IIR_FIFO_BITS | UART_IIR_RDI;
                    sbi_serial_update_irq();
                } else if (sbi_serial_tx_irq_pending && (sbi_serial_IER & UART_IER_THRI)) {
                    sbi_serial_base[2] = UART_IIR_FIFO_BITS | UART_IIR_THRI;
                    sbi_serial_tx_irq_pending = false;
                    sbi_serial_update_irq();
                } else {
                    sbi_serial_base[2] = UART_IIR_FIFO_BITS | UART_IIR_NO_INT;
                }
                break;
            case LCR_OFFSET:
                //? pass
                break;
            case MCR_OFFSET:
                //? pass
                break;
            case LSR_OFFSET:
                sbi_serial_base[5] = UART_LSR_THRE | UART_LSR_TEMT | (key_available() ? UART_LSR_DR : 0);
                break;
            case MSR_OFFSET:
                //? pass
                break;
            case SCR_OFFSET:
                //? pass
                break;
            default:
                panic("do not support offset = %d", offset);
        }
    }
}


void init_sbi_serial()
{
    sbi_serial_base = new_space(8);
    add_mmio_map("sbi_serial", CONFIG_SBI_SERIAL_MMIO, sbi_serial_base, 8, sbi_serial_io_handler);
    sbi_serial_base[3] = 0x3;
    sbi_serial_base[4] = 0x0;
    sbi_serial_base[5] = UART_LSR_THRE | UART_LSR_TEMT;
    sbi_serial_base[6] = 0x0;
    sbi_serial_base[7] = 0x0;
    sbi_serial_tx_irq_pending = false;
    key_status = 1;
    init_keymap();
}
