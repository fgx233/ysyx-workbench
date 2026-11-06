import chisel3._
import chisel3.util._ 

object Csrs {
  val mstatusAddr   = 0x300.U(12.W)
  val mtvecAddr     = 0x305.U(12.W)
  val mepcAddr      = 0x341.U(12.W)
  val mcauseAddr    = 0x342.U(12.W)
  val mcycleAddr    = 0xB00.U(12.W)
  val mcyclehAddr   = 0xB80.U(12.W)
  val mvendoridAddr = 0xF11.U(12.W)
  val marchidAddr   = 0xF12.U(12.W)
}

class Mstatus extends Bundle {
  val SD   = UInt(1.W)
  val Res5 = UInt(8.W)
  val TSR  = UInt(1.W)
  val TW   = UInt(1.W)
  val TVM  = UInt(1.W)
  val MXR  = UInt(1.W)
  val SUM  = UInt(1.W)
  val MPRV = UInt(1.W)
  val XS   = UInt(2.W)
  val FS   = UInt(2.W)
  val MPP  = UInt(2.W)
  val Res4 = UInt(2.W)
  val SPP  = UInt(1.W)
  val MPIE = UInt(1.W)
  val Res3 = UInt(1.W)
  val SPIE = UInt(1.W)
  val Res2 = UInt(1.W)
  val MIE  = UInt(1.W)
  val Res1 = UInt(1.W)
  val SIE  = UInt(1.W)
  val Res0 = UInt(1.W)
}

// class Csrs extends Module {
//   import Csrs._

//   val io = IO(new Bundle {
//     val in = Flipped(new WbuToCsrsPort)
//     val out = new CsrsToIduPort
//   })

//   val mstatusInit = WireDefault(0.U.asTypeOf(new Mstatus))
//   mstatusInit.MPP := 0b11.U
//   val mstatus     = RegInit(mstatusInit)

//   val mtvec       = RegInit(0.U(32.W))
//   val mepc        = RegInit(0.U(32.W))
//   val mcause      = RegInit(0.U(32.W))
//   val mcycle      = RegInit(0.U(32.W))
//   val mcycleh     = RegInit(0.U(32.W))
//   val mvendorid   = RegInit(0x79737978.U(32.W))
//   val marchid     = RegInit(26060187.U(32.W))

//   when (io.in.wEn) {
//     when (io.in.isEcall) {
//       mstatus.MPIE := mstatus.MIE
//       mstatus.MIE  := 0.U(1.W)
//       mstatus.MPP  := 0b11.U
//     } .elsewhen (io.in.isMret) {
//       mstatus.MIE  := mstatus.MPIE
//       mstatus.MPIE := 1.U(1.W)
//       mstatus.MPP  := 0.U(2.W)
//     } .elsewhen (io.in.wAddr === mstatusAddr) {
//       mstatus := io.in.wData.asTypeOf(new Mstatus) 
//     }
//   }

//   when (io.in.wEn && io.in.wAddr === mtvecAddr) {
//     mtvec := io.in.wData
//   }

//   when (io.in.wEn && (io.in.isEcall || io.in.wAddr === mepcAddr)) {
//     mepc := io.in.wData
//   }

//   when (io.in.wEn) {
//     when (io.in.isEcall) {
//       mcause := 0xB.U(32.W)
//     }.elsewhen (io.in.wAddr === mcauseAddr) {
//       mcause := io.in.wData
//     }
//   }

//   when (io.in.wEn && io.in.wAddr === mcycleAddr) {
//     mcycle := io.in.wData
//   }

//   when (io.in.wEn && io.in.wAddr === mcyclehAddr) {
//     mcycleh := io.in.wData
//   }

//   io.out.rData    := MuxLookup(io.out.rAddr, 0.U(32.W))(Seq(
//     mstatusAddr   -> mstatus.asTypeOf(UInt(32.W)),
//     mtvecAddr     -> mtvec,
//     mepcAddr      -> mepc,
//     mcauseAddr    -> mcause,
//     mcycleAddr    -> mcycle,
//     mcyclehAddr   -> mcycleh,
//     mvendoridAddr -> mvendorid,
//     marchidAddr   -> marchid
//   ))

//   io.out.mtvec := mtvec
//   io.out.mepc := mepc
// }

class Csrs extends Module {
  import Csrs._

  val io = IO(new Bundle {
    val in = Flipped(new WbuToCsrsPort)
    val out = new CsrsToIduPort
  })

  val mstatus     = RegInit(0x1800.U(32.W))

  val mtvec       = RegInit(0.U(32.W))
  val mepc        = RegInit(0.U(32.W))
  val mcause      = RegInit(0.U(32.W))
  val mcycle      = RegInit(0.U(32.W))
  val mcycleh     = RegInit(0.U(32.W))
  val mvendorid   = RegInit(0x79737978.U(32.W))
  val marchid     = RegInit(26060187.U(32.W))

  when (io.in.wEn) {
    when (io.in.isEcall) {
      val mstatusNext = WireInit(mstatus.asTypeOf(new Mstatus))
      mstatusNext.MPIE := mstatus.asTypeOf(new Mstatus).MIE
      mstatusNext.MIE  := 0.U(1.W)
      mstatusNext.MPP  := 0b11.U
      mstatus := mstatusNext.asTypeOf(UInt(32.W))
    } .elsewhen (io.in.isMret) {
      val mstatusNext = WireInit(mstatus.asTypeOf(new Mstatus))
      mstatusNext.MIE  := mstatus.asTypeOf(new Mstatus).MPIE
      mstatusNext.MPIE := 1.U(1.W)
      mstatusNext.MPP  := 0.U(2.W)
      mstatus := mstatusNext.asTypeOf(UInt(32.W))
    } .elsewhen (io.in.wAddr === mstatusAddr) {
      mstatus := io.in.wData
    }
  }

  when (io.in.wEn && io.in.wAddr === mtvecAddr) {
    mtvec := io.in.wData
  }

  when (io.in.wEn && (io.in.isEcall || io.in.wAddr === mepcAddr)) {
    mepc := io.in.wData
  }

  when (io.in.wEn) {
    when (io.in.isEcall) {
      mcause := 0xB.U(32.W)
    }.elsewhen (io.in.wAddr === mcauseAddr) {
      mcause := io.in.wData
    }
  }

  when (io.in.wEn && io.in.wAddr === mcycleAddr) {
    mcycle := io.in.wData
  }

  when (io.in.wEn && io.in.wAddr === mcyclehAddr) {
    mcycleh := io.in.wData
  }

  io.out.rData    := MuxLookup(io.out.rAddr, 0.U(32.W))(Seq(
    mstatusAddr   -> mstatus.asTypeOf(UInt(32.W)),
    mtvecAddr     -> mtvec,
    mepcAddr      -> mepc,
    mcauseAddr    -> mcause,
    mcycleAddr    -> mcycle,
    mcyclehAddr   -> mcycleh,
    mvendoridAddr -> mvendorid,
    marchidAddr   -> marchid
  ))

  io.out.mtvec := mtvec
  io.out.mepc := mepc
}