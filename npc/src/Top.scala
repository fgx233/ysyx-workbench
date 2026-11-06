import chisel3._
import chisel3.util._

class top extends Module {
  val pc            = Module(new Pc)
  val regs          = Module(new Regs)
  val csrs          = Module(new Csrs)
  val ramInstFetch  = Module(new Ram("vaddr_ifetch"))
  val ramLsu        = Module(new Ram("vaddr_read"))
  val ifu           = Module(new Ifu)
  val idu           = Module(new Idu)
  val exu           = Module(new Exu)
  val lsu           = Module(new Lsu)
  val wbu           = Module(new Wbu)
  val monitor       = Module(new Monitor)

  ifu.io.pc         := pc.io.pc
  ifu.io.ifetch     <> ramInstFetch.io
  ifu.io.in         <> wbu.io.out
  ifu.io.out        <> idu.io.in

  idu.io.pc         := pc.io.pc
  idu.io.regs       <> regs.io.out
  idu.io.csrs       <> csrs.io.out
  idu.io.out        <> exu.io.in
  idu.io.monitor    <> monitor.io.idu

  exu.io.out        <> lsu.io.in

  lsu.io.out        <> wbu.io.in
  lsu.io.ram        <> ramLsu.io

  wbu.io.pc         <> pc.io.in
  wbu.io.regs       <> regs.io.in
  wbu.io.csrs       <> csrs.io.in
  
  monitor.io.commit := wbu.io.commit
  
}