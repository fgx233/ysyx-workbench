import chisel3._
import chisel3.util._

/* 
功能：接收ifu取出来的指令，根据这条指令，给出指导ISA状态机跳转的次态信息，把这些信息打包传给exu
状态：
1、等待valid
2、等待ready
 */

object Idu {
  object State extends ChiselEnum {
    val sWaitValid, sWaitReady = Value
  }
}

class Idu extends Module {
  import Idu.State._

  val io = IO(new Bundle {
    val pc = Input(UInt(32.W))
    val regs = Flipped(new RegsToIduPort)
    val csrs = Flipped(new CsrsToIduPort)
    val in = Flipped(Decoupled(new IfuToIduPort))
    val out = Decoupled(new IduToExuPort)
    val monitor = new IduToMonitorPort
  })

  val inst = Reg(UInt(32.W))
  val state = RegInit(sWaitValid)

    // 译码逻辑   
  val isLui           = BitPat("b???????_?????_?????_???_?????_01101_11") === inst
  val isAuipc         = BitPat("b???????_?????_?????_???_?????_00101_11") === inst
  val isAdd           = BitPat("b0000000_?????_?????_000_?????_01100_11") === inst
  val isAddi          = BitPat("b???????_?????_?????_000_?????_00100_11") === inst
  val isSub           = BitPat("b0100000_?????_?????_000_?????_01100_11") === inst
  val isAnd           = BitPat("b0000000_?????_?????_111_?????_01100_11") === inst
  val isAndi          = BitPat("b???????_?????_?????_111_?????_00100_11") === inst
  val isOr            = BitPat("b0000000_?????_?????_110_?????_01100_11") === inst
  val isOri           = BitPat("b???????_?????_?????_110_?????_00100_11") === inst
  val isXor           = BitPat("b0000000_?????_?????_100_?????_01100_11") === inst
  val isXori          = BitPat("b???????_?????_?????_100_?????_00100_11") === inst
  val isSll           = BitPat("b0000000_?????_?????_001_?????_01100_11") === inst
  val isSlli          = BitPat("b0000000_?????_?????_001_?????_00100_11") === inst
  val isSrl           = BitPat("b0000000_?????_?????_101_?????_01100_11") === inst
  val isSrli          = BitPat("b0000000_?????_?????_101_?????_00100_11") === inst
  val isSra           = BitPat("b0100000_?????_?????_101_?????_01100_11") === inst
  val isSrai          = BitPat("b0100000_?????_?????_101_?????_00100_11") === inst
  val isSlt           = BitPat("b0000000_?????_?????_010_?????_01100_11") === inst
  val isSlti          = BitPat("b???????_?????_?????_010_?????_00100_11") === inst
  val isSltu          = BitPat("b0000000_?????_?????_011_?????_01100_11") === inst
  val isSltiu         = BitPat("b???????_?????_?????_011_?????_00100_11") === inst
  val isLb            = BitPat("b???????_?????_?????_000_?????_00000_11") === inst
  val isLbu           = BitPat("b???????_?????_?????_100_?????_00000_11") === inst
  val isLh            = BitPat("b???????_?????_?????_001_?????_00000_11") === inst
  val isLhu           = BitPat("b???????_?????_?????_101_?????_00000_11") === inst
  val isLw            = BitPat("b???????_?????_?????_010_?????_00000_11") === inst
  val isSb            = BitPat("b???????_?????_?????_000_?????_01000_11") === inst
  val isSh            = BitPat("b???????_?????_?????_001_?????_01000_11") === inst
  val isSw            = BitPat("b???????_?????_?????_010_?????_01000_11") === inst
  val isBeq           = BitPat("b???????_?????_?????_000_?????_11000_11") === inst
  val isBne           = BitPat("b???????_?????_?????_001_?????_11000_11") === inst
  val isBlt           = BitPat("b???????_?????_?????_100_?????_11000_11") === inst
  val isBge           = BitPat("b???????_?????_?????_101_?????_11000_11") === inst
  val isBltu          = BitPat("b???????_?????_?????_110_?????_11000_11") === inst
  val isBgeu          = BitPat("b???????_?????_?????_111_?????_11000_11") === inst
  val isJal           = BitPat("b???????_?????_?????_???_?????_11011_11") === inst
  val isJalr          = BitPat("b???????_?????_?????_000_?????_11001_11") === inst
  val isCsrrw         = BitPat("b???????_?????_?????_001_?????_11100_11") === inst
  val isCsrrs         = BitPat("b???????_?????_?????_010_?????_11100_11") === inst
  val isEcall         = BitPat("b0000000_00000_00000_000_00000_11100_11") === inst
  val isMret          = BitPat("b0011000_00010_00000_000_00000_11100_11") === inst
  val isEbreak        = BitPat("b0000000_00001_00000_000_00000_11100_11") === inst
  val isInvalid       = !(isLui || isAuipc || isAdd ||isAddi || isSub || isAnd ||
                         isAndi || isOr || isOri  || isXor  || isXori || isSll ||
                         isSlli || isSrl  || isSrli || isSra  || isSrai || isSlt ||
                         isSlti || isSltu || isSltiu|| isLb || isLbu || isLh ||
                         isLhu || isLw || isSb || isSh || isSw || isBeq  || isBne ||
                         isBlt || isBge || isBltu || isBgeu || isJal  || isJalr || isEbreak ||
                         isCsrrs || isCsrrw || isEcall || isMret)

