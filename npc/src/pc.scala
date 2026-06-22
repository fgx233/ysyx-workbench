import chisel3._

class pc extends Module {
  val pc_en   = IO(Input(Bool()))
  val next_pc = IO(Input(UInt(4.W)))//写pc地址数据
  val curr_pc = IO(Output(UInt(4.W)))//pc寄存器存储的地址

  //pc寄存器
  val pc_reg = RegInit(0.U(4.W))

  when (pc_en) {
    pc_reg := next_pc
  } 
  
  curr_pc := pc_reg
}