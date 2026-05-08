#include <proc.h>
#include <elf.h>
#include <fs.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

extern uintptr_t loader(PCB *pcb, const char *filename);

void init_proc() {
  switch_boot_pcb();

  Log("Initializing processes...");

  // load the first user program (dummy)
  uintptr_t entry = loader(&pcb[0], "/bin/dummy");
  pcb[0].cp = kcontext((Area){pcb[0].stack, pcb[0].stack + STACK_SIZE}, (void *)entry, NULL);
}

Context* schedule(Context *prev) {
  // save the context of the previous process
  if (prev != NULL) {
    current->cp = prev;
  }

  // simple round-robin: always switch to pcb[0]
  current = &pcb[0];

  return current->cp;
}
