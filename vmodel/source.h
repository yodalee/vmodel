#pragma once

#include "vmodel/vmodel_base.h"

#include <vector>

template <typename T>
class Source : public vmodel::IModule {
public:
    Source(vmodel::IChannelWrite<T>& out_ch, const std::vector<T>& inputs)
        : out_ch_(out_ch), inputs_(inputs) {}

    void Reset() {
        if (inputs_.empty()) {
            return;
        }
        out_ch_.Write(inputs_[0]);
    }

    void Comb() override {
        did_transfer_ = out_ch_.CanWrite();
    }

    void Seq() override {
        if (!did_transfer_) {
            return;
        }

        ++input_idx_;
        if (input_idx_ < static_cast<int>(inputs_.size())) {
            out_ch_.Write(inputs_[input_idx_]);
        }

        did_transfer_ = false;
    }

private:
    vmodel::IChannelWrite<T>& out_ch_;
    const std::vector<T>& inputs_;
    int input_idx_ = 0;
    bool did_transfer_ = false;
};
