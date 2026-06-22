import chisel3._
import chisel3.util._

class regs extends Module {
  val raddr1 = IO(Input(UInt(2.W)))     //  rs1 地址
  val rdata1 = IO(Output(UInt(8.W)))    //  读rs1数据

  val raddr2 = IO(Input(UInt(2.W)))     //  rs2 地址
  val rdata2 = IO(Output(UInt(8.W)))    //  读rs2数据

  val waddr  = IO(Input(UInt(2.W)))     //  rd 地址
  val wdata  = IO(Input(UInt(8.W)))     //  写回数据
  val wen    = IO(Input(Bool()))        //  写使能


  val regs = RegInit(VecInit(Seq.fill(4)(0.U(8.W))))     //  寄存器堆

  when (wen) {                          //  寄存器更新逻辑
    regs(waddr) := wdata
  }

  rdata1 := regs(raddr1)                //  rs1
  rdata2 := regs(raddr2)                //  rs2
}