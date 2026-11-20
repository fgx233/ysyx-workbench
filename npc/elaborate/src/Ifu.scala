package elaborate
import chisel3._
import chisel3.util._
import os.stat

/*
功能：从ram取出指令，送给IDU。
状态：
1、等待WBU就绪
2、发出读请求
3、等待读响应
4、等待IDU就绪
*/

object Ifu {
  object State extends ChiselEnum {
    val sReady, sReqValid, sRespReady, sValid = Value
  }
}

class Ifu extends Module {
  import Ifu.State._
  import RESP._

  val io = IO(new Bundle {
    val pc            = Input(UInt(32.W))
    val ifetch        = new axi4Lite
    val in            = Flipped(Decoupled(new WbuToIfuPort))
    val out           = Decoupled(new IfuToIduPort)
  })    

  val state           = RegInit(sReqValid)
  val rData           = Reg(UInt(32.W))
  val rResp           = RegInit(OKAY)

  io.ifetch.AR.arValid := (state === sReqValid)
  io.ifetch.AR.arAddr  := io.pc

  when (io.ifetch.R.rValid && io.ifetch.R.rReady) {
    rData := io.ifetch.R.rData
    rResp := io.ifetch.R.rResp
  }
  io.ifetch.R.rReady := (state === sRespReady)

  io.in.ready := (state === sReady)
  io.out.valid := (state === sValid)
  io.out.bits.inst := rData
  
  switch (state) {
    is (sReady) {
      when (io.in.valid && io.in.ready) {
        state := sReqValid
      }
    }
    is (sReqValid) {
      when (io.ifetch.AR.arValid && io.ifetch.AR.arReady) {
        state := sRespReady
      }
    }
    is (sRespReady) {
      when (io.ifetch.R.rValid && io.ifetch.R.rReady) {
        state := sValid
      }
    }
    is (sValid) {
      when (io.out.valid && io.out.ready) {
        state := sReady
      }
    }
  }

  if (DEBUG.debug) {
    assert(rResp === OKAY, "IFU get inst fail,rResp = %b", rResp)
  }

  io.ifetch.AW.awAddr := 0.U(32.W)
  io.ifetch.AW.awValid := false.B

  io.ifetch.W.wData := 0.U(32.W)
  io.ifetch.W.wStrb := 0.U(4.W)
  io.ifetch.W.wValid := false.B
  
  io.ifetch.B.bReady := false.B
}