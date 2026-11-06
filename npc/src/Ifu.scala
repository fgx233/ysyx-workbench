import chisel3._
import chisel3.util._

/*
功能：从ram取出指令，送给IDU。
状态：
1、等待Wbu的完成信号，
2、等待reqValid，（随机延迟）
3、reqValid，
4、等待respReady,(随机延迟)
5、respReady
6、valid
*/

object Ifu {
  object State extends ChiselEnum {
    val sReady, sReqValid, sRespReady, sValid = Value
  }
}

class Ifu extends Module {
  import Ifu.State._

  val io = IO(new Bundle {
    val pc            = Input(UInt(32.W))
    val ifetch        = new simpleBus
    val in            = Flipped(Decoupled(new WbuToIfuPort))
    val out           = Decoupled(new IfuToIduPort)
  })    

  val state           = RegInit(sReqValid)
  val rData           = Reg(UInt(32.W))

  io.ifetch.reqValid  := (state === sReqValid)
  io.ifetch.addr      := io.pc
  io.ifetch.wEn       := false.B
  io.ifetch.wData     := 0.U(32.W)
  io.ifetch.wMask     := 0.U(4.W)
  io.ifetch.respReady := (state === sRespReady)

  io.in.ready         := (state === sReady)
  io.out.valid        := (state === sValid)
  io.out.bits.inst    := rData

  when (io.ifetch.respValid && io.ifetch.respReady) {
    rData             := io.ifetch.rData
  }

  switch (state) {
    is (sReady) {
      when (io.in.valid && io.in.ready) {
        state := sReqValid
      }
    }
    is (sReqValid) {
      when (io.ifetch.reqValid && io.ifetch.reqReady) {
        state := sRespReady
      }
    }
    is (sRespReady) {
      when (io.ifetch.respValid && io.ifetch.respReady) {
        state := sValid
      }  
    }
    is (sValid) {
      when (io.out.valid && io.out.ready) {
        state := sReady
      }
    }
  }
}