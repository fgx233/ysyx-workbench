import chisel3._
import chisel3.util._
import chisel3.util.experimental._

class rom extends Module {
  val inst_addr   = IO(Input(UInt(4.W)))          //指令地址输入
  val inst        = IO(Output(UInt(8.W)))         //指令输出

  val mem = Mem(16, UInt(8.W))                    //存储器本体
  loadMemoryFromFileInline(mem, "/home/fgx/projects/ysyx-workbench/npc/nvboard/resource/inst.hex")

  inst := mem(inst_addr)                          //指令获取
}