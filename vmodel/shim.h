#pragma once

#include "vmodel/vmodel_base.h"

#include <cassert>
#include <cstdint>
#include <string>

namespace vmodel {

// Simple valid/ready handshake container.
template <typename Req>
struct ValidReady : public IChannel, public IChannelWrite<Req>, public IChannelRead<Req> {
    explicit ValidReady(std::string name = {})
        : name_(std::move(name)) {}

    bool valid = false;
    bool valid_next = false;
    bool ready = false;
    Req data{};
    Req data_next{};

    bool can_write() const override { return ready; }
    void write(Req d) override {
        valid_next = true;
        ready = false;
        data_next = d;
    }

    bool can_read() const override { return valid; }
    Req read() override { 
        valid_next = false;
        ready = true;
        return data;
    }
    Req peek() const override { return data; }

    void Seq() override {
        valid = valid_next;
        data = data_next;
    }

    void setUpstream(IModule* m) override { upstream_ = m; }
    void setDownstream(IModule* m) override { downstream_ = m; }
    const std::string& name() const override { return name_; }

    IModule* upstream() const override { return upstream_; }
    IModule* downstream() const override { return downstream_; }

private:
    const std::string& ChannelLabel() const {
        static const std::string unnamed = "<unnamed>";
        return name_.empty() ? unnamed : name_;
    }

    std::string name_;
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

    bool can_read() override {
        return vr_.can_read();
    }

    Req read() override {
        return vr_.read();
    }

    Req peek() const override {
        return vr_.peek();
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
