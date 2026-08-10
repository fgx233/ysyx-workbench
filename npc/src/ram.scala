import chisel3._
import chisel3.util.circt.dpi._
import common._

object instFetch extends DPINonVoidFunctionImport[UInt] {
  override val functionName = "vaddr_ifetch"
  override val ret          = UInt(32.W)
  override val clocked      = false
  override val inputNames   = Some(Seq("inst_addr"))
  override val outputName  = Some("inst")
  final def apply(inst_addr: UInt): UInt = super.call(inst_addr)
}

object vaddrRead extends DPINonVoidFunctionImport[UInt] {
  override val functionName = "vaddr_read"
  override val ret          = UInt(32.W)
  override val clocked      = false               // 异步读:组合路径,立即求值
  override val inputNames   = Some(Seq("raddr"))  // 只影响生成 Verilog 的可读性,可省略
  override val outputName   = Some("rdata")
  final def apply(raddr: UInt): UInt = super.call(raddr)
}

object vaddrWrite extends DPIClockedVoidFunctionImport {
  override val functionName = "vaddr_write"
  override val inputNames   = Some(Seq("waddr", "wdata", "wmask"))
  final def apply(waddr: UInt, wdata: UInt, wmask: UInt): Unit =
    super.call(waddr, wdata, wmask)
}


class Ram extends Module {
  val raddr1 = IO(Input(UInt(32.W)))
  val rdata1 = IO(Output(UInt(32.W)))
  val ren1   = IO(Input(Bool()))
  val raddr2 = IO(Input(UInt(32.W)))
  val rdata2 = IO(Output(UInt(32.W)))
  val ren2   = IO(Input(Bool()))
  val waddr  = IO(Input(UInt(32.W)))
  val wdata  = IO(Input(UInt(32.W)))
  val wen    = IO(Input(Bool()))
  val wmask  = IO(Input(UInt(4.W)))

  when(ren1 && !reset.asBool) {
    rdata1 := instFetch(raddr1)        // 调用点 1:取指
  } otherwise {
    rdata1 := 0.U
  }

  when(ren2 && !reset.asBool) {
    rdata2 := vaddrRead(raddr2)
  } otherwise {
    rdata2 := 0.U
  }

  when(wen && !reset.asBool) {
    vaddrWrite(waddr, wdata, wmask.pad(8))
  }
}