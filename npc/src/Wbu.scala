import chisel3._
import chisel3.util._
import common._

class Wbu extends Module {
  val iduRd     = IO(Input(UInt(32.W)))
  val exuRd     = IO(Input(UInt(32.W)))
  val lsuRd     = IO(Input(UInt(32.W)))
  val wbuChoose = IO(Input(UInt(2.W)))
  val rd        = IO(Output(UInt(32.W)))

  rd := MuxLookup(wbuChoose, exuRd)(Seq(
    WbuChoose.idu -> iduRd,
    WbuChoose.exu -> exuRd,
    WbuChoose.lsu -> lsuRd
  ))
}