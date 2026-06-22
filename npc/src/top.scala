import chisel3._
import chisel3.util.experimental.loadMemoryFromFile
import chisel3.util._

/** Compute GCD using subtraction method. Subtracts the smaller from the larger until register y is zero. value in
  * register x is then the GCD
  */
class top extends Module {
  val inst      = IO(Input(UInt(8.W)))
  val inst_addr = IO(Output(UInt(4.W)))

  val pc        = Module(new pc)
  val regs      = Module(new regs)


  //指令译码标志信号
  val is_add    = inst(7,6) === "b00".U(2.W)
  val is_out    = inst(7,6) === "b01".U(2.W)
  val is_li     = inst(7,6) === "b10".U(2.W)
  val is_bner0  = inst(7,6) === "b11".U(2.W)

  //指令信息提取
  val addr      = inst(5,2)
  val imm       = inst(3,0)
  val rs1       = inst(3,2)
  val rs2       = inst(1,0)
  val rd        = inst(5,4)

  
  //寄存器数据
  val src1      = regs.rdata1
  val src2      = regs.rdata2

  val rd_wdata  = Mux(is_add, src1 + src2, imm)
  val pc_wdata  = Mux(is_bner0 && (src1 =/= src2), addr, pc.curr_pc + 1.U)

  //regs输入连接
  regs.raddr1   := Mux(is_bner0, 0.U, rs1)
  regs.raddr2   := rs2  
  regs.wen      := (is_add | is_li)
  regs.waddr    := rd
  regs.wdata    := rd_wdata

  //pc输入连接
  pc.next_pc    := pc_wdata

  //rom输入连接
  inst_addr := pc.curr_pc

}