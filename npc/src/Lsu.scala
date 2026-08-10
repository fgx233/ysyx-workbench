import chisel3._
import chisel3.util._
import common._

class Lsu extends Module {
  val cpuReadAddr1 = IO(Input(UInt(32.W)))
  val cpuReadData1 = IO(Output(UInt(32.W)))
  val cpuReadLen1  = IO(Input(UInt(2.W)))
  val cpuReadEn1   = IO(Input(Bool()))
  val cpuReadSign1 = IO(Input(Bool()))

  val cpuReadAddr2 = IO(Input(UInt(32.W)))
  val cpuReadData2 = IO(Output(UInt(32.W)))
  val cpuReadLen2  = IO(Input(UInt(2.W)))
  val cpuReadEn2   = IO(Input(Bool()))
  val cpuReadSign2 = IO(Input(Bool()))
  
  val cpuWriteAddr = IO(Input(UInt(32.W)))
  val cpuWriteData = IO(Input(UInt(32.W)))
  val cpuWriteLen  = IO(Input(UInt(2.W)))
  val cpuWriteEn   = IO(Input(Bool()))

  val ramReadAddr1 = IO(Output(UInt(32.W)))
  val ramReadData1 = IO(Input(UInt(32.W)))
  val ramReadEn1   = IO(Output(Bool()))
  val ramReadAddr2 = IO(Output(UInt(32.W)))
  val ramReadData2 = IO(Input(UInt(32.W)))
  val ramReadEn2   = IO(Output(Bool()))
  val ramWriteAddr = IO(Output(UInt(32.W)))
  val ramWriteData = IO(Output(UInt(32.W)))
  val ramWriteEn   = IO(Output(Bool()))
  val ramWriteMask = IO(Output(UInt(4.W)))

  // 读1
  ramReadAddr1 := Cat(cpuReadAddr1(31,2), "b00".U(2.W))
  ramReadEn1   := cpuReadEn1
  val word1     = ramReadData1
  val halfWordLowS1 = word1(15,0).asSInt.pad(32).asUInt
  val halfWordLowU1 = word1(15,0).pad(32)
  val halfWordHighS1 = word1(31,16).asSInt.pad(32).asUInt
  val halfWordHighU1 = word1(31,16).pad(32)
  val quarWordOneS1 = word1(7,0).asSInt.pad(32).asUInt
  val quarWordOneU1 = word1(7,0).pad(32)
  val quarWordTwoS1 = word1(15,8).asSInt.pad(32).asUInt
  val quarWordTwoU1 = word1(15,8).pad(32)
  val quarWordThreeS1 = word1(23,16).asSInt.pad(32).asUInt
  val quarWordThreeU1 = word1(23,16).pad(32)
  val quarWordFourS1 = word1(31,24).asSInt.pad(32).asUInt
  val quarWordFourU1 = word1(31,24).pad(32)
  
  when (cpuReadLen1 === RamLen.four) {
    cpuReadData1 := word1
  }.elsewhen (cpuReadSign1 === true.B) {
    when (cpuReadLen1 === RamLen.two) {
      cpuReadData1 := Mux(cpuReadAddr1(1,0) === 0.U(2.W), halfWordLowS1, halfWordHighS1)
    }.elsewhen(cpuReadLen1 === RamLen.one) {
      cpuReadData1 := MuxLookup(cpuReadAddr1(1,0), quarWordOneS1)(Seq(
        "b00".U(2.W) -> quarWordOneS1,
        "b01".U(2.W) -> quarWordTwoS1,
        "b10".U(2.W) -> quarWordThreeS1,
        "b11".U(2.W) -> quarWordFourS1
      ))
    }.otherwise {
      cpuReadData1 := word1
    }
  }.otherwise {
    when (cpuReadLen1 === RamLen.two) {
      cpuReadData1 := Mux(cpuReadAddr1(1,0) === 0.U(2.W), halfWordLowU1, halfWordHighU1)
    }.elsewhen(cpuReadLen1 === RamLen.one) {
      cpuReadData1 := MuxLookup(cpuReadAddr1(1,0), quarWordOneU1)(Seq(
        "b00".U(2.W) -> quarWordOneU1,
        "b01".U(2.W) -> quarWordTwoU1,
        "b10".U(2.W) -> quarWordThreeU1,
        "b11".U(2.W) -> quarWordFourU1
      ))
    }.otherwise {
      cpuReadData1 := word1
    }
  }

