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
    if(fd==-1)
        return -2;
    assert(fd > 0);
    // printf("filename is %c\n",*filename);
    int offset = fs_lseek(fd, 0, SEEK_SET);
    assert(offset == 0);
    Elf_Ehdr ehdr;  //ELF文件的文件头
    int len = fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
    assert(len == sizeof(Elf_Ehdr));
    //检查魔术头 7f E L F
    assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
    assert(ehdr.e_type == ET_EXEC); //文件类型是可执行文件
    assert(ehdr.e_machine == EM_RISCV);//architect是riscv的
    for (int i = 0; i < ehdr.e_phnum;i++){
        Elf_Phdr phdr;
        //遍历 ELF 文件的 Program Header Table（程序头表）
        offset = fs_lseek(fd, ehdr.e_phoff + (i * ehdr.e_phentsize), SEEK_SET);
        assert(offset == (ehdr.e_phoff + (i * ehdr.e_phentsize)));
        // len = ramdisk_read(&phdr, ehdr.e_phoff + (i * ehdr.e_phentsize), sizeof(Elf_Phdr));
        len = fs_read(fd, &phdr, sizeof(Elf_Phdr));
        assert(len == sizeof(Elf_Phdr));
        if(phdr.p_type==PT_LOAD){ //如果要加载到内存中
            offset = fs_lseek(fd, phdr.p_offset, SEEK_SET);
            assert(offset == phdr.p_offset);
            len = fs_read(fd, (void *)(phdr.p_vaddr), phdr.p_memsz);
            // len = ramdisk_read((void *)(phdr.p_vaddr), phdr.p_offset, phdr.p_memsz);
            // assert(len == phdr.p_memsz);
            // memset清空.bss段——ELF 文件加载的标准操作。
            memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
        }
    }
    //   TODO();
    return ehdr.e_entry;
}



void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
  Log("Jump out of entry = %p", entry);

}

