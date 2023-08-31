import chisel3._
import chisel3.util._

object ASCONXOFState extends ChiselEnum {
  val Wait, Absorb, SqueezeInit, Squeeze, SqueezePermutaion  = Value
}

class ASCONXOF() extends Module{
    val io = IO(new Bundle{
        val in = new AXI4StreamSubordinate(64, true)
        val out = new AXI4StreamManager(64, false)
        val flush = Input(Bool())
    })
    val a = 12
    val b = 12
    val S = Reg(Vec(5,UInt(64.W)))

    io.in.TREADY := false.B
    io.out.TVALID := false.B
    io.out.TDATA := S(0)

    val cntreg = Reg(UInt(4.W))
    val permutation = Module(new ASCONPermuitation)
    permutation.io.in := S
    permutation.io.r := cntreg

    val statereg = RegInit(ASCONXOFState.Wait)
    switch(statereg){
        is(ASCONXOFState.Wait){
            S(0) := BigInt("b57e273b814cd416",16).U
            S(1) := BigInt("2b51042562ae2420",16).U
            S(2) := BigInt("66a3a7768ddf2218",16).U
            S(3) := BigInt("5aad0a7a8153650c",16).U
            S(4) := BigInt("4f3e0e32539493b6",16).U
            cntreg := 0.U
            when(io.in.TVALID){
                io.in.TREADY := true.B
                statereg := ASCONXOFState.Absorb
                permutation.io.in(0) := S(0) ^ io.in.TDATA
                permutation.io.r := cntreg
                S := permutation.io.out
                cntreg := cntreg + 1.U
            }
        }
        // is(ASCONXOFState.Absorb){
        //     when(io.in.TVALID){
        //         io.in.TREADY := true.B
        //         when(io.in.TLAST.get){
        //             S(0) := S(0) ^ io.in.TDATA
        //             statereg := ASCONXOFState.SqueezeInit
        //         }.otherwise{
        //             io.in.TREADY := true.B
        //             statereg := ASCONXOFState.AbsorbPermutation
        //             permutation.io.in(0) := S(0) ^ io.in.TDATA
        //             S := permutation.io.out
        //             cntreg := cntreg + 1.U
        //         }
        //     }   
        // }
        is(ASCONXOFState.Absorb){
            S := permutation.io.out
            cntreg := cntreg + 1.U
            when(cntreg === (b-1).U){
                cntreg := 0.U
                when(io.in.TVALID){
                    io.in.TREADY := true.B
                    S(0) := permutation.io.out(0) ^ io.in.TDATA
                    when(io.in.TLAST.get){
                        statereg := ASCONXOFState.SqueezeInit
                    }.otherwise{
                        statereg := ASCONXOFState.Absorb
                    }
                }
            }
        }
        is(ASCONXOFState.SqueezeInit){
            S := permutation.io.out
            cntreg := cntreg + 1.U
            when(cntreg === (a-1).U){
                cntreg := 0.U
                statereg := ASCONXOFState.Squeeze
            }
        }
        is(ASCONXOFState.Squeeze){
            io.out.TVALID := true.B
            when(io.out.TREADY){
                statereg := ASCONXOFState.SqueezePermutaion
                S := permutation.io.out
                cntreg := cntreg + 1.U
            }
        }
        is(ASCONXOFState.SqueezePermutaion){
            S := permutation.io.out
            cntreg := cntreg + 1.U
            when(cntreg === (b-1).U){
                cntreg := 0.U
                statereg := ASCONXOFState.Squeeze
            }
        }
    }
    when(io.flush){
        statereg := ASCONXOFState.Wait
        cntreg := 0.U
    }
}

object ASCONXOFTop extends App {
  (new chisel3.stage.ChiselStage).emitVerilog(new ASCONXOF, Array("--emission-options=disableMemRandomization,disableRegisterRandomization"))
}