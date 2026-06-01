#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload();


void switch_boot_pcb() {
  current = &pcb_boot; //里面是空的，但是注册了32kb的栈，运行内核的时候栈帧就落在这篇内存上。
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  switch_boot_pcb(); //用于启动的虚假进程

  Log("Initializing processes...");

  // load program here
  naive_uload(NULL, "/bin/hello");
  // naive_uload(NULL, "/bin/dummy");
  // naive_uload();
}

Context* schedule(Context *prev) {
  current->cp = prev;
  return current->cp;
}
