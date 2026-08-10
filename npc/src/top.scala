import chisel3._
import chisel3.util._

class top extends Module {
  val pc            = Module(new Pc)
  val regs          = Module(new Regs)
  val ram           = Module(new Ram)
  val ifu           = Module(new Ifu)
  val idu           = Module(new Idu)
  val exu           = Module(new Exu)
  val lsu           = Module(new Lsu)
  val wbu           = Module(new Wbu)
  val monitor       = Module(new Monitor)

  ifu.pc            := pc.rdata
  ifu.cpuReadData1  := lsu.cpuReadData1
  
  idu.inst          := ifu.inst
  idu.regsRdata1    := regs.rdata1
  idu.regsRdata2    := regs.rdata2
  idu.pc            := pc.rdata

  exu.aluOprand1    := idu.aluOprand1
  exu.aluOprand2    := idu.aluOprand2
  exu.aluOpcode     := idu.aluOpcode
  exu.pcOprand1     := idu.pcOprand1
  exu.pcOprand2     := idu.pcOprand2
  exu.isBranch      := idu.isBranch
  exu.isUncondjmp   := idu.isUncondjmp

  lsu.cpuReadAddr1  := ifu.cpuReadAddr1
  lsu.cpuReadLen1   := ifu.cpuReadLen1
  lsu.cpuReadEn1    := ifu.cpuReadEn1
  lsu.cpuReadSign1  := ifu.cpuReadSign1

  lsu.cpuReadAddr2  := exu.aluResult
  lsu.cpuReadLen2   := idu.cpuReadLen2
  lsu.cpuReadEn2    := idu.cpuReadEn2
  lsu.cpuReadSign2  := idu.cpuReadSign2

  lsu.cpuWriteAddr  := exu.aluResult
  lsu.cpuWriteData  := idu.cpuWriteData
  lsu.cpuWriteLen   := idu.cpuWriteLen
  lsu.cpuWriteEn    := idu.cpuWriteEn

  lsu.ramReadData1  := ram.rdata1
  lsu.ramReadData2  := ram.rdata2

  wbu.iduRd         := idu.regsWdata
  wbu.exuRd         := exu.aluResult
  wbu.lsuRd         := lsu.cpuReadData2
  wbu.wbuChoose     := idu.wbuChoose

  pc.wdata          := exu.pc
  pc.wen            := idu.pcEn

  regs.raddr1       := idu.regsRaddr1
  regs.raddr2       := idu.regsRaddr2
  regs.waddr        := idu.regsWaddr
  regs.wdata        := wbu.rd
  regs.wen          := idu.regsWen

  ram.raddr1        := lsu.ramReadAddr1
  ram.ren1          := lsu.ramReadEn1
  ram.raddr2        := lsu.ramReadAddr2
  ram.ren2          := lsu.ramReadEn2
  ram.waddr         := lsu.ramWriteAddr
  ram.wdata         := lsu.ramWriteData
  ram.wen           := lsu.ramWriteEn
  ram.wmask         := lsu.ramWriteMask

  monitor.pc        := pc.rdata
  monitor.isEbreak  := idu.isEbreak
  monitor.isInvalid := idu.isInv
}