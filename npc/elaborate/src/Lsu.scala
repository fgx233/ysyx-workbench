package elaborate
import chisel3._
import chisel3.util._
import chisel3.experimental.dataview.DataViewable


/* 
功能：将Exu传入的信息进一步处理，若有读写任务，则进行读写任务，若无读写任务，直接传递信息到Wbu。
状态：
1、等待EXU valid
2、接收EXU传递的信息
3、向ram发出读写请求
4、接收ram的回应
5、等待WBU的ready
 */

object Lsu {
  object State extends ChiselEnum {
    val sLsuReady, sJudge,
        sReadAddrValid, sReadDataReady,
        sWriteAddrAndDataValid, sWaitAddrReady, sWaitDataReady, sWriteRespReady,
        sLsuValid = Value
  }
}

class Lsu extends Module {
  import Lsu.State._
  import LsuByteLen._
  import LsuSign._
  import RESP._

  val io = IO(new Bundle {
    val in = Flipped(Decoupled(new ExuToLsuPort))
    val out= Decoupled(new LsuToWbuPort)
    val ram= new axi4Lite 
  })

  val state = RegInit(sLsuReady)

  val memAddr   = Reg(UInt(32.W))
  val memRdEn   = Reg(Bool())
  val memWrEn   = Reg(Bool())
  val memWrData = Reg(UInt(32.W))
  val memLen    = Reg(UInt(2.W))
  val memSign   = Reg(Bool())

  val regsWaddr = Reg(UInt(4.W))
  val regsWdata = Reg(UInt(32.W))
  val regsWen   = Reg(Bool())

  val pcWdata   = Reg(UInt(32.W))
  val pcWen     = Reg(Bool())

  val csrWaddr  = Reg(UInt(12.W))
  val csrWdata  = Reg(UInt(32.W))
  val csrWen    = Reg(Bool())

  val isEcall   = Reg(Bool())
  val isMret    = Reg(Bool())

  val rData     = Reg(UInt(32.W))
  val rResp     = RegInit(OKAY)
  val bResp     = RegInit(OKAY)

  io.in.ready := state === sLsuReady
  io.out.valid := state === sLsuValid

  // AR
  io.ram.AR.arAddr := memAddr(31,2) ## "b00".U(2.W)
  io.ram.AR.arValid := state === sReadAddrValid
  // R
  when (io.ram.R.rValid && io.ram.R.rReady) {
    rData := io.ram.R.rData
    rResp := io.ram.R.rResp
  }
  io.ram.R.rReady := state === sReadDataReady
  // AW
  io.ram.AW.awAddr := memAddr(31,2) ## "b00".U(2.W)
  io.ram.AW.awValid := state === sWriteAddrAndDataValid || state === sWaitAddrReady
  // W
  when (memLen === four) {
    io.ram.W.wData := memWrData
  }.elsewhen (memLen === two) {
    io.ram.W.wData := Fill(2, memWrData(15,0))
  }.otherwise {
    io.ram.W.wData := Fill(4, memWrData(7,0))
  }
  when (memLen === four) {
    io.ram.W.wStrb := "b1111".U(4.W)
  }.elsewhen (memLen === two) {
    io.ram.W.wStrb := Mux(memAddr(1,0) === "b00".U(2.W), "b0011".U(4.W), "b1100".U(4.W))
  }.otherwise {
    io.ram.W.wStrb := MuxLookup(memAddr(1,0), "b0001".U(4.W))(Seq(
      "b00".U(2.W) -> "b0001".U(4.W),
      "b01".U(2.W) -> "b0010".U(4.W),
      "b10".U(2.W) -> "b0100".U(4.W),
      "b11".U(2.W) -> "b1000".U(4.W)
    ))
  }
  io.ram.W.wValid := state === sWriteAddrAndDataValid || state === sWaitDataReady
  // B
  when (io.ram.B.bValid && io.ram.B.bReady) {
    bResp := io.ram.B.bResp
  }
  io.ram.B.bReady := state === sWriteRespReady

  when (io.in.valid && io.in.ready) {
    memAddr   := io.in.bits.memAddr  
    memRdEn   := io.in.bits.memRdEn  
    memWrEn   := io.in.bits.memWrEn  
    memWrData := io.in.bits.memWrData
    memLen    := io.in.bits.memLen   
    memSign   := io.in.bits.memSign

    regsWaddr := io.in.bits.regsWaddr
    regsWdata := io.in.bits.regsWdata
    regsWen   := io.in.bits.regsWen  

    pcWdata   := io.in.bits.pcWdata  
    pcWen     := io.in.bits.pcWen    

    csrWaddr  := io.in.bits.csrWaddr 
    csrWdata  := io.in.bits.csrWdata 
    csrWen    := io.in.bits.csrWen   

    isEcall   := io.in.bits.isEcall  
    isMret    := io.in.bits.isMret   
  }

