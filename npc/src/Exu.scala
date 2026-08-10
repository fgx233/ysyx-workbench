import chisel3._
import chisel3.util._
import common._

class Exu extends Module {
  val aluOprand1          = IO(Input(UInt(32.W)))
  val aluOprand2          = IO(Input(UInt(32.W)))
  val aluOpcode           = IO(Input(UInt(4.W)))
  val pcOprand1           = IO(Input(UInt(32.W)))
  val pcOprand2           = IO(Input(UInt(32.W)))
  val isBranch            = IO(Input(Bool()))
  val isUncondjmp         = IO(Input(Bool()))

  val pc                  = IO(Output(UInt(32.W)))
  val aluResult           = IO(Output(UInt(32.W)))

  val add                 = (aluOprand1.asSInt + aluOprand2.asSInt).asUInt
  val sub                 = (aluOprand1.asSInt - aluOprand2.asSInt).asUInt
  val and                 = aluOprand1 & aluOprand2
  val or                  = aluOprand1 | aluOprand2
  val xor                 = aluOprand1 ^ aluOprand2
  val equ                 = (aluOprand1 === aluOprand2).asUInt
  val neq                 = (aluOprand1 =/= aluOprand2).asUInt
  val largeOrEquS         = (aluOprand1.asSInt >= aluOprand2.asSInt).asUInt
  val largeOrEquU         = (aluOprand1 >= aluOprand2).asUInt
  val smallS              = (aluOprand1.asSInt < aluOprand2.asSInt).asUInt
  val smallU              = (aluOprand1 < aluOprand2).asUInt
  val leftShiftU          = (aluOprand1 << aluOprand2(4,0))(31,0)
  val rightShiftS         = (aluOprand1.asSInt >> aluOprand2(4,0)).asUInt
  val rightShiftU         = aluOprand1 >> aluOprand2(4,0)

  aluResult               := MuxLookup(aluOpcode, add)(Seq(
    AluOpcode.add         -> add,
    AluOpcode.sub         -> sub,
    AluOpcode.and         -> and,
    AluOpcode.or          -> or,
    AluOpcode.xor         -> xor,
    AluOpcode.equ         -> equ,
    AluOpcode.neq         -> neq,
    AluOpcode.largeOrEquS -> largeOrEquS,
    AluOpcode.largeOrEquU -> largeOrEquU,
    AluOpcode.smallS      -> smallS,
    AluOpcode.smallU      -> smallU,
    AluOpcode.leftShiftU  -> leftShiftU,
    AluOpcode.rightShiftS -> rightShiftS,
    AluOpcode.rightShiftU -> rightShiftU
  ))
  val isBranchSuccess     = isBranch && aluResult(0).asBool
  
  val pcTmp               = pcOprand1 + MuxCase(4.U(32.W), Seq(
    isUncondjmp           -> pcOprand2,
    isBranchSuccess       -> pcOprand2
  ))
  pc                      := pcTmp & "hffff_fffe".U(32.W)
}
