import chisel3._
import chisel3.util._
import chisel3.util.circt.dpi.RawClockedVoidFunctionCall

class Monitor extends Module {
  val io = IO(new Bundle {
    val idu = Flipped(new IduToMonitorPort)
    val commit = Input(Bool())
  })

  RawClockedVoidFunctionCall(
    "ebreak",
    Some(Seq("pc"))
    )(clock, io.idu.isEbreak, io.idu.pc)
  RawClockedVoidFunctionCall(
    "inv",
    Some(Seq("pc"))
    )(clock, io.idu.isInvalid, io.idu.pc)
  RawClockedVoidFunctionCall("inst_commit")(clock, io.commit)
}