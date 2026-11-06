import chisel3._
import chisel3.util._

class simpleBus extends Bundle {
  val reqValid   = Output(Bool())
  val reqReady   = Input(Bool())
  val addr       = Output(UInt(32.W))
  val wEn        = Output(Bool())
  val wData      = Output(UInt(32.W))
  val wMask      = Output(UInt(4.W))
  val respValid  = Input(Bool())
  val respReady  = Output(Bool())
  val rData      = Input(UInt(32.W))
}

class IfuToIduPort extends Bundle {
  val inst          = Output(UInt(32.W))
}

class IduToMonitorPort extends Bundle {
  val pc            = Output(UInt(32.W))
  val isEbreak      = Output(Bool())
  val isInvalid     = Output(Bool())
}

class IduToExuPort extends ExuToLsuPort {
  val exuAction     = Output(ExuAction())
  val aluOption     = Output(AluOption())
  val aluOprand1    = Output(UInt(32.W))
  val aluOprand2    = Output(UInt(32.W))
  val pcOprand1     = Output(UInt(32.W))
  val pcOprand2     = Output(UInt(32.W))
}

class ExuToLsuPort extends LsuToWbuPort {
  val memAddr       = Output(UInt(32.W))
  val memRdEn       = Output(Bool())
  val memWrEn       = Output(Bool())
  val memWrData     = Output(UInt(32.W))
  val memLen        = Output(LsuByteLen())
  val memSign       = Output(LsuSign())
}

class LsuToWbuPort extends Bundle {
  val regsWaddr     = Output(UInt(4.W))
  val regsWdata     = Output(UInt(32.W))
  val regsWen       = Output(Bool())

  val pcWdata       = Output(UInt(32.W))
  val pcWen         = Output(Bool())

  val csrWaddr      = Output(UInt(12.W))
  val csrWdata      = Output(UInt(32.W))
  val csrWen        = Output(Bool())

  val isEcall       = Output(Bool())
  val isMret        = Output(Bool())
}


class WbuToIfuPort extends Bundle {
}

class WbuToPcPort extends Bundle {
  val wData         = Output(UInt(32.W))
  val wEn           = Output(Bool())
}

class WbuToRegsPort extends Bundle {
  val wData         = Output(UInt(32.W))
  val wAddr         = Output(UInt(4.W))
  val wEn           = Output(Bool())
}

class WbuToCsrsPort extends Bundle {
  val wData         = Output(UInt(32.W))
  val wAddr         = Output(UInt(12.W))
  val wEn           = Output(Bool())
  val isEcall       = Output(Bool())
  val isMret        = Output(Bool())  
}

class RegsToIduPort extends Bundle {
  val rAddr1        = Input(UInt(4.W))
  val rAddr2        = Input(UInt(4.W))
  val src1          = Output(UInt(32.W))
  val src2          = Output(UInt(32.W))
}

class CsrsToIduPort extends Bundle {
  val mtvec         = Output(UInt(32.W))
  val mepc          = Output(UInt(32.W))
  val rAddr         = Input(UInt(12.W))
  val rData         = Output(UInt(32.W))
}

object LsuByteLen extends ChiselEnum {
  val one, two, four = Value
}

object LsuSign extends ChiselEnum {
  val signed, unsigned = Value
}