    io.out.bits.regsWaddr := regsWaddr
    io.out.bits.regsWdata := regsWdata
    when (memRdEn) {
      when (memLen === four) {
        io.out.bits.regsWdata := rData
      }.elsewhen (memLen === two) {
        when (memSign === unsigned) {
          io.out.bits.regsWdata := Mux(memAddr(1,0) === "b00".U(2.W), rData(15,0).pad(32), rData(31,16).pad(32))
        }.otherwise {
          io.out.bits.regsWdata := Mux(memAddr(1,0) === "b00".U(2.W), rData(15,0).asSInt.pad(32).asUInt, rData(31,16).asSInt.pad(32).asUInt)
        }
      }.otherwise {
        when (memSign === unsigned) {
          io.out.bits.regsWdata := MuxLookup(memAddr(1,0), rData(7,0).pad(32))(Seq(
            "b00".U(2.W)  -> rData(7,0).pad(32),
            "b01".U(2.W)  -> rData(15,8).pad(32),
            "b10".U(2.W)  -> rData(23,16).pad(32),
            "b11".U(2.W)  -> rData(31,24).pad(32)
          ))
        }.otherwise {
          io.out.bits.regsWdata := MuxLookup(memAddr(1,0), rData(7,0).pad(32))(Seq(
            "b00".U(2.W)  -> rData(7,0).asSInt.pad(32).asUInt,
            "b01".U(2.W)  -> rData(15,8).asSInt.pad(32).asUInt,
            "b10".U(2.W)  -> rData(23,16).asSInt.pad(32).asUInt,
            "b11".U(2.W)  -> rData(31,24).asSInt.pad(32).asUInt
          ))
        }
      }
    }.otherwise {
      io.out.bits.regsWdata := regsWdata
    }
    io.out.bits.regsWen   := regsWen  

    io.out.bits.pcWdata   := pcWdata  
    io.out.bits.pcWen     := pcWen    

    io.out.bits.csrWaddr  := csrWaddr 
    io.out.bits.csrWdata  := csrWdata 
    io.out.bits.csrWen    := csrWen   

    io.out.bits.isEcall   := isEcall  
    io.out.bits.isMret    := isMret   
  
  switch (state) {
    is (sLsuReady) {
      when (io.in.valid && io.in.ready) {
        state := sJudge
      }
    }
    is (sJudge) {
      when (memRdEn) {
        state := sReadAddrValid
      }.elsewhen (memWrEn) {
        state := sWriteAddrAndDataValid
      }.otherwise {
        state := sLsuValid
      }
    }
    is (sReadAddrValid) {
      when (io.ram.AR.arValid && io.ram.AR.arReady) {
        state := sReadDataReady
      }
    }
    is (sReadDataReady) {
      when (io.ram.R.rValid && io.ram.R.rReady) {
        state := sLsuValid
      }
    }
    is (sWriteAddrAndDataValid) {
      when (io.ram.AW.awValid && io.ram.AW.awReady && io.ram.W.wValid && io.ram.W.wReady) {
        state := sWriteRespReady
      }.elsewhen (io.ram.AW.awValid && io.ram.AW.awReady) {
        state := sWaitDataReady
      }.elsewhen (io.ram.W.wValid && io.ram.W.wReady) {
        state := sWaitAddrReady
      }
    }
    is (sWaitDataReady) {
      when (io.ram.W.wValid && io.ram.W.wReady) {
        state := sWriteRespReady
      }
    }
    is (sWaitAddrReady) {
      when (io.ram.AW.awValid && io.ram.AW.awReady) {
        state := sWriteRespReady
      }
    }
    is (sWriteRespReady) {
      when (io.ram.B.bValid && io.ram.B.bReady) {
        state := sLsuValid
      }
    }
    is (sLsuValid) {
      when (io.out.valid && io.out.ready) {
        state := sLsuReady
      }
    }
  }

  if (DEBUG.debug) {
    // 检测AXI读写时是否出现错误
    assert(rResp === OKAY, "rResp = %b", rResp)
    assert(bResp === OKAY, "bResp = %b", bResp)
    // 检测是否出现不支持的访存行为
    assert(!((memAddr(1,0) === "b01".U(2.W) || memAddr(1,0) === "b11".U(2.W)) && memLen === two && (memRdEn || memWrEn)), "lh/lhu/sh inst has unsupport behavior")
    assert(!((memRdEn || memWrEn) && memLen === four && memAddr(1,0) =/= "b00".U(2.W)), "lw/sw inst has unsupport behavior")
  }
  
}