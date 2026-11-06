import chisel3._
import chisel3.util._
import os.stat


/* 
Exu的行为：
1、什么都不做，只传递IFU的信号
2、执行运算，将运算结果写入regsWdata
3、执行运算，将运算结果写入memAddr
4、执行运算，将运算结果写入csrWdata
 */

object ExuAction extends ChiselEnum {
  val onlyTransfer, computeReg, computeCsr, computeAddr = Value
}

/* 
alu的行为：
1、加法
2、减法
3、与
4、或
5、异或
6、逻辑左移
7、逻辑右移
8、算术右移
9、有符号小于
10、无符号小于
11、空闲
 */

object AluOption extends ChiselEnum {
  val add, sub, and, or, xor, llshift, lrshift,arshift, sless, uless = Value
}

/* 
功能：传递IFU提取的参数，进行进一步加工，计算pc，把信息打包传给lsu
状态：
1、等待valid
2、等待ready
 */

object Exu {
  object State extends ChiselEnum {
    val sWaitValid, sWaitReady = Value
  }
}

class Exu extends Module {
  import ExuAction._
  import AluOption._
  import Exu.State._
  
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(new IduToExuPort))
    val out = Decoupled(new ExuToLsuPort)
  })

  val state     = RegInit(sWaitValid)
  
  val exuAction = Reg(ExuAction())
  val aluOption = Reg(AluOption())
  val aluOprand1= Reg(UInt(32.W))
  val aluOprand2= Reg(UInt(32.W))
  val pcOprand1 = Reg(UInt(32.W))
  val pcOprand2 = Reg(UInt(32.W))

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

  when (io.in.valid && io.in.ready) {
    exuAction := io.in.bits.exuAction 
    aluOption := io.in.bits.aluOption 
    aluOprand1:= io.in.bits.aluOprand1
    aluOprand2:= io.in.bits.aluOprand2
    pcOprand1 := io.in.bits.pcOprand1 
    pcOprand2 := io.in.bits.pcOprand2 
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
  
  val aluResult = MuxLookup(aluOption, 0.U(32.W))(Seq(
    add     -> (aluOprand1 + aluOprand2),
    sub     -> (aluOprand1 - aluOprand2),
    and     -> (aluOprand1 & aluOprand2),
    or      -> (aluOprand1 | aluOprand2),
    xor     -> (aluOprand1 ^ aluOprand2),
    llshift -> (aluOprand1 << aluOprand2(4,0)),
    lrshift -> (aluOprand1 >> aluOprand2(4,0)),
    arshift -> (aluOprand1.asSInt >> aluOprand2(4,0)).asUInt,
    sless   -> Mux(aluOprand1.asSInt < aluOprand2.asSInt, 1.U(32.W), 0.U(32.W)),
    uless   -> Mux(aluOprand1 < aluOprand2, 1.U(32.W), 0.U(32.W))
  ))

  val pcResult = (pcOprand1 + pcOprand2) & "hfffffffe".U(32.W)

  io.out.bits.memAddr   := aluResult
  io.out.bits.memRdEn   := memRdEn
  io.out.bits.memWrEn   := memWrEn
  io.out.bits.memWrData := memWrData
  io.out.bits.memLen    := memLen
  io.out.bits.memSign   := memSign

  io.out.bits.regsWaddr := regsWaddr
  io.out.bits.regsWdata := Mux(exuAction === computeReg, aluResult, regsWdata)
  io.out.bits.regsWen   := regsWen
  
  io.out.bits.pcWdata   := Mux(isEcall || isMret, pcWdata, pcResult)
  io.out.bits.pcWen     := pcWen

  io.out.bits.csrWaddr  := csrWaddr
  io.out.bits.csrWdata  := Mux(exuAction === computeCsr, aluResult, csrWdata)
  io.out.bits.csrWen    := csrWen

  io.out.bits.isEcall   := isEcall
  io.out.bits.isMret    := isMret

  io.in.ready           := state === sWaitValid
  io.out.valid          := state === sWaitReady

  switch (state) {
    is (sWaitValid) {
      when (io.in.valid && io.in.ready) {
        state := sWaitReady
      }
    }
    is (sWaitReady) {
      when (io.out.valid && io.out.ready) {
        state := sWaitValid
      }
    }
  }

}