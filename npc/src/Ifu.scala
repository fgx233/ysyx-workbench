import chisel3._
import chisel3.util._

// 负责根据当前PC从存储器中取出一条指令
class Ifu extends Module {
  // pc寄存器输入
  val pc = IO(Input(UInt(32.W)))
  // 取到的指令
  val inst = IO(Output(UInt(32.W)))

  // 接入ram中的其中一个读端口
  // 读地址
  val raddr = IO(Output(UInt(32.W)))
  // 读数据
  val rdata = IO(Input(UInt(32.W)))
  val ren = IO(Output(Bool()))
  
  ren := true.B
  raddr := pc
  inst := rdata
}