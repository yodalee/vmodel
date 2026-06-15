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
        top_->i_valid = in_ch_.valid_next;
        top_->i_data = in_ch_.data_next;
        // Drive DUT output ready from sink channel.
        top_->o_ready = out_ch_.ready;

        top_.Comb();

        // Publish DUT handshakes/data back to channels.
        in_ch_.ready = top_->i_ready;
        out_ch_.setValid(top_->o_valid);
        out_ch_.setData(top_->o_data);
    }

    void Seq() override {
        top_.Seq();
    }

private:
    VMod<VTop> top_;
    Channel& in_ch_;
    Channel& out_ch_;
};

class Source : public vmodel::IModule {
public:
    Source(Channel& out_ch, const std::vector<uint8_t>& inputs)
        : out_ch_(out_ch), inputs_(inputs) {}

    void Prime() {
        if (inputs_.empty()) {
            out_ch_.setValid(0);
            out_ch_.setData(0);
            return;
        }

        out_ch_.setValid(1);
        out_ch_.setData(inputs_[0]);
    }

    void Comb() {
        did_transfer_ = out_ch_.transfer();
    }

    void Seq() {
        if (!did_transfer_) {
            return;
        }

        ++input_idx_;
        if (input_idx_ < (int)inputs_.size()) {
            out_ch_.setValid(1);
            out_ch_.setData(inputs_[input_idx_]);
        } else {
            out_ch_.setValid(0);
            out_ch_.setData(0);
        }

        did_transfer_ = false;
    }

private:
    Channel& out_ch_;
    const std::vector<uint8_t>& inputs_;
    int input_idx_ = 0;
    bool did_transfer_ = false;
};

class Sink : public vmodel::IModule {
public:
    Sink(Channel& in_ch, const std::vector<uint8_t>& expected)
        : in_ch_(in_ch), expected_(expected) {}

    void SetReady(bool ready) {
        in_ch_.ready = ready;
    }

    void Comb() {
        if (!in_ch_.transfer()) {
            got_transfer_ = false;
            return;
        }

        got_transfer_ = true;
        sampled_data_ = in_ch_.snapshot().data;
    }

    void Seq() {
        if (!got_transfer_) {
            ++cycle_;
            return;
        }

        const uint8_t got = sampled_data_;
        printf("[cycle %4d] output[%2d]: got=%3u  expected=%3u  %s\n",
               cycle_, output_idx_, got, expected_[output_idx_],
               got == expected_[output_idx_] ? "OK" : "MISMATCH");
        assert(got == expected_[output_idx_]);
        ++output_idx_;
        got_transfer_ = false;
        ++cycle_;
    }

    bool Done() const {
        return output_idx_ == (int)expected_.size();
    }

    int OutputCount() const {
        return output_idx_;
    }

    int ExpectedCount() const {
        return (int)expected_.size();
    }

private:
    Channel& in_ch_;
    const std::vector<uint8_t>& expected_;
    int output_idx_ = 0;
    int cycle_ = 0;
    bool got_transfer_ = false;
    uint8_t sampled_data_ = 0;
};

int main(int argc, char** argv) {
    // Input stream and expected output sequence.
    const std::vector<uint8_t> inputs   = {3, 1};
    const std::vector<uint8_t> expected = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 1, 2, 2};

    vmodel::SimGraph graph;
    Channel& source_to_dut = graph.CreateChannel<Channel>();
    Channel& dut_to_sink = graph.CreateChannel<Channel>();

    DUT dut(argc, argv, source_to_dut, dut_to_sink);
    Source source(source_to_dut, inputs);
    Sink sink(dut_to_sink, expected);

    graph.AddModule(&source);
    graph.AddModule(&dut);
    graph.AddModule(&sink);
    graph.Connect(source_to_dut, &source, &dut);
    graph.Connect(dut_to_sink, &dut, &sink);
    graph.Compile();

    sink.SetReady(true);
    dut.Reset();
    source.Prime();

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
