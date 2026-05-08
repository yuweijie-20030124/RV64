#include <proc.h>
#include <elf.h>
#include <fs.h>
#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
# define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_AM_NATIVE__) || defined(__ISA_X86_64__)
# define EXPECT_TYPE EM_X86_64
#else
# error Unsupported ISA
#endif

#define USER_HEAP_GAP (8 * 1024 * 1024)

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  uint32_t phdr_size = sizeof(Elf_Phdr);
  Elf_Ehdr ehdr;
  Elf_Phdr phdr;
  uintptr_t max_brk = 0;
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  assert(EXPECT_TYPE == ehdr.e_machine);
  uint32_t phdr_num = ehdr.e_phnum;
  for (int i = 0; i < phdr_num; i++) {
    fs_lseek(fd, ehdr.e_phoff + i * phdr_size, SEEK_SET);
    fs_read(fd, &phdr, phdr_size);
    if (phdr.p_type != PT_LOAD) continue;
    fs_lseek(fd, phdr.p_offset, SEEK_SET);
    fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
    memset((void *)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
    uintptr_t brk = phdr.p_vaddr + phdr.p_memsz;
    if (max_brk < brk) {
      max_brk = brk;
    }
  }
  fs_close(fd);
  if (pcb != NULL) {
    pcb->max_brk = ROUNDUP(max_brk, PGSIZE);
  }
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

