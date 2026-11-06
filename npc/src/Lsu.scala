import chisel3._
import chisel3.util._
import os.stat

/* 
功能：将Exu传入的信息进一步处理，若有读写任务，则进行读写任务，若无读写任务，直接传递信息到Wbu。
 */

object Lsu {
  object State extends ChiselEnum {
    val sLsuReady, sJudge,
        sReadReqValid, sReadRespReady, 
        sWriteReqValid, sWriteRespReady, 
        sReadReqValid1, sReadRespReady1, sReadReqValid2, sReadRespReady2, 
        sWriteReqValid1, sWriteRespReady1, sWriteReqValid2, sWriteRespReady2, 
        sLsuValid = Value
  }
}

class Lsu extends Module {
  import Lsu.State._
  import LsuByteLen._
  import LsuSign._

  val io = IO(new Bundle {
    val in = Flipped(Decoupled(new ExuToLsuPort))
    val out= Decoupled(new LsuToWbuPort)
    val ram= new simpleBus 
  })

  val state = RegInit(sLsuReady)

  val memAddr   = Reg(UInt(32.W))
  val memRdEn   = Reg(Bool())
  val memWrEn   = Reg(Bool())
  val memWrData = Reg(UInt(32.W))
  val memLen    = Reg(LsuByteLen())
  val memSign   = Reg(LsuSign())

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

  val rData1    = Reg(UInt(32.W))
  val rData2    = Reg(UInt(32.W))

