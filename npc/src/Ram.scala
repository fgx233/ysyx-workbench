import chisel3._
import chisel3.util._
import chisel3.util.circt.dpi._

object Ram {
  object State extends ChiselEnum {
    val sReqReady, sReadOrWrite, sRespValid = Value
  }
}

class Ram(readFunc: String) extends Module {
  import Ram.State._

  val io = IO(Flipped(new simpleBus))

  val cnt   = RegInit(1.U(4.W))
  val state = RegInit(sReqReady)
  val addr  = Reg(UInt(32.W))
  val wEn   = Reg(Bool())
  val wData = Reg(UInt(32.W))
  val wMask = Reg(UInt(4.W))

  switch (state) {
    is (sReqReady) {
      when (io.reqValid && io.reqReady) {
        state := sReadOrWrite
      }
    }
    is (sReadOrWrite) {
      state := sRespValid
    }
    is (sRespValid) {
      when (io.respValid && io.respReady) {
        state := sReqReady
      }
    }
  }

  io.reqReady := Mux(state === sReqReady, true.B, false.B)
  io.respValid := Mux(state === sRespValid, true.B, false.B)

  when (io.reqValid && io.reqReady) {
    addr := io.addr
    wEn := io.wEn
    wData := io.wData
    wMask := io.wMask
  }


  RawClockedVoidFunctionCall(
    "vaddr_write",
    Some(Seq("waddr", "wdata", "wmask"))
  )(clock, state === sReadOrWrite && wEn, addr, wData, wMask.pad(8))

  io.rData := RawClockedNonVoidFunctionCall(
    readFunc,
    UInt(32.W),
    Some(Seq("inst_addr")),
    Some("inst")
  )(clock, state === sReadOrWrite && !wEn, addr)
    
  
}