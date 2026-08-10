package common
import chisel3._
import chisel3.util._

object RamLen {
  val one = 0.U(2.W)
  val two = 1.U(2.W)
  val four = 2.U(2.W)
}

object WbuChoose {
  val idu = 0.U(2.W)
  val exu = 1.U(2.W)
  val lsu = 2.U(2.W)
}

object AluOpcode {
  val add         = 0.U(4.W)
  val sub         = 1.U(4.W)
  val and         = 2.U(4.W)
  val or          = 3.U(4.W)
  val xor         = 4.U(4.W)
  val equ         = 5.U(4.W)
  val neq         = 6.U(4.W)
  val largeOrEquS = 7.U(4.W)
  val largeOrEquU = 8.U(4.W)
  val smallS      = 9.U(4.W)
  val smallU      = 10.U(4.W)
  val leftShiftU  = 11.U(4.W)
  val rightShiftS = 12.U(4.W)
  val rightShiftU = 13.U(4.W)
}