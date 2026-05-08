#include <common.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD: printf("EVENT_YIELD:yield!\n"); break;
    case EVENT_SYSCALL: /*do_syscall(c)*/; printf("EVENT_SYSCALL:syscall!\n"); break;
    default: panic("Unhandled eventdd ID = %d", e.event);
  }
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
