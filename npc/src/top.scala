import chisel3._
import chisel3.util._

class top extends Module {
  val pc = Module(new Pc)
  val regs = Module(new Regs)
  val ram = Module(new Ram)
  val monitor = Module(new Monitor)

  val ifu = Module(new Ifu)
  val idu = Module(new Idu)
  val exu = Module(new Exu)
  val lsu = Module(new Lsu)

  // Monitor模块连线
  monitor.pc := pc.rdata
  monitor.isEbreak := idu.isEbreak
  monitor.isInvalid := idu.isInvalid

  // Ifu模块连线
  ifu.pc := pc.rdata
  ifu.rdata := lsu.rdata1Cpu

  // Idu模块连线
  idu.inst := ifu.inst

  idu.rdata1 := regs.rdata1
  idu.rdata2 := regs.rdata2

  // Exu模块连线
  exu.pc := pc.rdata
  exu.src1 := idu.src1
  exu.src2 := idu.src2
  exu.immI := idu.immI
  exu.immU := idu.immU
  exu.immS := idu.immS

  exu.isAdd := idu.isAdd
  exu.isAddi := idu.isAddi
  exu.isLw := idu.isLw
  exu.isLbu := idu.isLbu
  exu.isJalr := idu.isJalr
  exu.isLui := idu.isLui
  exu.isSb := idu.isSb
  exu.isSw := idu.isSw
  exu.isAuipc := idu.isAuipc
  
  exu.ramReadData := lsu.rdata2Cpu

  // Lsu模块连线
  lsu.ren1Cpu := ifu.ren
  lsu.raddr1Cpu := ifu.raddr

  lsu.ren2Cpu := idu.ramRen
  lsu.raddr2Cpu := exu.ramReadAdder

  lsu.wenCpu := idu.ramWen
  lsu.waddrCpu := exu.ramWriteAddr
  lsu.wdataCpu := exu.ramWriteData

  lsu.isLw := idu.isLw
  lsu.isLbu := idu.isLbu
  lsu.isSw := idu.isSw
  lsu.isSb := idu.isSb

  lsu.rdata1Ram := ram.inst
  lsu.rdata2Ram := ram.rdata

  // pc模块连线
  pc.wen := ifu.ren
  pc.wdata := exu.nextPc

  // regs模块连线
  regs.raddr1 := idu.rs1(3,0)
  regs.raddr2 := idu.rs2(3,0)
  regs.waddr := idu.rd(3,0)
  regs.wdata := exu.regsWriteData
  regs.wen := idu.regsEn

  // ram模块连线
  ram.fetch_en := lsu.ren1Ram
  ram.pc := lsu.raddr1Ram

  ram.ren := lsu.ren2Ram
  ram.raddr := lsu.raddr2Ram

  ram.wen := lsu.wenRam
  ram.waddr := lsu.waddrRam
  ram.wdata := lsu.wdataRam
  ram.wmask := lsu.wmask
}