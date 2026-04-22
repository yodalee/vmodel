#pragma once

#include "verilated.h"
#include "verilated_fst_c.h"

#include <utility>

template <typename Top>
class VMod {
public:
    VMod(int argc, char** argv, const char* trace_path)
        : ctx_(new VerilatedContext()), top_(new Top(ctx_)), tfp_(new VerilatedFstC()) {
        ctx_->commandArgs(argc, argv);
        ctx_->traceEverOn(true);

        top_->trace(tfp_, 99);
        tfp_->open(trace_path);
    }

    ~VMod() {
        if (top_ != nullptr) {
            top_->final();
        }
        if (tfp_ != nullptr) {
            tfp_->close();
        }
        delete tfp_;
        delete top_;
        delete ctx_;
    }

    VMod(const VMod&) = delete;
    VMod& operator=(const VMod&) = delete;
    VMod(VMod&&) = delete;
    VMod& operator=(VMod&&) = delete;

    Top* operator->() { return top_; }
    const Top* operator->() const { return top_; }

    Top& dut() { return *top_; }
    const Top& dut() const { return *top_; }

    void Comb() {
        top_->eval();
    }

    void Seq() {
        top_->clk = 1;
        top_->eval();
        tfp_->dump(sim_time_++);

        top_->clk = 0;
        top_->eval();
        tfp_->dump(sim_time_++);
    }

    void Reset(int cycles = 5) {
        top_->clk = 0;
        top_->rst_n = 0;
        top_->eval();
        tfp_->dump(sim_time_++);

        for (int cycle = 0; cycle < cycles; ++cycle) {
            Seq();
        }

        top_->rst_n = 1;

        for (int cycle = 0; cycle < cycles; ++cycle) {
            Seq();
        }
    }

private:
    VerilatedContext* ctx_ = nullptr;
    Top* top_ = nullptr;
    VerilatedFstC* tfp_ = nullptr;
    vluint64_t sim_time_ = 0;
};