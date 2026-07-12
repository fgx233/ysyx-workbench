import chisel3._
import chisel3.util._

class Exu extends Module {
  val pc   = IO(Input(UInt(32.W)))
  val src1 = IO(Input(UInt(32.W)))
  val src2 = IO(Input(UInt(32.W)))
  val immI = IO(Input(UInt(32.W)))
  val immU = IO(Input(UInt(32.W)))
  val immS = IO(Input(UInt(32.W)))

  val isAdd        = IO(Input(Bool()))
  val isAddi       = IO(Input(Bool()))
  val isLw         = IO(Input(Bool()))
  val isAuipc      = IO(Input(Bool()))
  val isLbu        = IO(Input(Bool()))
  val isJalr       = IO(Input(Bool()))
  val isLui        = IO(Input(Bool()))
  val isSb         = IO(Input(Bool()))
  val isSw         = IO(Input(Bool()))

  val ramWriteAddr = IO(Output(UInt(32.W)))
  val ramWriteData = IO(Output(UInt(32.W)))

  val ramReadAdder = IO(Output(UInt(32.W)))
  val ramReadData  = IO(Input(UInt(32.W)))

  val regsWriteData = IO(Output(UInt(32.W)))

  val nextPc       = IO(Output(UInt(32.W)))

  val snpc = pc + 4.U
  

  val imm = MuxCase(immI, Seq(
    isLui -> immU,
    isSb -> immS,
    isSw -> immS,
    isAuipc -> immU
  ))
  val add1 = Mux(isAuipc, pc, src1)
  val add2 = Mux(isAdd, src2, imm)
  val adder = add1 + add2

  ramWriteAddr := adder
  ramWriteData := src2

  ramReadAdder := adder

  regsWriteData := MuxCase(adder, Seq(
    isAdd  -> adder,
    isAddi -> adder,
    isAuipc -> adder,
    isJalr -> snpc,
    isLui  -> immU,
    isLw   -> ramReadData,
    isLbu  -> ramReadData
  ))

  val dnpc = Mux(isJalr, adder & (~1.U(32.W)), snpc)
  nextPc := dnpc
}
