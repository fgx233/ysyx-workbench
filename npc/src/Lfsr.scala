import chisel3._
import chisel3.util._

class Lfsr(seed: Int) extends Module {
  val io = IO(new Bundle {
    val en = Input(Bool())
    val out = Output(UInt(4.W))
  })

  val reg = RegInit(seed.U(4.W))
  when (io.en) {
    reg := Cat(reg(0) ^ reg(1), reg(3,1))
  }
  
  io.out := reg
}