import chisel3._
import chisel3.util._

object Wbu {
  object State extends ChiselEnum {
    val sWbuReady, sWriteRegs, sWbuValid = Value
  }
}

class Wbu extends Module {
  import Wbu.State._

  val io = IO(new Bundle {
    val in      = Flipped(Decoupled(new LsuToWbuPort))
    val pc      = new WbuToPcPort
    val regs    = new WbuToRegsPort
    val csrs    = new WbuToCsrsPort
    val out     = Decoupled(new WbuToIfuPort)
    val commit  = Output(Bool())
  })

  val state         = RegInit(sWbuReady)

  val regsWaddr     = Reg(UInt(4.W))   
  val regsWdata     = Reg(UInt(32.W))
  val regsWen       = Reg(Bool())    
  val pcWdata       = Reg(UInt(32.W))
  val pcWen         = Reg(Bool())    
  val csrWaddr      = Reg(UInt(12.W))
  val csrWdata      = Reg(UInt(32.W))
  val csrWen        = Reg(Bool())    
  val isEcall       = Reg(Bool())    
  val isMret        = Reg(Bool())    

  when (io.in.valid && io.in.ready) {
    regsWaddr     := io.in.bits.regsWaddr
    regsWdata     := io.in.bits.regsWdata
    regsWen       := io.in.bits.regsWen  
    pcWdata       := io.in.bits.pcWdata  
    pcWen         := io.in.bits.pcWen    
    csrWaddr      := io.in.bits.csrWaddr 
    csrWdata      := io.in.bits.csrWdata 
    csrWen        := io.in.bits.csrWen   
    isEcall       := io.in.bits.isEcall  
    isMret        := io.in.bits.isMret   
  }

  //pc
  io.pc.wData := pcWdata
  io.pc.wEn := Mux(state === sWriteRegs, pcWen, false.B)
  //regs
  io.regs.wData := regsWdata
  io.regs.wAddr := regsWaddr
  io.regs.wEn := Mux(state === sWriteRegs, regsWen, false.B)
  //csrs
  io.csrs.wData := csrWdata
  io.csrs.wAddr := csrWaddr
  io.csrs.wEn := Mux(state === sWriteRegs, csrWen, false.B)
  io.csrs.isEcall := isEcall
  io.csrs.isMret := isMret

  io.in.ready := state === sWbuReady
  io.out.valid := state === sWbuValid
  io.commit := io.out.valid && io.out.ready

  switch (state) {
    is (sWbuReady) {
      when (io.in.valid && io.in.ready) {
        state := sWriteRegs
      }
    }
    is (sWriteRegs) {
      state := sWbuValid
    }
    is (sWbuValid) {
      when (io.out.valid && io.out.ready) {
        state := sWbuReady
      }
    }
  }
}