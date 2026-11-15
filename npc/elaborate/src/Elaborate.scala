package elaborate
object Elaborate {
  def main(args: Array[String]): Unit = {
    val firtoolOptions = Array(
      "--default-layer-specialization=enable",
      "--verification-flavor=immediate",
      "--lowering-options=" + List(
        // make yosys happy
        // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
        "disallowLocalVariables",
        "disallowPackedArrays",
        "locationInfoStyle=wrapInAtSquareBracket"
      ).reduce(_ + "," + _)
    )
    circt.stage.ChiselStage.emitSystemVerilogFile(new top(), args, firtoolOptions)
  }
  
}
