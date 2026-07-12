import chisel3._
import chisel3.util.circt.dpi._

object Ebreak extends DPIClockedVoidFunctionImport {
  override val functionName = "ebreak"
  override val inputNames = Some(Seq("pc"))
  final def apply(pc: UInt): Unit = 
    super.call(pc)
}

object Invalid extends DPIClockedVoidFunctionImport {
  override val functionName = "inv"
  override val inputNames = Some(Seq("pc"))
  final def apply(pc: UInt): Unit =
    super.call(pc)
}

class Monitor extends Module {
  val pc = IO(Input(UInt(32.W)))
  val isEbreak = IO(Input(Bool()))
  val isInvalid = IO(Input(Bool()))

  when (isEbreak && !reset.asBool) {
    Ebreak(pc)
  }

  when (isInvalid && !reset.asBool) {
    Invalid(pc)
  }
}