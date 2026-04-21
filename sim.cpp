// Simulation driver for VTop (Plus1 → ForEach → Repeat)
//
// Feeds inputs 1 and 3, verifies the output sequence:
//   N=1: Plus1->2, ForEach->[1,2], Repeat->[1, 2,2]        => [1,2,2]
//   N=3: Plus1->4, ForEach->[1,2,3,4], Repeat->[1,2,2,3,3,3,4,4,4,4] => [1,2,2,3,3,3,4,4,4,4]
//
// Waveform dumped to cpp.fst

#include "VTop.h"
#include "verilated_fst_c.h"

#include <cassert>
#include <cstdio>
#include <vector>

int main(int argc, char** argv) {
    VerilatedContext* ctx = new VerilatedContext;
    ctx->commandArgs(argc, argv);
    ctx->traceEverOn(true);

    VTop* top = new VTop(ctx);

    VerilatedFstC* tfp = new VerilatedFstC;
    top->trace(tfp, 99);
    tfp->open("cpp.fst");

    uint64_t sim_time = 0;

    // Advance one clock cycle (posedge then negedge)
    auto tick = [&]() {
        top->clk = 1;
        top->eval();
        tfp->dump(sim_time++);
        top->clk = 0;
        top->eval();
        tfp->dump(sim_time++);
    };

    // Initialize signals
    top->clk    = 0;
    top->rst_n  = 0;
    top->i_valid = 0;
    top->i_data  = 0;
    top->o_ready = 1;   // always ready to accept output
    top->eval();
    tfp->dump(sim_time++);

    // Hold reset for 5 cycles
    for (int i = 0; i < 5; i++) tick();
    top->rst_n = 1;
    for (int i = 0; i < 5; i++) tick();

    // Input stream and expected output sequence
    // N=1: [1, 2, 2]
    // N=3: [1, 2, 2, 3, 3, 3, 4, 4, 4, 4]
    const std::vector<uint8_t> inputs   = {3, 1};
    const std::vector<uint8_t> expected = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 1, 2, 2};

    int input_idx  = 0;
    int output_idx = 0;

    // Present first input
    top->i_valid = 1;
    top->i_data  = inputs[0];

    const int MAX_CYCLES = 2000;
    for (int cycle = 0; cycle < MAX_CYCLES && output_idx < (int)expected.size(); ++cycle) {
        // Settle combinational logic with current inputs
        top->eval();

        // Sample handshakes — these are what the RTL sees at the upcoming posedge
        bool i_xfer = top->i_valid && top->i_ready;
        bool o_xfer = top->o_valid && top->o_ready;

        if (o_xfer) {
            uint8_t got = top->o_data;
            printf("[cycle %4d] output[%2d]: got=%3u  expected=%3u  %s\n",
                   cycle, output_idx, got, expected[output_idx],
                   got == expected[output_idx] ? "OK" : "MISMATCH");
            assert(got == expected[output_idx]);
            output_idx++;
        }

        // Clock edge: registers capture current inputs and state
        top->clk = 1; top->eval(); tfp->dump(sim_time++);
        top->clk = 0; top->eval(); tfp->dump(sim_time++);

        // Update inputs for the NEXT cycle after the clock edge
        if (i_xfer) {
            input_idx++;
            if (input_idx < (int)inputs.size()) {
                top->i_data  = inputs[input_idx];
                top->i_valid = 1;
            } else {
                top->i_valid = 0;
                top->i_data  = 0;
            }
        }
    }

    if (output_idx == (int)expected.size()) {
        printf("\nPASS: all %d outputs matched.\n", output_idx);
    } else {
        printf("\nFAIL: received %d/%zu outputs before timeout.\n",
               output_idx, expected.size());
    }

    tfp->close();
    top->final();
    delete top;
    delete tfp;
    delete ctx;
    return output_idx == (int)expected.size() ? 0 : 1;
}