  val immI            = inst(31,20).asSInt.pad(32).asUInt
  val immU            = inst(31,12) << 12
  val immS            = Cat(inst(31,25), inst(11,7)).asSInt.pad(32).asUInt
  val immJ            = Cat(inst(31), inst(19,12), inst(20), inst(30,21), 0.U(1.W)).asSInt.pad(32).asUInt
  val immB            = Cat(inst(31), inst(7), inst(30,25), inst(11,8), 0.U(1.W)).asSInt.pad(32).asUInt

  val typeI           = isAddi  || isAndi   || isJalr || isLb   || isLbu  ||
                        isLh    || isLhu    || isLw   || isOri  || isSlli ||
                        isSlti  || isSltiu  || isSrai || isSrli || isXori
  val typeR           = isAdd   || isAnd    || isOr   || isSll  || isSlt  ||
                        isSltu  || isSra    || isSrl  || isSub  || isXor
  val typeU           = isLui   || isAuipc  
  val typeB           = isBeq   || isBge    || isBgeu || isBlt  || isBltu ||
                        isBne
  val typeS           = isSb    || isSh     || isSw 
  val typeJ           = isJal
  // 立即数判断逻辑
  val imm             = Mux1H(Seq(
    typeI             -> immI,
    typeJ             -> immJ,
    typeS             -> immS,
    typeB             -> immB,
    typeU             -> immU
  ))

  val rAddr1          = inst(18,15)
  val rAddr2          = inst(23,20)
  io.regs.rAddr1      := rAddr1
  io.regs.rAddr2      := rAddr2

  val rAddrCsr        = immI
  io.csrs.rAddr       := rAddrCsr


//exuAction
  val onlyTransfer    = isLui || isBeq || isBne || isBlt || isCsrrw ||
                        isBge || isBltu || isBgeu
  val computeReg      = isAuipc || isAdd || isAddi || isSub || isAnd || isAndi ||
                        isOr || isOri || isXor || isXori || isSll || isSlli ||
                        isSrl || isSrli || isSra || isSrai || isSlt || isSlti ||
                        isSltu || isSltiu || isJal || isJalr
  val computeCsr      = isCsrrs
  val computeAddr     = isLb || isLbu || isLh || isLhu ||
                        isLw || isSb || isSh || isSw
  io.out.bits.exuAction := Mux1H(Seq(
    onlyTransfer        -> ExuAction.onlyTransfer,
    computeReg          -> ExuAction.computeReg,
    computeCsr          -> ExuAction.computeCsr,
    computeAddr         -> ExuAction.computeAddr
  ))

  //aluOption
  val add             = isAuipc || isAdd || isAddi || isLb || isLbu || isLh ||
                        isLhu || isLw || isSb || isSh || isSw || isJal ||
                        isJalr
  val sub             = isSub
  val and             = isAnd || isAndi
  val or              = isOr || isOri || isCsrrs
  val xor             = isXor || isXori
  val llshift         = isSll || isSlli
  val lrshift         = isSrl || isSrli
  val arshift         = isSra || isSrai
  val sless           = isSlt || isSlti
  val uless           = isSltu || isSltiu
  io.out.bits.aluOption := MuxCase(AluOption.add ,Seq(
    add                 -> AluOption.add,
    sub                 -> AluOption.sub,
    and                 -> AluOption.and,
    or                  -> AluOption.or,
    xor                 -> AluOption.xor,
    llshift             -> AluOption.llshift,
    lrshift             -> AluOption.lrshift,
    arshift             -> AluOption.arshift,
    sless               -> AluOption.sless,
    uless               -> AluOption.uless
  ))

  //aluOprand1
  /* 
  1.pc
  2.src1
   */
  val aluOprand1FromPc = isAuipc || isJal || isJalr
  val aluOprand1FromSrc1 = !aluOprand1FromPc
  io.out.bits.aluOprand1 := Mux1H(Seq(
    aluOprand1FromPc     -> io.pc,
    aluOprand1FromSrc1   -> io.regs.src1
  ))

