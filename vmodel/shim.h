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

    void setValid(bool v) { valid_next = v; }
    void setReady(bool r) { ready = r; }
    void setData(Req d) { data_next = d; }

    // Initialization helpers for values that must be visible before first Seq.
    void initValid(bool v) {
        valid = v;
        valid_next = v;
    }
    void initData(Req d) {
        data = d;
        data_next = d;
    }

    bool getValid() const { return valid; }
    bool getReady() const { return ready; }
    Req getData() const { return data; }

    bool can_write() const override { return getReady(); }
    void write(Req req) override {
        setValid(true);
        setData(req);
    }

    bool can_read() const override { return getValid(); }
    Req read() const override {
        assert(can_read());
        return getData();
    }

    void Seq() override {
        valid = valid_next;
        data = data_next;
    }

    ValidReady<Req> snapshot() const { return *this; }
    bool transfer() const { return getValid() && getReady(); }

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
        return vr_.getReady();
    }

    void write(Req d) const override {
        vr_.setValid(true);
        vr_.setData(d);
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
        return vr_.getValid();
    }

    Req read() const override {
        assert(can_read());
        return vr_.getData();
    }

private:
    ValidReady<Req>& vr_;
};

} // namespace vmodel
