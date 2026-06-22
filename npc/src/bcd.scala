import chisel3._
import chisel3.util._
import os.stat
/** Compute GCD using subtraction method. Subtracts the smaller from the larger until register y is zero. value in
  * register x is then the GCD
  */
class bcd extends Module {
  val bin = IO(Input(UInt(8.W)))
  val seg1 = IO(Output(UInt(7.W)))
  val seg2 = IO(Output(UInt(7.W)))
  val seg3 = IO(Output(UInt(7.W)))


  val bcd3 = RegInit(0.U(4.W))
  val bcd2 = RegInit(0.U(4.W))
  val bcd1 = RegInit(0.U(4.W))

  val state_cnt = RegInit(0.U(4.W))
  val cnt_reg = RegInit(0.U(8.W))

  when(state_cnt === 8.U) {
    cnt_reg := bin
  }.otherwise {
    cnt_reg := cnt_reg << 1
  }

  when(state_cnt === 8.U) {
    state_cnt := 0.U
  }.otherwise {
    state_cnt := state_cnt + 1.U
  }
  val carry_1 = Mux(bcd1 <= 4.U, bcd1, bcd1 + 3.U)
  val carry_2 = Mux(bcd2 <= 4.U, bcd2, bcd2 + 3.U)
  val carry_3 = Mux(bcd3 <= 4.U, bcd3, bcd3 + 3.U)
  when(state_cnt === 8.U) {
    bcd1 := 0.U
  }.elsewhen(state_cnt <= 7.U) {
    bcd1 := Cat(carry_1(2,0), cnt_reg(7))
  }

  when(state_cnt === 8.U) {
    bcd2 := 0.U
  }.elsewhen(state_cnt <= 7.U) {
    bcd2 := Cat(carry_2(2,0), carry_1(3))
  }

  when(state_cnt === 8.U) {
    bcd3 := 0.U
  }.elsewhen(state_cnt <= 7.U) {
    bcd3 := Cat(carry_3(2,0), carry_2(3))
  }

  val seg12 = RegInit(0.U(12.W))
  when(state_cnt === 8.U) {
    seg12 := Cat(bcd3, bcd2, bcd1)
  }

  val bcd7seg_1 = Module(new bcd7seg)
  bcd7seg_1.in := seg12(3,0)
  seg1 := bcd7seg_1.out

  val bcd7seg_2 = Module(new bcd7seg)
  bcd7seg_2.in := seg12(7,4)
  seg2 := bcd7seg_2.out


  val bcd7seg_3 = Module(new bcd7seg)
  bcd7seg_3.in := seg12(11,8)
  seg3 := bcd7seg_3.out

}

class bcd7seg extends RawModule {
  val in = IO(Input(UInt(4.W)))
  val out = IO(Output(UInt(7.W)))

  out := MuxLookup(in, 0.U(7.W))(Seq(
    0.U -> "b1000000".U(7.W),
    1.U -> "b1111001".U(7.W),
    2.U -> "b0100100".U(7.W),
    3.U -> "b0110000".U(7.W),
    4.U -> "b0011001".U(7.W),
    5.U -> "b0010010".U(7.W),
    6.U -> "b0000010".U(7.W),
    7.U -> "b1111000".U(7.W),
    8.U -> "b0000000".U(7.W),
    9.U -> "b0010000".U(7.W),
    10.U -> "b0001000".U(7.W),
    11.U -> "b0000011".U(7.W),
    12.U -> "b1000110".U(7.W),
    13.U -> "b0100001".U(7.W),
    14.U -> "b0000110".U(7.W),
    15.U -> "b0001110".U(7.W)
  ))
}