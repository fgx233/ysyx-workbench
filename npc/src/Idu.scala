import chisel3._
import chisel3.util._

// 负责根据IDU给出的指令，把这个指令的信息提取出来，送给EXU和LSU
class Idu extends Module {
  // IDU的指令输入
  val inst = IO(Input(UInt(32.W)))
  // 寄存器操作数输出
  val src1 = IO(Output(UInt(32.W)))
  val src2 = IO(Output(UInt(32.W)))
  val rd   = IO(Output(UInt(5.W)))
  // 立即数操作数输出
  val immI = IO(Output(UInt(32.W)))
  val immU = IO(Output(UInt(32.W)))
  val immS = IO(Output(UInt(32.W)))

  // 译码判断信号
  val isAdd        = IO(Output(Bool()))
  val isAddi       = IO(Output(Bool()))
  val isLw         = IO(Output(Bool()))
  val isLbu        = IO(Output(Bool()))
  val isJalr       = IO(Output(Bool()))
  val isLui        = IO(Output(Bool()))
  val isSb         = IO(Output(Bool()))
  val isSw         = IO(Output(Bool()))
  val isAuipc      = IO(Output(Bool()))
  val isEbreak     = IO(Output(Bool()))
  val isInvalid    = IO(Output(Bool()))

  // 与寄存器堆的接口
  val rs1 = IO(Output(UInt(5.W)))
  val rs2 = IO(Output(UInt(5.W)))
  val rdata1 = IO(Input(UInt(32.W)))
  val rdata2 = IO(Input(UInt(32.W)))

  // 存储器读使能
  val ramRen = IO(Output(Bool()))

  // 存储器/寄存器写使能
  val regsEn = IO(Output(Bool()))
  val ramWen   = IO(Output(Bool()))

  ramRen := isLw || isLbu

  regsEn := isAdd || isAddi || isLw || isLbu || isJalr || isLui || isAuipc
  ramWen := isSb || isSw
  // 寄存器操作数输出
  rs1 := inst(19,15)
  rs2 := inst(24,20)

  src1 := rdata1
  src2 := rdata2
  rd := inst(11,7)

  // 立即数操作数输出
  immI := inst(31,20).asSInt.pad(32).asUInt
  immU := Cat(inst(31,12), 0.U(12.W))
  immS := Cat(inst(31,25), inst(11,7)).asSInt.pad(32).asUInt

  // R型操作数译码
  isAdd  := (inst(31,25) === "b0000000".U) && (inst(14,12) === "b000".U) && (inst(6,0) === "b0110011".U)
  // I型操作数译码
  isAddi := (inst(14,12) === "b000".U) && (inst(6,0) === "b0010011".U)
  isLw   := (inst(14,12) === "b010".U) && (inst(6,0) === "b0000011".U)
  isLbu  := (inst(14,12) === "b100".U) && (inst(6,0) === "b0000011".U)
  isJalr := (inst(14,12) === "b000".U) && (inst(6,0) === "b1100111".U)
  // U型操作数译码
  isAuipc := (inst(6,0) === "b0010111".U)
  isLui   := (inst(6,0) === "b0110111".U)
  // S型操作数译码
  isSb   := (inst(14,12) === "b000".U) && (inst(6,0) === "b0100011".U) 
  isSw   := (inst(14,12) === "b010".U) && (inst(6,0) === "b0100011".U)
  // ebreak识别
  isEbreak := inst === "b0000000_00001_00000_000_00000_1110011".U
  // invalid识别
  isInvalid := !(isAdd || isAddi || isLw || isLbu || isJalr || isLui || isSb || isSw || isEbreak || isAuipc)
}
