#pragma once

#include "vmodel/vmodel_base.h"

#include <cassert>
#include <cstdio>
#include <vector>

template <typename T>
class Sink : public vmodel::IModule {
public:
    Sink(vmodel::IChannelRead<T>& in_ch, const std::vector<T>& expected)
        : in_ch_(in_ch), expected_(expected) {}

    void Reset() override {}

    void Comb() override {
        got_transfer_ = in_ch_.CanRead();
        if (got_transfer_) {
            sampled_data_ = in_ch_.Read();
        }
    }

    void Seq() override {
        if (!got_transfer_) {
            ++cycle_;
            return;
        }

        const T got = sampled_data_;
         printf("[cycle %4d] output[%2d]: got=%3u  expected=%3u  %s\n",
             cycle_, output_idx_, static_cast<unsigned>(got), static_cast<unsigned>(expected_[output_idx_]),
               got == expected_[output_idx_] ? "OK" : "MISMATCH");
        assert(got == expected_[output_idx_]);
        ++output_idx_;
        got_transfer_ = false;
        ++cycle_;
    }

    bool Done() const {
        return output_idx_ == static_cast<int>(expected_.size());
    }

    int OutputCount() const {
        return output_idx_;
    }

private:
    vmodel::IChannelRead<T>& in_ch_;
    const std::vector<T>& expected_;
    int output_idx_ = 0;
    int cycle_ = 0;
    bool got_transfer_ = false;
    T sampled_data_{};
};
