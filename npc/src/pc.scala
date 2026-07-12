import chisel3._

class Pc extends Module {
  val wdata = IO(Input(UInt(32.W)))    //  写pc数据
  val wen   = IO(Input(Bool()))       //  写pc使能
  val rdata = IO(Output(UInt(32.W)))   //  读pc数据

  //pc寄存器
  val pcReg = RegInit("h8000_0000".U(32.W))


  when (wen) {
    pcReg := wdata
  }
  
  rdata := pcReg
}