import chisel3._
import chisel3.util._
import common._

// 负责根据IDU给出的指令，把这个指令的信息提取出来，送给EXU和LSU
class Idu extends Module {
  // 指令输入
  val inst            = IO(Input(UInt(32.W)))
  // 寄存器读取   
  val regsRaddr1      = IO(Output(UInt(4.W)))
  val regsRdata1      = IO(Input(UInt(32.W)))
  val regsRaddr2      = IO(Output(UInt(4.W)))
  val regsRdata2      = IO(Input(UInt(32.W)))
  // 寄存器写入   
  val regsWen         = IO(Output(Bool()))
  val regsWaddr       = IO(Output(UInt(4.W)))
  val regsWdata       = IO(Output(UInt(32.W)))
  // pc输入   
  val pc              = IO(Input(UInt(32.W)))
  val pcEn            = IO(Output(Bool()))
  // 内存读写控制
  val cpuReadEn2      = IO(Output(Bool()))
  val cpuReadLen2     = IO(Output(UInt(2.W)))
  val cpuReadSign2    = IO(Output(Bool()))
  val cpuWriteData    = IO(Output(UInt(32.W)))
  val cpuWriteEn      = IO(Output(Bool()))
  val cpuWriteLen     = IO(Output(UInt(2.W)))
  // WBU输入选择    
  val wbuChoose       = IO(Output(UInt(2.W)))
  // ALU数据控制    
  val aluOprand1      = IO(Output(UInt(32.W)))
  val aluOprand2      = IO(Output(UInt(32.W)))
  val aluOpcode       = IO(Output(UInt(4.W)))
  val pcOprand1       = IO(Output(UInt(32.W)))
  val pcOprand2       = IO(Output(UInt(32.W)))
  val isBranch        = IO(Output(Bool()))
  val isUncondjmp     = IO(Output(Bool()))
  // 特殊控制信号
  val isEbreak        = IO(Output(Bool()))
  val isInv           = IO(Output(Bool()))
    
    
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
  isEbreak            := BitPat("b0000000_00001_00000_000_00000_11100_11") === inst
  isInv               := !(isLui || isAuipc || isAdd ||isAddi || isSub || isAnd ||
                         isAndi || isOr || isOri  || isXor  || isXori || isSll ||
                         isSlli || isSrl  || isSrli || isSra  || isSrai || isSlt ||
                         isSlti || isSltu || isSltiu|| isLb || isLbu || isLh ||
                         isLhu || isLw || isSb || isSh || isSw || isBeq  || isBne ||
                         isBlt || isBge || isBltu || isBgeu || isJal  || isJalr || isEbreak)
  // 寄存器读取逻辑
  val rs1             = inst(19,15)
  val rs2             = inst(24,20)
  val src1            = regsRdata1
  regsRaddr1          := rs1
  val src2            = regsRdata2
  regsRaddr2          := rs2
  // pc使能
  pcEn                := true.B
  cpuWriteData        := src2
  
  // 立即数提取逻辑
  val immI            = inst(31,20).asSInt.pad(32).asUInt
  val immU            = inst(31,12) << 12
  val immS            = Cat(inst(31,25), inst(11,7)).asSInt.pad(32).asUInt
  val immJ            = Cat(inst(31), inst(19,12), inst(20), inst(30,21), 0.U(1.W)).asSInt.pad(32).asUInt
  val immB            = Cat(inst(31), inst(7), inst(30,25), inst(11,8), 0.U(1.W)).asSInt.pad(32).asUInt

  // 指令类型判断逻辑
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
  // 写寄存器逻辑
  regsWaddr           := inst(10,7)
  regsWdata           := Mux(isLui, imm, 0.U(32.W))
  regsWen             := Mux(typeS || typeB || isEbreak || isInv, false.B, true.B)
  // alu控制信号逻辑
  val needAdd         = isAdd   || isAddi || isAuipc || isJal || isJalr ||
                        isLb    || isLbu  || isLh    || isLhu || isLw   ||
                        isSb    || isSh   || isSw
  val needSub         = isSub 
  val needAnd         = isAnd   || isAndi 
  val needOr          = isOr    || isOri
  val needXor         = isXor   || isXori
  val needEqu         = isBeq 
  val needNeq         = isBne 
  val needLargeOrEquS = isBge 
  val needLargeOrEquU = isBgeu  
  val needSmallS      = isBlt   || isSlt  || isSlti
  val needSmallU      = isBltu  || isSltu || isSltiu
  val needLeftShiftU  = isSll   || isSlli
  val needRightShiftS = isSra   || isSrai
  val needRightShiftU = isSrl   || isSrli
//内存读写使能
  cpuReadEn2          := isLb || isLbu || isLh || isLhu || isLw
  cpuReadLen2         := Mux1H(Seq(
    isLb              -> RamLen.one,
    isLbu             -> RamLen.one,
    isLh              -> RamLen.two,
    isLhu             -> RamLen.two,
    isLw              -> RamLen.four
  ))
  cpuReadSign2        := Mux(isLb || isLh, true.B, false.B)
  cpuWriteEn          := typeS
  cpuWriteLen         := Mux1H(Seq(
    isSb              -> RamLen.one,
    isSh              -> RamLen.two,
    isSw              -> RamLen.four
  ))
// alu控制信号选择
  aluOpcode           := Mux1H(Seq(
    needAdd           -> AluOpcode.add,
    needSub           -> AluOpcode.sub,
    needAnd           -> AluOpcode.and,
    needOr            -> AluOpcode.or,
    needXor           -> AluOpcode.xor,
    needEqu           -> AluOpcode.equ,
    needNeq           -> AluOpcode.neq,
    needLargeOrEquS   -> AluOpcode.largeOrEquS,
    needLargeOrEquU   -> AluOpcode.largeOrEquU,
    needSmallS        -> AluOpcode.smallS,
    needSmallU        -> AluOpcode.smallU,
    needLeftShiftU    -> AluOpcode.leftShiftU,
    needRightShiftS   -> AluOpcode.rightShiftS,
    needRightShiftU   -> AluOpcode.rightShiftU
  ))
  // alu操作数选择逻辑
  aluOprand1          := Mux(isAuipc || isJal || isJalr, pc, src1)
  aluOprand2          := MuxCase(imm, Seq(
    typeB             -> src2,
    typeR             -> src2,
    typeJ             -> "d4".U(32.W),
    isJalr            -> "d4".U(32.W)
  ))
  // pc计算逻辑
  pcOprand1           := Mux(isJalr, src1, pc)
  pcOprand2           := imm
  // 是否为条件分支
  isBranch            := typeB
  // 是否为无条件分支
  isUncondjmp         := isJal || isJalr
  // WBU输入选择
  wbuChoose           := MuxCase(WbuChoose.exu, Seq(
    isLui             -> WbuChoose.idu,
    cpuReadEn2        -> WbuChoose.lsu
  ))

}
