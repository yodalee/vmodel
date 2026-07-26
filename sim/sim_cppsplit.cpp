// Simulation driver for split DUTs: VPlus1 -> VForEach -> VRepeat.
//
// Feeds inputs 3 and 1, verifies the final output sequence:
//   N=3: Plus1->4, ForEach->[1,2,3,4], Repeat->[1,2,2,3,3,3,4,4,4,4]
//   N=1: Plus1->2, ForEach->[1,2], Repeat->[1,2,2]

#include "VForEach.h"
#include "VPlus1.h"
#include "VRepeat.h"
#include "vmodel/vmodel.h"

#include <cstdio>
#include <vector>

using Channel = vmodel::ValidReady<uint8_t>;

template <typename Top>
class DUTStage : public vmodel::IModule {
public:
    DUTStage(int argc, char** argv, const char* trace_path, Channel& in_ch, Channel& out_ch)
        : top_(argc, argv, trace_path),
          in_ch_(in_ch),
          out_ch_(out_ch) {}

    void Reset() {
        top_->clk = 0;
        top_->i_valid = 0;
        top_->i_data = 0;
        top_->o_ready = 1;
        top_.Reset();
    }

    void Comb() override {
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
    VMod<Top> top_;
    Channel& in_ch_;
    Channel& out_ch_;
};

int main(int argc, char** argv) {
    // Input stream and expected output sequence.
    const std::vector<uint8_t> inputs   = {3, 1};
    const std::vector<uint8_t> expected = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 1, 2, 2};

    vmodel::SimGraph graph;
    Channel& source_to_plus1 = graph.CreateChannel<Channel>();
    Channel& plus1_to_foreach = graph.CreateChannel<Channel>();
    Channel& foreach_to_repeat = graph.CreateChannel<Channel>();
    Channel& repeat_to_sink = graph.CreateChannel<Channel>();

    DUTStage<VPlus1> plus1(argc, argv, "cpp_plus1.fst", source_to_plus1, plus1_to_foreach);
    DUTStage<VForEach> foreach(argc, argv, "cpp_foreach.fst", plus1_to_foreach, foreach_to_repeat);
    DUTStage<VRepeat> repeat(argc, argv, "cpp_repeat.fst", foreach_to_repeat, repeat_to_sink);
    Source<uint8_t> source(source_to_plus1, inputs);
    Sink<uint8_t> sink(repeat_to_sink, expected);

    graph.AddModule(&source);
    graph.AddModule(&plus1);
    graph.AddModule(&foreach);
    graph.AddModule(&repeat);
    graph.AddModule(&sink);
    graph.Connect(source_to_plus1, &source, &plus1);
    graph.Connect(plus1_to_foreach, &plus1, &foreach);
    graph.Connect(foreach_to_repeat, &foreach, &repeat);
    graph.Connect(repeat_to_sink, &repeat, &sink);
    graph.Compile();

    source.Reset();
    plus1.Reset();
    foreach.Reset();
    repeat.Reset();

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
