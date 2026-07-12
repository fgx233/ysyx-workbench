#include "common.hpp"
#include "sdb.hpp"
#include "utils.hpp"
#include "paddr.hpp"
#include "cpu.hpp"
#include "monitor.hpp"
#include <getopt.h>


// 日志文件字符串指针
static char *log_file = NULL;
// elf文件字符串指针
static char *elf_file = NULL;
// 差分测试共享库字符串指针
static char *diff_so_file = NULL;
// 程序二进制文件字符串指针
static char *img_file = NULL;
// 差分测试端口
static int difftest_port = 1234;
// 波形文件字符串指针
static char *wave_file = NULL;

//打印初始化信息
static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NPC!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

// 解析命令行参数
static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {"elf"      , required_argument, NULL, 'e'},
    {"wave"     , required_argument, NULL, 'w'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:e:w:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'e': elf_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'w': wave_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-e,--ELF=FILE           read elf from FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\t-w,--wave=FILE.         output wave to FILE\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

// 加载镜像
static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

// 初始化npc监视器
void init_monitor(int argc, char *argv[]) {
  //解析命令行参数
  parse_args(argc, argv);

  // 设置随机数种子
  init_rand();

  // 初始化日志
  init_log(log_file);

  // 初始化仿真环境
  init_context(wave_file);

  // 初始化内存
  init_mem();

  // 初始化cpu状态
  init_isa();

  // 加载镜像
  load_img();

  // 初始化sdb
  init_sdb();

  welcome();
}