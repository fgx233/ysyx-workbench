import chisel3._
import chisel3.util._

class Pc extends Module {
  val io = IO(new Bundle {
    val pc = Output(UInt(32.W))
    val in = Flipped(new WbuToPcPort)
  })

  val pcReg = RegInit("h8000_0000".U(32.W))
  
  when (io.in.wEn) {
    pcReg := io.in.wData
  }

  io.pc := pcReg
}