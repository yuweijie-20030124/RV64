#include <cstdlib>
#include <getopt.h>
#include "common.h"
#include "memory.h"
#include "stdio.h"

void init_log(const char *log_file);
void parse_elf(const char *elf_file);
void parse_mrom(const char *elf_file);
void init_rand();
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
#ifdef CONFIG_DEVICE
void init_device();
#endif
void init_sdb();
void init_disasm();
void init_isa();
void cpu_reset();

uint8_t* mrom_guest_to_host(paddr_t addr);
uint8_t* sram_guest_to_host(paddr_t addr);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NPC!\n", ANSI_FMT("riscv32", ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  
}

void sdb_set_batch_mode();

static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *flash_img_file = NULL;


static char *elf_file =NULL;
static char *log_file = NULL;
static int difftest_port = 1234;

//程序存到0x80000000
static long load_img() {
  if (img_file == NULL) {
    Log("0x80000000:No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }
  //printf ("%s!!!!!!!!!!\n",img_file);
  FILE *fp = fopen(img_file, "rb");//二进制读入imgfile
  Assert(fp, "0x800000000:Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);//将fp的指针移到最后位置
  long size = ftell(fp);//返回fp当前文件位置

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);//将fp的指针移到文件最开头
  //size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) 从给定流 stream 读取数据到 ptr 所指向的数组中。
  //如果fread读取成功就会返回nmemb，也就是“1”。
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  //从文件指针 fp 指向的文件中读取二进制数据，并将其直接写入到客户机（Guest）物理内存的 RESET_VECTOR 地址处
  
  assert(ret == 1);

  fclose(fp);
  return size;
}


//在这里开启是否批处理模式
//批处理模式下，sdb_mainloop()不会被调用
//而是直接执行cpu_exec(-1)来执行指令
static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"ftrace"   , required_argument, NULL, 'f'},
    {"img"      , required_argument, NULL, 'i'},
    {"mromimg"  , required_argument, NULL, 'm'},
    // {"sramimg"  , required_argument, NULL, 's'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:f:e:i:m:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'f': elf_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'i': img_file = optarg; break;
      // case 's': sram_img_file = optarg; break;
      case 1  : img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch                  run with batch mode\n");
        printf("\t-l,--log=FILE               output log to FILE\n");
        printf("\t-f,--ftrace=ELF_FILE        ftrace ELF to log\n");
        printf("\t-d,--diff=REF_SO            run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT              run DiffTest with port PORT\n");
        printf("\t-i,--img=img.bin            用户程序存放在0x80000000\n");
        printf("\t-m,--mromimg=mrom_img.bin   MROM在0x20000000\n");
        // printf("\t-s,--sramimg=sram_img.bin   SRAM在0x0f00_0000~0x0fff_ffff\n");

        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {

  /* Parse arguments. */
  parse_args(argc, argv);
  
  /* Set random seed. */
  init_rand();

  /* init log*/
  init_log(log_file);

  #ifdef CONFIG_FTRACE
  /* Open the elf file. */
  parse_elf(elf_file);
  #endif

  /* Initialize memory. */
  init_mem();
  
  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();


  cpu_reset();

 #ifdef CONFIG_DIFFTEST

  
  init_difftest(diff_so_file, img_size, 0);

  #endif
  /* Initialize the simple debugger. */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm());

  /* Display welcome message. */
  
  welcome();
}