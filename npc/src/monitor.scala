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

object CsrInv extends DPIClockedVoidFunctionImport {
  override val functionName = "csr_inv"
  override val inputNames = Some(Seq("pc", "idx"))
  final def apply(pc: UInt, idx: UInt): Unit =
    super.call(pc, idx)
}

class Monitor extends Module {
  val pc = IO(Input(UInt(32.W)))
  val isEbreak = IO(Input(Bool()))
  val isInvalid = IO(Input(Bool()))
  val isCsrInv = IO(Input(Bool()))
  val csrIdx = IO(Input(UInt(12.W)))

  when (isEbreak && !reset.asBool) {
    Ebreak(pc)
  }

  when (isInvalid && !reset.asBool) {
    Invalid(pc)
  }

  when (isCsrInv && !reset.asBool) {
    CsrInv(pc, csrIdx.pad(32))
  }

}