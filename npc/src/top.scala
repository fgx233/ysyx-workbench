import chisel3._
import chisel3.util.experimental.loadMemoryFromFile

/** Compute GCD using subtraction method. Subtracts the smaller from the larger until register y is zero. value in
  * register x is then the GCD
  */
class top extends Module {
  val hsync = IO(Output(Bool()))
  val vsync = IO(Output(Bool()))
  val vga_ena = IO(Output(Bool()))

  val R = IO(Output(UInt(8.W)))
  val G = IO(Output(UInt(8.W)))
  val B = IO(Output(UInt(8.W)))


  val rom = Mem(307200, UInt(32.W))
  // loadMemoryFromFile(rom, "/home/fgx/projects/Verilog/digital_expriment/vga/nvboard/resource/taki.hex")

  val hcnt = RegInit(0.U(10.W))
  val vcnt = RegInit(0.U(10.W))

  when(hcnt === 799.U) {
    hcnt := 0.U
  }.otherwise {
    hcnt := hcnt + 1.U
  }

  when(vcnt === 524.U && hcnt === 799.U) {
    vcnt := 0.U
  }.elsewhen(hcnt === 799.U){
    vcnt := vcnt + 1.U
  }

  val hactive = (hcnt >= 144.U) && (hcnt <= 783.U)
  val vactive = (vcnt >= 35.U) && (vcnt <= 514.U)


  hsync := hcnt >= 96.U
  vsync := vcnt >= 2.U
  vga_ena := hactive && vactive
  
  val x = Wire(UInt(10.W))
  val y = Wire(UInt(10.W))

  when(vga_ena) {
    x := hcnt - 144.U
    y := vcnt - 35.U
  }.otherwise {
    x := 0.U
    y := 0.U
  }

  val pixel = rom.read(x + y*640.U)

  when(vga_ena) {
    R := pixel(23, 16)
    G := pixel(15, 8)
    B := pixel(7, 0)
  }.otherwise {
    R := 0.U
    G := 0.U
    B := 0.U
  }
  
}