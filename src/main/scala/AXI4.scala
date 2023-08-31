import chisel3._
import chisel3.util._

class AXI4StreamManager(val buswidth: Int, val hasLast: Boolean = false) extends Bundle{
	val TVALID = Output(Bool())
	val TREADY = Input(Bool())
	val TDATA = Output(UInt(buswidth.W))
    val TLAST = if (hasLast) Some(Output(Bool())) else None
}

class AXI4StreamSubordinate(val buswidth: Int, val hasLast: Boolean = false) extends Bundle{
	val TVALID = Input(Bool())
	val TREADY = Output(Bool())
	val TDATA = Input(UInt(buswidth.W))
    val TLAST = if (hasLast) Some(Input(Bool())) else None
}