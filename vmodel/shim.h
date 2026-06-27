#pragma once

#include "vmodel/vmodel_base.h"

#include <cassert>
#include <cstdint>

namespace vmodel {

// Simple valid/ready handshake container.
template <typename Req>
struct ValidReady : public IChannel, public IChannelWrite<Req>, public IChannelRead<Req> {
    bool valid = false;
    bool valid_next = false;
    bool ready = false;
    Req data{};
    Req data_next{};

    ValidReady<Req> snapshot() const { return *this; }
    bool transfer() const { return valid && ready; }

    bool can_write() const override { return ready; }
    void write(Req d) override {
        valid_next = true;
        data_next = d;
    }

    bool can_read() const override { return valid; }
    Req read() const override { return data; }

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
        return vr_.can_write();
    }

    void write(Req d) override {
        vr_.write(d);
    }

    void drive(const ValidReady<Req>& v) const {
        vr_.valid_next = v.valid;
        vr_.data_next = v.data;
    }

    bool ready() const {
        return vr_.ready;
    }

    ValidReady<Req> snapshot() const {
        ValidReady<Req> s;
        s.valid = vr_.valid;
        s.ready = vr_.ready;
        s.data = vr_.data;
        return s;
    }

    bool transfer() const {
        return vr_.valid && vr_.ready;
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
        return vr_.can_read();
    }

    Req read() const override {
        return vr_.read();
    }

    ValidReady<Req> sample() const {
        ValidReady<Req> s;
        s.valid = vr_.valid;
        s.ready = vr_.ready;
        s.data = vr_.data;
        return s;
    }

    void set_ready(bool r) const {
        vr_.ready = r;
    }

    bool transfer() const {
        return vr_.valid && vr_.ready;
    }

private:
    ValidReady<Req>& vr_;
};

} // namespace vmodel
