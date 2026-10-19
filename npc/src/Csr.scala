import chisel3._
import chisel3.util._

class Csr extends Module {
  val isCsrCtl= IO(Input(Bool()))
  val csrAddr = IO(Input(UInt(12.W)))
  val csrWen  = IO(Input(Bool()))
  val csrWdata= IO(Input(UInt(32.W)))
  val csrRdata= IO(Output(UInt(32.W)))
  val isCsrInv= IO(Output(Bool()))
  val csrIdx  = IO(Output(UInt(12.W)))
  val isEcall = IO(Input(Bool()))
  val isMret  = IO(Input(Bool()))
  val pc      = IO(Input(UInt(32.W)))
  val npc     = IO(Output(UInt(32.W)))

  val mstatus   = RegInit("h1800".U(32.W))
  val mtvec     = RegInit(0.U(32.W))
  val mepc      = RegInit(0.U(32.W))
  val mcause    = RegInit(0.U(32.W))
  val mcycle    = RegInit(0.U(32.W))
  val mcycleh   = RegInit(0.U(32.W))
  val mvendorid = RegInit("h79737978".U(32.W))
  val marchid   = RegInit(26060187.U(32.W))

  val mie  = mstatus(3)
  val mpie  = mstatus(7)

  npc := Mux(isEcall, mtvec, mepc)

  when (isEcall) {
    mstatus := (((mstatus & ("hFFFF_FFF7".U(32.W))) & ("hFFFF_FF7F".U(32.W))) | (mie << 7)) | ("h1800".U(32.W))
  }.elsewhen(isMret) {
    mstatus := (((mstatus & ("hFFFF_FFF7".U(32.W))) | (mpie << 3)) | ("h80".U(32.W))) & ("hFFFF_E7FF".U(32.W))
  }.elsewhen(csrAddr === "h300".U(12.W) && csrWen) {
    mstatus := csrWdata
  }

  when (csrAddr === "h305".U(12.W) && csrWen) {
    mtvec := csrWdata
  }

  when (isEcall) {
    mepc := pc
  } .elsewhen (csrAddr === "h341".U(12.W) && csrWen) {
    mepc := csrWdata
  }

  when (isEcall) {
    mcause := "hB".U(32.W)
  } .elsewhen (csrAddr === "h342".U(12.W) && csrWen) {
    mcause := csrWdata
  }

  when (csrAddr === "hB00".U(12.W) && csrWen) {
    mcycle := csrWdata
  } .otherwise {
    mcycle := mcycle + 1.U(32.W)
  }

  when (csrAddr === "hB80".U(12.W) && csrWen) {
    mcycleh := csrWdata
  } . elsewhen (mcycle === "hFFFFFFFF".U(32.W)){
    mcycleh := mcycleh + 1.U(32.W)
  }

  isCsrInv := !(csrAddr === "h300".U(12.W) || csrAddr === "h305".U(12.W) || csrAddr === "h341".U(12.W) ||
                csrAddr === "h342".U(12.W) || csrAddr === "hB00".U(12.W) || csrAddr === "hB80".U(12.W) ||
                csrAddr === "hF11".U(12.W) || csrAddr === "hF12".U(12.W)) && isCsrCtl
  csrIdx   := csrAddr

  csrRdata := MuxLookup(csrAddr, "h12345678".U(32.W))(Seq(
    "h300".U(12.W) -> mstatus,
    "h305".U(12.W) -> mtvec,
    "h341".U(12.W) -> mepc,
    "h342".U(12.W) -> mcause,
    "hB00".U(12.W) -> mcycle,
    "hB80".U(12.W) -> mcycleh,
    "hF11".U(12.W) -> mvendorid,
    "hF12".U(12.W) -> marchid
  ))
}