  //reqValid
  io.ram.reqValid := (state === sReadReqValid) || (state === sWriteReqValid) ||
                     (state === sReadReqValid1) || (state === sWriteReqValid1) ||
                     (state === sReadReqValid2) || (state === sWriteReqValid2) 
  //addr
  val len         = MuxLookup(memLen, 0.U(32.W))(Seq(
    one           -> 1.U(32.W),
    two           -> 2.U(32.W),
    four          -> 4.U(32.W)
  ))
  val lowAddr   = memAddr & 0b11.U(32.W)
  val oneStep   = ((lowAddr + len) <= 4.U(32.W))
  when (oneStep) {
    io.ram.addr := memAddr(31,2) ## 0.U(2.W)
  }.elsewhen (state === sReadReqValid1 || state === sWriteReqValid1) {
    io.ram.addr := memAddr(31,2) ## 0.U(2.W)
  }.otherwise {
    io.ram.addr := (memAddr(31,2) ## 0.U(2.W)) + 4.U(32.W)
  }
  //wEn
  io.ram.wEn      := memWrEn
  //wData
  when (memLen === one) {
    io.ram.wData := Fill(4, memWrData(7,0))
  }.elsewhen (memLen === two) {
    when (oneStep) {
      when (lowAddr === 0.U(32.W) || lowAddr === 2.U(32.W)) {
        io.ram.wData := Fill(2, memWrData(15,0))
      }.otherwise {
        io.ram.wData := 0.U(8.W) ## memWrData(15,0) ## 0.U(8.W)
      }
    }.otherwise {
      when (state === sWriteReqValid1) {
        io.ram.wData := memWrData(7,0) ## 0.U(24.W)
      }.otherwise {
        io.ram.wData := 0.U(24.W) ## memWrData(15,8)
      }
    }
  }.otherwise {
    when (oneStep) {
      io.ram.wData := memWrData
    }.otherwise {
      when (state === sWriteReqValid1) {
        when (lowAddr === 1.U(32.W)) {
          io.ram.wData := memWrData(23,0) ## 0.U(8.W)
        }.elsewhen (lowAddr === 2.U(32.W)) {
          io.ram.wData := memWrData(15,0) ## 0.U(16.W)
        }.otherwise {
          io.ram.wData := memWrData(7,0) ## 0.U(24.W)
        }
      }.otherwise {
        when (lowAddr === 1.U(32.W)) {
          io.ram.wData := 0.U(24.W) ## memWrData(31,24)
        }.elsewhen (lowAddr === 2.U(32.W)) {
          io.ram.wData := 0.U(16.W) ## memWrData(31,16)
        }.otherwise {
          io.ram.wData := 0.U(8.W) ## memWrData(31,8)
        }
      }
    }
  }
  //wMask
  when (memLen === one) {
    io.ram.wMask := MuxLookup(lowAddr, 0.U(4.W))(Seq(
      0.U(32.W)  -> 0b0001.U(4.W),
      1.U(32.W)  -> 0b0010.U(4.W),
      2.U(32.W)  -> 0b0100.U(4.W),
      3.U(32.W)  -> 0b1000.U(4.W)
    ))
  }.elsewhen (memLen === two) {
    when (oneStep) {
      io.ram.wMask := MuxLookup(lowAddr, 0.U(4.W))(Seq(
        0.U(32.W)  -> 0b0011.U(4.W),
        1.U(32.W)  -> 0b0110.U(4.W),
        2.U(32.W)  -> 0b1100.U(4.W)
      ))
    }.otherwise {
      io.ram.wMask := Mux(state === sWriteReqValid1, 0b1000.U(4.W), 0b0001.U(4.W))
    }
  }.otherwise {
    when (oneStep) {
      io.ram.wMask := 0b1111.U(4.W)
    }.otherwise {
      when (state === sWriteReqValid1) {
        io.ram.wMask := MuxLookup(lowAddr, 0.U(4.W))(Seq(
          1.U(32.W)  -> 0b1110.U(4.W),
          2.U(32.W)  -> 0b1100.U(4.W),
          3.U(32.W)  -> 0b1000.U(4.W)
        ))
      }.otherwise {
        io.ram.wMask := MuxLookup(lowAddr, 0.U(4.W))(Seq(
          1.U(32.W)  -> 0b0001.U(4.W),
          2.U(32.W)  -> 0b0011.U(4.W),
          3.U(32.W)  -> 0b0111.U(4.W)
        ))
      }
    }
  }
  //respReady
  io.ram.respReady := (state === sReadRespReady || state === sWriteRespReady ||
                       state === sReadRespReady1 || state === sWriteRespReady1 ||
                       state === sReadRespReady2 || state === sWriteRespReady2)
  //rData
  when ((state === sReadRespReady || state === sReadRespReady1) && (io.ram.respValid && io.ram.respReady)) {
    rData1 := io.ram.rData
  }
  when ((state === sReadRespReady2) && (io.ram.respValid && io.ram.respReady)) {
    rData2 := io.ram.rData
  }

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

  when (memRdEn) {
    when (oneStep) {
      when (memLen === one) {
        when (memSign === signed) {
          io.out.bits.regsWdata := MuxLookup(lowAddr, 0.U(32.W))(Seq(
            0.U(32.W)           -> rData1(7,0).asSInt.pad(32).asUInt,
            1.U(32.W)           -> rData1(15,8).asSInt.pad(32).asUInt,
            2.U(32.W)           -> rData1(23,16).asSInt.pad(32).asUInt,
            3.U(32.W)           -> rData1(31,24).asSInt.pad(32).asUInt
          ))
        }.otherwise {
          io.out.bits.regsWdata := MuxLookup(lowAddr, 0.U(32.W))(Seq(
            0.U(32.W)           -> rData1(7,0).pad(32),
            1.U(32.W)           -> rData1(15,8).pad(32),
            2.U(32.W)           -> rData1(23,16).pad(32),
            3.U(32.W)           -> rData1(31,24).pad(32)
          ))
        }
      }.elsewhen (memLen === two) {
        when (memSign === signed) {
          io.out.bits.regsWdata := MuxLookup(lowAddr, 0.U(32.W))(Seq(
            0.U(32.W)           -> rData1(15,0).asSInt.pad(32).asUInt,
            1.U(32.W)           -> rData1(23,8).asSInt.pad(32).asUInt,
            2.U(32.W)           -> rData1(31,16).asSInt.pad(32).asUInt
          ))
        }.otherwise {
          io.out.bits.regsWdata := MuxLookup(lowAddr, 0.U(32.W))(Seq(
            0.U(32.W)           -> rData1(15,0).pad(32),
            1.U(32.W)           -> rData1(23,8).pad(32),
            2.U(32.W)           -> rData1(31,16).pad(32)
          ))
        }
      }.otherwise {
        io.out.bits.regsWdata := rData1
      }
    }.otherwise {
      when (memLen === two) {
        io.out.bits.regsWdata := Mux(memSign === signed,
                                     (rData2(7,0) ## rData1(31,24)).asSInt.pad(32).asUInt,
                                     (rData2(7,0) ## rData1(31,24)).pad(32))
      }.otherwise {
        io.out.bits.regsWdata := MuxLookup(lowAddr, 0.U(32.W))(Seq(
          1.U(32.W)           -> rData2(7,0) ## rData1(31,8),
          2.U(32.W)           -> rData2(15,0) ## rData1(31,16),
          3.U(32.W)           -> rData2(23,0) ## rData1(31,24),
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

  io.in.ready  := state === sLsuReady
  io.out.valid := state === sLsuValid

  switch (state) {
    is (sLsuReady) {
      when (io.in.valid && io.in.ready) {
        state := sJudge
      }
    }
    is (sJudge) {
      when (memRdEn || memWrEn) {
        when (oneStep) {
          state := Mux(memRdEn, sReadReqValid, sWriteReqValid)
        }.otherwise {
          state := Mux(memRdEn, sReadReqValid1, sWriteReqValid1)
        }
      }.otherwise {
        state := sLsuValid
      }
    }
    is (sReadReqValid) {
      when (io.ram.reqValid && io.ram.reqReady) {
        state := sReadRespReady
      }
    }
    is (sReadRespReady) {
      when (io.ram.respValid && io.ram.respReady) {
        state := sLsuValid
      }
    }
    is (sWriteReqValid) {
      when (io.ram.reqValid && io.ram.reqReady) {
        state := sWriteRespReady
      }
    }
    is (sWriteRespReady) {
      when (io.ram.respValid && io.ram.respReady) {
        state := sLsuValid
      }
    }
    is (sReadReqValid1) {
      when (io.ram.reqValid && io.ram.reqReady) {
        state := sReadRespReady1
      }
    }
    is (sReadRespReady1) {
      when (io.ram.respValid && io.ram.respReady) {
        state := sReadReqValid2
      }
    }
    is (sReadReqValid2) {
      when (io.ram.reqValid && io.ram.reqReady) {
        state := sReadRespReady2
      }
    }
    is (sReadRespReady2) {
      when (io.ram.respValid && io.ram.respReady) {
        state := sLsuValid
      }
    }
    is (sWriteReqValid1) {
      when (io.ram.reqValid && io.ram.reqReady) {
        state := sWriteRespReady1
      }
    }
    is (sWriteRespReady1) {
      when (io.ram.respValid && io.ram.respReady) {
        state := sWriteReqValid2
      }
    }
    is (sWriteReqValid2) {
      when (io.ram.reqValid && io.ram.reqReady) {
        state := sWriteRespReady2
      }
    }
    is (sWriteRespReady2) {
      when (io.ram.respValid && io.ram.respReady) {
        state := sLsuValid
      }
    }
    is (sLsuValid) {
      when (io.out.valid && io.out.ready) {
        state := sLsuReady
      }
    }
  }
}