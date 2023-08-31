#include <verilated.h>
#include <verilated_fst_c.h>
#include <VASCONXOF.h>

#include <crypto_hash.h>

#include<limits>
#include<iostream>
#include<random>

void clock(VASCONXOF *dut, VerilatedFstC* tfp){
  static uint64_t time_counter = 0;
  dut->eval();
  tfp->dump(1000*time_counter);
  time_counter++;
  dut->clock = !dut->clock;
  dut->eval();
  tfp->dump(1000*time_counter);
  time_counter++;
  dut->clock = !dut->clock;
}

int main(int argc, char** argv) {
    //generatros
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
    std::uniform_int_distribution<uint64_t> inputgen(0, std::numeric_limits<uint64_t>::max());

    constexpr uint inputbyte = 40;
    constexpr uint wordbyte = 8;
    constexpr uint inputword = inputbyte/wordbyte;
    std::array<uint64_t,inputword> input;
    for(int i = 0; i < inputword; i++){
        input[i] = inputgen(engine);
    }
    constexpr uint crypto_byte = 32;
    constexpr uint outword = crypto_byte/wordbyte;

    std::array<uint64_t,outword> trueout,circout;

    crypto_xof(&trueout[0], &input[0], inputbyte);

    Verilated::commandArgs(argc, argv);
    VASCONXOF *dut = new VASCONXOF();

    Verilated::traceEverOn(true);
    VerilatedFstC* tfp = new VerilatedFstC;
    dut->trace(tfp, 100);  // Trace 100 levels of hierarchy
    tfp->open("simx.fst");

    // Format
    dut->reset = 1;
    dut->clock = 0;
    dut->io_in_TVALID = 0;
    dut->io_in_TDATA = 0;
    dut->io_in_TLAST= 0;
    dut->io_out_TREADY = 0;
    dut->io_flush = 0;

    // Reset
    clock(dut, tfp);

    //Release reset
    dut->reset = 0;
    dut->io_in_TDATA = input[0];

    clock(dut, tfp);

    for(int i = 0; i < inputword; i++){
        int watchdog = 0;
        dut->io_in_TVALID = 1;
        dut->io_in_TDATA = input[i];
        dut->eval();
        while(dut->io_in_TREADY==0){
            clock(dut, tfp);
            watchdog++;
            if(watchdog>100){
                dut->final();
                tfp->close(); 
                exit(1);
            }
        }
        clock(dut,tfp);
    }
    dut->io_in_TVALID = 1;
    dut->io_in_TLAST = 1;
    dut->io_in_TDATA = 1ULL<<63;
    dut->eval();
    {
    int watchdog = 0;
    while(dut->io_in_TREADY==0){
        clock(dut, tfp);
        watchdog++;
        if(watchdog>100){
            dut->final();
            tfp->close(); 
            exit(1);
        }
    }
    }
    clock(dut,tfp);
    dut->io_in_TVALID = 0;
    dut->io_in_TLAST = 0;

    for(int i = 0; i < outword; i++){
        int watchdog = 0;
        dut->io_out_TREADY = 1;
        while(dut->io_out_TVALID==0){
            clock(dut, tfp);
            watchdog++;
            if(watchdog>1000){
                dut->final();
                tfp->close(); 
                exit(1);
            }
        }
        circout[i] = dut->io_out_TDATA;
        clock(dut,tfp);
    }

    for(int i = 0; i < outword; i++){
        if(trueout[i] != circout[i]){
            std::cout<<"Output ERROR"<<std::endl;
            std::cout<<i<<":"<<std::hex<<trueout[i]<<":"<<circout[i]<<std::endl;
            dut->final();
            tfp->close(); 
            exit(1);
        }
    }
    std::cout<<"PASS"<<std::endl;
}