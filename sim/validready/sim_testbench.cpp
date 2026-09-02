// Minimal C++ main for the Verilog testbench (tb_top).
// All stimulus and checking live in tb_top.sv; this file just runs the
// event loop and records the FST waveform to tb.fst.

#include "Vtb_top.h"
#include "verilated_fst_c.h"

int main(int argc, char** argv) {
    VerilatedContext* ctx = new VerilatedContext;
    ctx->commandArgs(argc, argv);
    ctx->traceEverOn(true);

    Vtb_top* top = new Vtb_top(ctx);

    VerilatedFstC* tfp = new VerilatedFstC;
    top->trace(tfp, 99);
    tfp->open("tb.fst");

    // Event-driven loop: eval at current time, then advance to next event
    while (!ctx->gotFinish()) {
        top->eval();
        tfp->dump(ctx->time());
        if (!top->eventsPending()) break;
        ctx->timeInc(top->nextTimeSlot() - ctx->time());
    }

    top->final();
    tfp->close();
    delete top;
    delete tfp;
    delete ctx;
    return 0;
}
