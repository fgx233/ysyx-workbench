import chisel3._
import chisel3.util._

class Lsu extends Module {
  // 对cpu接口

  // cpu读端口1:取指令
  val ren1Cpu = IO(Input(Bool()))
  val raddr1Cpu = IO(Input(UInt(32.W)))
  val rdata1Cpu = IO(Output(UInt(32.W)))

  // cpu读端口2:取指令中的内存数据
  val ren2Cpu = IO(Input(Bool()))
  val raddr2Cpu = IO(Input(UInt(32.W)))
  val rdata2Cpu = IO(Output(UInt(32.W)))

  // cpu写端口:把数据写指定内存地址
  val wenCpu = IO(Input(Bool()))
  val waddrCpu = IO(Input(UInt(32.W)))
  val wdataCpu = IO(Input(UInt(32.W)))

  // IDU输出的指令识别
  val isLw = IO(Input(Bool()))
  val isLbu = IO(Input(Bool()))
  val isSw = IO(Input(Bool()))
  val isSb = IO(Input(Bool()))

  // 对ram接口

  // ram读端口1
  val ren1Ram = IO(Output(Bool()))
  val raddr1Ram = IO(Output(UInt(32.W)))
  val rdata1Ram = IO(Input(UInt(32.W)))

  // ram读端口2
  val ren2Ram = IO(Output(Bool()))
  val raddr2Ram = IO(Output(UInt(32.W)))
  val rdata2Ram = IO(Input(UInt(32.W)))

  // ram写端口
  // cpu写端口:把数据写指定内存地址
  val wenRam = IO(Output(Bool()))
  val waddrRam = IO(Output(UInt(32.W)))
  val wdataRam = IO(Output(UInt(32.W)))
  val wmask = IO(Output(UInt(4.W)))

  // 读端口1的连接：取指令
  ren1Ram := ren1Cpu
  raddr1Ram := raddr1Cpu
  rdata1Cpu := rdata1Ram

  // 读端口2的连接：取数据
  ren2Ram := ren2Cpu
  raddr2Ram := raddr2Cpu
  val selectByte = MuxLookup(raddr2Cpu(1,0), rdata2Ram(7,0))(Seq(
    "b00".U -> rdata2Ram(7,0),
    "b01".U -> rdata2Ram(15,8),
    "b10".U -> rdata2Ram(23,16),
    "b11".U -> rdata2Ram(31,24)
  ))
  rdata2Cpu := Mux(isLw, rdata2Ram, selectByte.pad(32))

  // 写端口的连接：写数据
  wenRam := wenCpu
  waddrRam := waddrCpu
  wdataRam := Mux(isSw, wdataCpu, Cat(Seq.fill(4)(wdataCpu(7,0))))
  val mask = MuxLookup(waddrRam(1,0), "b1111".U)(Seq(
    "b00".U -> "b0001".U,
    "b01".U -> "b0010".U,
    "b10".U -> "b0100".U,
    "b11".U -> "b1000".U
  ))
  wmask := Mux(isSw, "b1111".U, mask)
}