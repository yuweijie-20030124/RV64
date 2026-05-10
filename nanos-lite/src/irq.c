#include <common.h>
#include <proc.h>

Context *do_syscall(Context *c);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD: printf("EVENT_YIELD:yield!\n");c = schedule(c); break;
    case EVENT_SYSCALL: c = do_syscall(c); Log("do_syscall"); break;
    default: panic("Unhandled eventdd ID = %d", e.event);
  }
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
