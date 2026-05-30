#include <common.h>

extern uint64_t g_nr_guest_inst;


FILE *log_fp = NULL;

void init_log(const char *log_file) {
  //stdout是标准输出，即如果用户没有明确指定日志输出的目标文件，那么会打印到terminal上。
  //printf("%s**********!!!!!!!",log_file);
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

//动态控制日志或最终功能的开启/关闭。
//返回值：true 表示启用日志/追踪，false 表示禁用。
//MUXDEF是一个条件选择宏，类似三木运算符
//MUXDEF(条件, 值1, 值2) // 若条件为真，返回值1；否则返回值2
bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}

