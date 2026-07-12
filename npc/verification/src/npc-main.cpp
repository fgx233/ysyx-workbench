#include "common.hpp"
#include "utils.hpp"
#include "monitor.hpp"
#include "sdb.hpp"
#include "cpu.hpp"

int main(int argc, char** argv) {
  // 初始化npc环境
  init_monitor(argc, argv);

  //启动sdb

  sdb_mainloop();
  close_context();
  return is_exit_status_bad();
}