  //aluOprand2
  /* 
  1.imm
  2.src2
  3.4
  4.csr
   */
  val aluOprand2FromImm = isAuipc || isAddi || isAndi || isOri || isXori || isSlli ||
                          isSrli  || isSrai || isSlti || isSltiu || isLb || isLbu ||
                          isLh || isLhu || isLw || isSb || isSh || isSw
  val aluOprand2FromFix = isJal || isJalr
  val aluOprand2FromCsr = isCsrrs
  val aluOprand2FromSrc2= !(aluOprand2FromImm || aluOprand2FromFix || aluOprand2FromCsr)
  io.out.bits.aluOprand2 := Mux1H(Seq(
    aluOprand2FromImm    -> imm,
    aluOprand2FromSrc2   -> io.regs.src2,
    aluOprand2FromFix    -> 4.U(32.W),
    aluOprand2FromCsr    -> io.csrs.rData
  ))
  

  //pcOprand1
  /* 
  1.PC
  2.src1
   */
  val pcOprand1FromSrc1   = isJalr 
  val pcOprand1FromPc     = !pcOprand1FromSrc1
  io.out.bits.pcOprand1   := Mux1H(Seq(
    pcOprand1FromSrc1     -> io.regs.src1,
    pcOprand1FromPc       -> io.pc
  ))

  //pcOprand2
  /* 
  1.4
  2.imm
   */
  val equ                 = io.regs.src1 === io.regs.src2
  val neq                 = io.regs.src1 =/= io.regs.src2
  val signedLess          = io.regs.src1.asSInt < io.regs.src2.asSInt
  val signedLarge         = io.regs.src1.asSInt >= io.regs.src2.asSInt
  val unsignedLess        = io.regs.src1 < io.regs.src2
  val unsignedLarge       = io.regs.src1 >= io.regs.src2
  val pcOprand2FromImm    = (isBeq && equ)|| 
                            (isBne  && neq)|| 
                            (isBlt  && signedLess)|| 
                            (isBge  && signedLarge)|| 
                            (isBltu && unsignedLess)|| 
                            (isBgeu && unsignedLarge)||
                            isJal  || isJalr
  val pcOprand2FromFix    = !pcOprand2FromImm
  io.out.bits.pcOprand2   := Mux1H(Seq(
    pcOprand2FromImm      -> imm,
    pcOprand2FromFix      -> 4.U(32.W)
  )) 

  //memAddr
  io.out.bits.memAddr     := 0.U(32.W)
  //memRdEn
  io.out.bits.memRdEn     := isLb || isLbu || isLh || isLhu || isLw
  //memWrEn
  io.out.bits.memWrEn     := isSb || isSh || isSw
  //memWrData
  io.out.bits.memWrData   := io.regs.src2
  //memLen
  val one                 = isLb || isLbu || isSb
  val two                 = isLh || isLhu || isSh
  val four                = isLw || isSw
  io.out.bits.memLen      := MuxCase(LsuByteLen.one, Seq(
    one                   -> LsuByteLen.one,
    two                   -> LsuByteLen.two,
    four                  -> LsuByteLen.four
  ))
  //memSign
  val signed              = isLb || isLh || isLw
  val unsigned            = isLbu || isLhu
  io.out.bits.memSign     := Mux1H(Seq(
    signed                -> LsuSign.signed,
    unsigned              -> LsuSign.unsigned
  ))

  //regsWaddr
  io.out.bits.regsWaddr   := inst(10,7)
  //regsWdata
  val regsWdataFromImm    = isLui
  val regsWdataFromCsr    = isCsrrw || isCsrrs
  io.out.bits.regsWdata   := Mux1H(Seq(
    regsWdataFromImm      -> imm,
    regsWdataFromCsr      -> io.csrs.rData
  ))
  //regsWen
  io.out.bits.regsWen     := !(isBeq || isBne || isBlt || isBge || isBltu || isBgeu ||
                               isSb  || isSh || isSw)

  //pcWdata
  io.out.bits.pcWdata     := Mux1H(Seq(
    isEcall               -> io.csrs.mtvec,
    isMret                -> io.csrs.mepc
  ))
  //pcWen
  io.out.bits.pcWen       := true.B

  //csrWaddr
  io.out.bits.csrWaddr    := inst(31,20)
  //csrWdata
  io.out.bits.csrWdata    := Mux(isEcall, io.pc, io.regs.src1)
  //csrWen
  io.out.bits.csrWen      := isCsrrs || isCsrrw || isEcall || isMret

  //isEcall,isMret
  io.out.bits.isEcall     := isEcall
  io.out.bits.isMret      := isMret

  //isEbreak,isInvalid
  io.monitor.pc           := io.pc
  io.monitor.isEbreak     := Mux(state === sWaitReady, isEbreak, false.B)
  io.monitor.isInvalid    := Mux(state === sWaitReady, isInvalid, false.B)

  when (io.in.valid && io.in.ready) {
    inst := io.in.bits.inst
  }

  io.in.ready             := state === sWaitValid
  io.out.valid            := state === sWaitReady
  switch (state) {
    is (sWaitValid) {
      when (io.in.valid && io.in.ready) {
        state := sWaitReady
      }
    }
    is (sWaitReady) {
      when (io.out.valid && io.out.ready) {
        state := sWaitValid
      }
    }
  }
}