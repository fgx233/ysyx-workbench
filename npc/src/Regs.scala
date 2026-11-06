import chisel3._
import chisel3.util._

class Regs extends Module {
  val io = IO(new Bundle {
    val in = Flipped(new WbuToRegsPort)
    val out = new RegsToIduPort
  })

  val regs = RegInit(VecInit(Seq.fill(16)(0.U(32.W))))

  io.out.src1 := regs(io.out.rAddr1)
  io.out.src2 := regs(io.out.rAddr2)

  when (io.in.wEn && io.in.wAddr =/= 0.U(4.W)) {
    regs(io.in.wAddr) := io.in.wData
  }
}