  // 读2
  ramReadAddr2 := Cat(cpuReadAddr2(31,2), "b00".U(2.W))
  ramReadEn2   := cpuReadEn2
  val word2     = ramReadData2
  val halfWordLowS2 = word2(15,0).asSInt.pad(32).asUInt
  val halfWordLowU2 = word2(15,0).pad(32)
  val halfWordHighS2 = word2(31,16).asSInt.pad(32).asUInt
  val halfWordHighU2 = word2(31,16).pad(32)
  val quarWordOneS2 = word2(7,0).asSInt.pad(32).asUInt
  val quarWordOneU2 = word2(7,0).pad(32)
  val quarWordTwoS2 = word2(15,8).asSInt.pad(32).asUInt
  val quarWordTwoU2 = word2(15,8).pad(32)
  val quarWordThreeS2 = word2(23,16).asSInt.pad(32).asUInt
  val quarWordThreeU2 = word2(23,16).pad(32)
  val quarWordFourS2 = word2(31,24).asSInt.pad(32).asUInt
  val quarWordFourU2 = word2(31,24).pad(32)
  
  when (cpuReadLen2 === RamLen.four) {
    cpuReadData2 := word2
  }.elsewhen (cpuReadSign2 === true.B) {
    when (cpuReadLen2 === RamLen.two) {
      cpuReadData2 := Mux(cpuReadAddr2(1,0) === 0.U(2.W), halfWordLowS2, halfWordHighS2)
    }.elsewhen(cpuReadLen2 === RamLen.one) {
      cpuReadData2 := MuxLookup(cpuReadAddr2(1,0), quarWordOneS2)(Seq(
        "b00".U(2.W) -> quarWordOneS2,
        "b01".U(2.W) -> quarWordTwoS2,
        "b10".U(2.W) -> quarWordThreeS2,
        "b11".U(2.W) -> quarWordFourS2
      ))
    }.otherwise {
      cpuReadData2 := word2
    }
  }.otherwise {
    when (cpuReadLen2 === RamLen.two) {
      cpuReadData2 := Mux(cpuReadAddr2(1,0) === 0.U(2.W), halfWordLowU2, halfWordHighU2)
    }.elsewhen(cpuReadLen2 === RamLen.one) {
      cpuReadData2 := MuxLookup(cpuReadAddr2(1,0), quarWordOneU2)(Seq(
        "b00".U(2.W) -> quarWordOneU2,
        "b01".U(2.W) -> quarWordTwoU2,
        "b10".U(2.W) -> quarWordThreeU2,
        "b11".U(2.W) -> quarWordFourU2
      ))
    }.otherwise {
      cpuReadData2 := word2
    }
  }

  // 写
  ramWriteAddr := Cat(cpuWriteAddr(31,2), "b00".U(2.W))
  ramWriteEn := cpuWriteEn
  val word = cpuWriteData
  ramWriteData := MuxLookup(cpuWriteLen, word)(Seq(
    RamLen.one -> Fill(4, word(7,0)),
    RamLen.two -> Fill(2, word(15,0)),
    RamLen.four -> word
  ))
  when (cpuWriteLen === RamLen.four) {
    ramWriteMask := "b1111".U(4.W)
  }.elsewhen (cpuWriteLen === RamLen.two) {
    ramWriteMask := Mux(cpuWriteAddr(1,0) === 0.U, "b0011".U(4.W), "b1100".U(4.W))
  }.otherwise {
    ramWriteMask := MuxLookup(cpuWriteAddr(1,0), "b1111".U(4.W))(Seq(
      "b00".U(2.W) -> "b0001".U(4.W),
      "b01".U(2.W) -> "b0010".U(4.W),
      "b10".U(2.W) -> "b0100".U(4.W),
      "b11".U(2.W) -> "b1000".U(4.W)
    ))
  }
}