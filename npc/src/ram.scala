import chisel3._
import chisel3.util.circt.dpi._

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
  // 取指读口 (IFU)
  val fetch_en = IO(Input(Bool()))
  val pc       = IO(Input(UInt(32.W)))
  val inst     = IO(Output(UInt(32.W)))
  // 数据读口 (LSU load)
  val ren      = IO(Input(Bool()))
  val raddr    = IO(Input(UInt(32.W)))
  val rdata    = IO(Output(UInt(32.W)))
  // 写口 (LSU store)
  val wen      = IO(Input(Bool()))
  val waddr    = IO(Input(UInt(32.W)))
  val wdata    = IO(Input(UInt(32.W)))
  val wmask    = IO(Input(UInt(4.W)))

  when(fetch_en && !reset.asBool) {
    inst := instFetch(pc)        // 调用点 1:取指
  } otherwise {
    inst := 0.U
  }

  when(ren && !reset.asBool) {
    rdata := vaddrRead(raddr)    // 调用点 2:load
  } otherwise {
    rdata := 0.U
  }

  when(wen && !reset.asBool) {
    vaddrWrite(waddr, wdata, wmask.pad(8))
  }
}