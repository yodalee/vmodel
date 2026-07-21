// Simulation driver for VTop (Plus1 → ForEach → Repeat)
//
// Feeds inputs 1 and 3, verifies the output sequence:
//   N=1: Plus1->2, ForEach->[1,2], Repeat->[1, 2,2]        => [1,2,2]
//   N=3: Plus1->4, ForEach->[1,2,3,4], Repeat->[1,2,2,3,3,3,4,4,4,4] => [1,2,2,3,3,3,4,4,4,4]
//
// Waveform dumped to cpp.fst

#include "VTop.h"
#include "vmodel/vmodel.h"

#include <cassert>
#include <cstdio>
#include <vector>

using Channel = vmodel::ValidReady<uint8_t>;

class DUT : public vmodel::IModule {
public:
    DUT(int argc, char** argv, Channel& in_ch, Channel& out_ch)
        : top_(argc, argv, "cpp.fst"),
          in_ch_(in_ch),
          out_ch_(out_ch) {}

    void Reset() {
        top_->clk = 0;
        top_->i_valid = 0;
        top_->i_data = 0;
        top_->o_ready = 0;
        top_.Reset();
    }

    void Comb() {
        // Drive DUT input ports from source channel.
        bool can_read = in_ch_.can_read();
        top_->i_valid = can_read;
        if (can_read) {
            top_->i_data = in_ch_.peek();
        }

        // Drive the DUT output ports from sink channel
        bool can_write = out_ch_.can_write();
        top_->o_ready = can_write;

        top_.Comb();

        // If input port has done a transaction, read the data.
        if (top_->i_ready && can_read) {
            in_ch_.read();
        }

        // If output port has done a transaction, write the data
        if (top_->o_valid && can_write) {
            out_ch_.write(top_->o_data);
        }
    }

    void Seq() override {
        top_.Seq();
    }

private:
    VMod<VTop> top_;
    Channel& in_ch_;
    Channel& out_ch_;
};

int main(int argc, char** argv) {
    // Input stream and expected output sequence.
    const std::vector<uint8_t> inputs   = {3, 1};
    const std::vector<uint8_t> expected = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 1, 2, 2};

    vmodel::SimGraph graph;
    Channel& source_to_dut = graph.CreateChannel<Channel>("source_to_dut");
    Channel& dut_to_sink = graph.CreateChannel<Channel>("dut_to_sink");

    DUT dut(argc, argv, source_to_dut, dut_to_sink);
    Source<uint8_t> source(source_to_dut, inputs);
    Sink<uint8_t> sink(dut_to_sink, expected);

    graph.AddModule(&source);
    graph.AddModule(&dut);
    graph.AddModule(&sink);
    graph.Connect(source_to_dut, &source, &dut);
    graph.Connect(dut_to_sink, &dut, &sink);
    graph.Compile();

    dut_to_sink.ready = true;
    dut.Reset();
    source.Reset();

    const int MAX_CYCLES = 2000;
    for (int cycle = 0; cycle < MAX_CYCLES && !sink.Done(); ++cycle) {
        graph.Comb();
        // Sequential phase: order can be changed safely.
        graph.Seq();
    }

    if (sink.Done()) {
        printf("\nPASS: all %d outputs matched.\n", sink.OutputCount());
    } else {
        printf("\nFAIL: received %d/%zu outputs before timeout.\n",
               sink.OutputCount(), expected.size());
    }

    return sink.Done() ? 0 : 1;
}
