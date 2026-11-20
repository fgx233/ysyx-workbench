package elaborate
import chisel3._
import chisel3.util._
import chisel3.util.circt.dpi._


object Ram {
  object readState extends ChiselEnum {
    val sReadAddrReady, sRead, sReadRespValid = Value
  }
  object writeState extends ChiselEnum {
    val sWriteAddrAndDataReady, sWriteAddrReady, sWriteDataReady, sWrite, sWriteRespValid = Value
  }
}

class Ram extends Module {
  import Ram.readState._
  import Ram.writeState._
  import RESP._

  val io = IO(Flipped(new axi4Lite))

  // 读逻辑
  val rState = RegInit(sReadAddrReady)
  val rAddr  = Reg(UInt(32.W))

  io.AR.arReady := rState === sReadAddrReady
  io.R.rValid := rState === sReadRespValid

  when (io.AR.arValid && io.AR.arReady) {
    rAddr := io.AR.arAddr
  }

  io.R.rResp := OKAY
  io.R.rData := RawClockedNonVoidFunctionCall(
    "vaddr_read",
    UInt(32.W),
    Some(Seq("raddr")),
    Some("rdata")
  )(clock, rState === sRead, rAddr)

  switch (rState) {
    is (sReadAddrReady) {
      when (io.AR.arValid && io.AR.arReady) {
        rState := sRead
      }
    }
    is (sRead) {
      rState := sReadRespValid
    }
    is (sReadRespValid) {
      when (io.R.rValid && io.R.rReady) {
        rState := sReadAddrReady
      }
    }
  }

  // 写逻辑
  val wState = RegInit(sWriteAddrAndDataReady)
  val wAddr = Reg(UInt(32.W))
  val wData = Reg(UInt(32.W))
  val wMask = Reg(UInt(4.W))

  io.AW.awReady := wState === sWriteAddrAndDataReady || wState === sWriteAddrReady
  io.W.wReady := wState === sWriteAddrAndDataReady || wState === sWriteDataReady
  io.B.bValid := wState === sWriteRespValid

  when (io.AW.awValid && io.AW.awReady) {
    wAddr := io.AW.awAddr
  }
  when (io.W.wValid && io.W.wReady) {
    wData := io.W.wData
    wMask := io.W.wStrb
  }

  RawClockedVoidFunctionCall(
    "vaddr_write",
    Some(Seq("waddr", "wdata", "wmask"))
  )(clock, wState === sWrite, wAddr, wData, wMask.pad(8))

  io.B.bResp := OKAY

  switch (wState) {
    is (sWriteAddrAndDataReady) {
      when (io.AW.awValid && io.AW.awReady && io.W.wValid && io.W.wReady) {
        wState := sWrite
      }.elsewhen (io.AW.awValid && io.AW.awReady) {
        wState := sWriteDataReady
      }.elsewhen (io.W.wValid && io.W.wReady) {
        wState := sWriteAddrReady
      }
    }
    is (sWriteAddrReady) {
      when (io.AW.awValid && io.AW.awReady) {
        wState := sWrite
      }
    }
    is (sWriteDataReady) {
      when (io.W.wValid && io.W.wReady) {
        wState := sWrite
      }
    }
    is (sWrite) {
      wState := sWriteRespValid
    }
    is (sWriteRespValid) {
      when (io.B.bValid && io.B.bReady) {
        wState := sWriteAddrAndDataReady
      }
    }
  }
}