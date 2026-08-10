import chisel3._
import chisel3.util._
import common._

// 负责根据当前PC从存储器中取出一条指令
class Ifu extends Module {
  val pc            = IO(Input(UInt(32.W)))
  val inst          = IO(Output(UInt(32.W)))

  val cpuReadAddr1  = IO(Output(UInt(32.W)))
  val cpuReadData1  = IO(Input(UInt(32.W)))
  val cpuReadEn1    = IO(Output(Bool()))
  val cpuReadLen1   = IO(Output(UInt(2.W)))
  val cpuReadSign1  = IO(Output(Bool()))

  inst              := cpuReadData1
  cpuReadAddr1      := pc
  cpuReadEn1        := true.B
  cpuReadLen1       := RamLen.four
  cpuReadSign1      := false.B
}