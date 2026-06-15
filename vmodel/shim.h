#pragma once

#include "vmodel/vmodel_base.h"

#include <cassert>
#include <cstdint>

namespace vmodel {

// Simple valid/ready handshake container.
template <typename Req>
struct ValidReady : public IChannel {
    bool valid = false;
    bool valid_next = false;
    bool ready = false;
    Req data{};
    Req data_next{};

    void setValid(bool v) { valid_next = v; }
    void setReady(bool r) { ready = r; }
    void setData(Req d) { data_next = d; }

    bool getValid() const { return valid; }
    bool getReady() const { return ready; }
    Req getData() const { return data; }

    ValidReady<Req> snapshot() const { return *this; }
    bool transfer() const { return getValid() && getReady(); }

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
class ValidReadyIn {
public:
    explicit ValidReadyIn(ValidReady<Req>& vr)
        : vr_(vr) {}

    void drive(const ValidReady<Req>& v) const {
        vr_.setValid(v.getValid());
        vr_.setData(v.getData());
    }

    bool ready() const {
        return vr_.getReady();
    }

    ValidReady<Req> snapshot() const {
        ValidReady<Req> s;
        s.valid = vr_.valid;
        s.ready = vr_.ready;
        s.data = vr_.data;
        return s;
    }

    bool transfer() const {
        return vr_.getValid() && vr_.getReady();
    }

private:
    ValidReady<Req>& vr_;
};

// DUT -> Host interface wrapper.
// Host reads valid/data and drives ready.
template <typename Req>
class ValidReadyOut {
public:
    explicit ValidReadyOut(ValidReady<Req>& vr)
        : vr_(vr) {}

    ValidReady<Req> sample() const {
        ValidReady<Req> s;
        s.valid = vr_.valid;
        s.ready = vr_.ready;
        s.data = vr_.data;
        return s;
    }

    void set_ready(bool r) const {
        vr_.setReady(r);
    }

    bool transfer() const {
        return vr_.getValid() && vr_.getReady();
    }

private:
    ValidReady<Req>& vr_;
};

} // namespace vmodel
