import chisel3._
import chisel3.util._
import common._

class Regs extends Module {
  val raddr1  = IO(Input(UInt(4.W)))
  val rdata1  = IO(Output(UInt(32.W)))
  val raddr2  = IO(Input(UInt(4.W)))
  val rdata2  = IO(Output(UInt(32.W)))
  val waddr   = IO(Input(UInt(4.W)))
  val wdata   = IO(Input(UInt(32.W)))
  val wen     = IO(Input(Bool()))

  val regs = RegInit(VecInit(Seq.fill(16)(0.U(32.W))))      //  寄存器堆

  when (wen && waddr =/= 0.U) {                 //  寄存器更新逻辑
    regs(waddr) := wdata
  }

  rdata1 := Mux(raddr1 === 0.U, 0.U, regs(raddr1))          //  rs1
  rdata2 := Mux(raddr2 === 0.U, 0.U, regs(raddr2))          //  rs2
}