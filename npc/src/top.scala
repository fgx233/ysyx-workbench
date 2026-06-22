import chisel3._
import chisel3.util.experimental.loadMemoryFromFile
import chisel3.util.MuxLookup

/** Compute GCD using subtraction method. Subtracts the smaller from the larger until register y is zero. value in
  * register x is then the GCD
  */
class top extends Module {
  val seg_out1   = IO(Output(UInt(7.W)))
  val seg_out2   = IO(Output(UInt(7.W)))
  val seg_out3   = IO(Output(UInt(7.W)))

  val pc        = Module(new pc)
  val regs      = Module(new regs)
  val rom       = Module(new rom)
  //数码管寄存器  
  val seg       = RegInit(0.U(8.W))

  //指令译码标志信号
  val is_add    = rom.inst(7,6) === "b00".U(2.W)
  val is_out    = rom.inst(7,6) === "b01".U(2.W)
  val is_li     = rom.inst(7,6) === "b10".U(2.W)
  val is_bner0  = rom.inst(7,6) === "b11".U(2.W)

  //指令信息提取
  val addr      = rom.inst(5,2)
  val imm       = rom.inst(3,0)
  val rs1       = rom.inst(3,2)
  val rs2       = rom.inst(1,0)
  val rd        = rom.inst(5,4)

  
  //寄存器数据
  val src1      = regs.rdata1
  val src2      = regs.rdata2

  val rd_wdata  = Mux(is_add, src1 + src2, imm)
  val pc_wdata  = Mux(is_bner0 && (src1 =/= src2), addr, pc.curr_pc + 1.U)

  //regs输入连接
  regs.raddr1   := Mux(is_bner0, 0.U, rs1)
  regs.raddr2   := rs2  
  regs.wen      := is_add | is_li
  regs.waddr    := rd
  regs.wdata    := rd_wdata

  //pc输入连接
  pc.next_pc    := pc_wdata

  //rom输入连接
  rom.inst_addr := pc.curr_pc

  when (is_out) {
    seg := src1
  }

  val seg_bin_to_bcd = Module(new bcd)
  seg_bin_to_bcd.bin := seg
  seg_out1 := seg_bin_to_bcd.seg1
  seg_out2 := seg_bin_to_bcd.seg2
  seg_out3 := seg_bin_to_bcd.seg3

}