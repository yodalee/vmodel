#pragma once

#include "vmodel/vmodel_base.h"

#include <cassert>
#include <cstdint>

namespace vmodel {

// Simple valid/ready handshake container.
template <typename Req>
struct ValidReady : public IChannel, public IChannelWrite<Req>, public IChannelRead<Req> {
    // valid/data are committed on Seq(); ready is immediate.
    bool valid = false;
    bool valid_next = false;
    bool ready = false;
    Req data{};
    Req data_next{};

    void Reset() {
        valid = false;
        valid_next = false;
        ready = false;
        data = Req{};
        data_next = Req{};
    }

    bool can_write() const override { return ready; }
    void write(Req req) override {
        valid_next = true;
        data_next = req;
    }

    bool can_read() const override { return valid; }
    Req read() const override {
        assert(can_read());
        return data;
    }

    void Seq() override {
        valid = valid_next;
        data = data_next;
    }

    void setUpstream(IModule* m) override { upstream_ = m; }
    void setDownstream(IModule* m) override { downstream_ = m; }

    IModule* upstream() const override { return upstream_; }
    IModule* downstream() const override { return downstream_; }

private:
    IModule* upstream_ = nullptr;
    IModule* downstream_ = nullptr;
};

// Host -> DUT interface wrapper.
// Host drives valid/data and reads ready.
template <typename Req>
class ValidReadyIn : public IChannelWrite<Req> {
public:
    explicit ValidReadyIn(ValidReady<Req>& vr)
        : vr_(vr) {}

    bool can_write() const override {
        return vr_.ready;
    }

    void write(Req d) const override {
        vr_.valid_next = true;
        vr_.data_next = d;
    }

private:
    ValidReady<Req>& vr_;
};

// DUT -> Host interface wrapper.
// Host reads valid/data and drives ready.
template <typename Req>
class ValidReadyOut : public IChannelRead<Req> {
public:
    explicit ValidReadyOut(ValidReady<Req>& vr)
        : vr_(vr) {}

    bool can_read() const override {
        return vr_.valid;
    }

    Req read() const override {
        assert(can_read());
        return vr_.data;
    }

private:
    ValidReady<Req>& vr_;
};

} // namespace vmodel
