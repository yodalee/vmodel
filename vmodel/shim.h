#pragma once

#include "vmodel/vmodel_base.h"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace vmodel {

template <typename Req>
struct Signal;

template <typename Req>
class SignalIn;

template <typename Req>
class SignalOut;

template <typename Req = void>
struct Valid;

template <typename Req = void>
class ValidIn;

template <typename Req = void>
class ValidOut;

// Simple always-available signal container.
template <typename Req>
struct Signal : public IChannel {
    explicit Signal(std::string name = {})
        : name_(std::move(name)) {}

    Req data{};
    Req data_next{};

    bool CanWrite() const { return true; }
    void Write(Req d) {
        data_next = d;
    }

    bool CanRead() const { return true; }
    Req Read() {
        return data;
    }
    Req Peek() const { return data; }

    void Seq() override {
        data = data_next;
    }

    const std::string& name() const override { return name_; }

    void set_upstream(IModule* m) override {
        upstream_ = m;
    }
    void set_downstream(IModule* m) override {
        downstream_ = m;
    }

    IModule* upstream() const override { return upstream_; }
    IModule* downstream() const override { return downstream_; }

private:
    std::string name_;
    IModule* upstream_ = nullptr;
    IModule* downstream_ = nullptr;
};

// Host -> DUT signal interface wrapper.
template <typename Req>
class SignalIn : public IChannelWrite<Req> {
public:
    explicit SignalIn(Signal<Req>& s)
        : s_(s) {}

    bool CanWrite() const override {
        return s_.CanWrite();
    }

    void Write(Req d) override {
        s_.Write(d);
    }

private:
    Signal<Req>& s_;
};

// DUT -> Host signal interface wrapper.
template <typename Req>
class SignalOut : public IChannelRead<Req> {
public:
    explicit SignalOut(Signal<Req>& s)
        : s_(s) {}

    bool CanRead() const override {
        return s_.CanRead();
    }

    Req Read() override {
        return s_.Read();
    }

    Req Peek() const override {
        return s_.Peek();
    }

private:
    Signal<Req>& s_;
};

// Simple valid-only container.
template <typename Req>
struct Valid : public IChannel {
    explicit Valid(std::string name = {})
        : name_(std::move(name)) {}

    bool valid = false;
    bool valid_next = false;
    Req data{};
    Req data_next{};

    bool CanWrite() const { return true; }
    void Write(Req d) {
        valid_next = true;
        data_next = d;
    }

    bool CanRead() const { return valid; }
    Req Read() {
        valid_next = false;
        return data;
    }
    Req Peek() const { return data; }

    void Seq() override {
        valid = valid_next;
        data = data_next;
    }

    const std::string& name() const override { return name_; }

    void set_upstream(IModule* m) override {
        upstream_ = m;
    }
    void set_downstream(IModule* m) override {
        downstream_ = m;
    }

    IModule* upstream() const override { return upstream_; }
    IModule* downstream() const override { return downstream_; }

private:
    std::string name_;
    IModule* upstream_ = nullptr;
    IModule* downstream_ = nullptr;
};

// Valid-only pulse container with no payload.
template <>
struct Valid<void> : public IChannel {
    explicit Valid(std::string name = {})
        : name_(std::move(name)) {}

    bool valid = false;
    bool valid_next = false;

    bool CanWrite() const { return true; }
    void Write() {
        valid_next = true;
    }

    bool CanRead() const { return valid; }
    void Read() {
        valid_next = false;
    }
    void Peek() const {}

    void Seq() override {
        valid = valid_next;
    }

    const std::string& name() const override { return name_; }

    void set_upstream(IModule* m) override {
        upstream_ = m;
    }
    void set_downstream(IModule* m) override {
        downstream_ = m;
    }

    IModule* upstream() const override { return upstream_; }
    IModule* downstream() const override { return downstream_; }

private:
    std::string name_;
    IModule* upstream_ = nullptr;
    IModule* downstream_ = nullptr;
};

// Host -> DUT valid-only interface wrapper.
// Host drives valid/data.
template <typename Req>
class ValidIn : public IChannelWrite<Req> {
public:
    explicit ValidIn(Valid<Req>& v)
        : v_(v) {}

    bool CanWrite() const override {
        return v_.CanWrite();
    }

    void Write(Req d) override {
        v_.Write(d);
    }

private:
    Valid<Req>& v_;
};

// Host -> DUT valid-only pulse wrapper with no payload.
template <>
class ValidIn<void> : public IChannelWrite<void> {
public:
    explicit ValidIn(Valid<>& v)
        : v_(v) {}

    bool CanWrite() const override {
        return v_.CanWrite();
    }

    void Write() override {
        v_.Write();
    }

private:
    Valid<>& v_;
};

// DUT -> Host valid-only interface wrapper.
// Host reads valid/data.
template <typename Req>
class ValidOut : public IChannelRead<Req> {
public:
    explicit ValidOut(Valid<Req>& v)
        : v_(v) {}

    bool CanRead() const override {
        return v_.CanRead();
    }

    Req Read() override {
        return v_.Read();
    }

    Req Peek() const override {
        return v_.Peek();
    }

private:
    Valid<Req>& v_;
};

// DUT -> Host valid-only pulse wrapper with no payload.
template <>
class ValidOut<void> : public IChannelRead<void> {
public:
    explicit ValidOut(Valid<>& v)
        : v_(v) {}

    bool CanRead() const override {
        return v_.CanRead();
    }

    void Read() override {
        v_.Read();
    }

    void Peek() const override {
        v_.Peek();
    }

private:
    Valid<>& v_;
};

template <typename Req = void>
struct ValidReady;

template <typename Req = void>
class ValidReadyIn;

template <typename Req = void>
class ValidReadyOut;

// Simple valid/ready handshake container.
template <typename Req>
struct ValidReady : public IChannel {
    explicit ValidReady(std::string name = {})
        : name_(std::move(name)) {}

    bool valid = false;
    bool valid_next = false;
    bool ready = true;
    Req data{};
    Req data_next{};

    bool CanWrite() const { return ready; }
    void Write(Req d) {
        valid_next = true;
        ready = false;
        data_next = d;
    }

    bool CanRead() const { return valid; }
    Req Read() {
        valid_next = false;
        ready = true;
        return data;
    }
    Req Peek() const { return data; }

    void Seq() override {
        valid = valid_next;
        data = data_next;
    }

    const std::string& name() const override { return name_; }

    void set_upstream(IModule* m) override {
        upstream_ = m;
    }
    void set_downstream(IModule* m) override {
        downstream_ = m;
    }

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

// Valid/ready pulse container with no payload.
template <>
struct ValidReady<void> : public IChannel {
    explicit ValidReady(std::string name = {})
        : name_(std::move(name)) {}

    bool valid = false;
    bool valid_next = false;
    bool ready = true;

    bool CanWrite() const { return ready; }
    void Write() {
        valid_next = true;
        ready = false;
    }

    bool CanRead() const { return valid; }
    void Read() {
        valid_next = false;
        ready = true;
    }
    void Peek() const {}

    void Seq() override {
        valid = valid_next;
    }

    const std::string& name() const override { return name_; }

    void set_upstream(IModule* m) override {
        upstream_ = m;
    }
    void set_downstream(IModule* m) override {
        downstream_ = m;
    }

    IModule* upstream() const override { return upstream_; }
    IModule* downstream() const override { return downstream_; }

private:
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

    bool CanWrite() const override {
        return vr_.CanWrite();
    }

    void Write(Req d) override {
        vr_.Write(d);
    }

private:
    ValidReady<Req>& vr_;
};

// Host -> DUT valid/ready pulse wrapper with no payload.
template <>
class ValidReadyIn<void> {
public:
    explicit ValidReadyIn(ValidReady<>& vr)
        : vr_(vr) {}

    bool CanWrite() const {
        return vr_.CanWrite();
    }

    void Write() const {
        vr_.Write();
    }

private:
    ValidReady<>& vr_;
};

// DUT -> Host interface wrapper.
// Host reads valid/data and drives ready.
template <typename Req>
class ValidReadyOut : public IChannelRead<Req> {
public:
    explicit ValidReadyOut(ValidReady<Req>& vr)
        : vr_(vr) {}

    bool CanRead() const override {
        return vr_.CanRead();
    }

    Req Read() override {
        return vr_.Read();
    }

    Req Peek() const override {
        return vr_.Peek();
    }

private:
    ValidReady<Req>& vr_;
};

// DUT -> Host valid/ready pulse wrapper with no payload.
template <>
class ValidReadyOut<void> {
public:
    explicit ValidReadyOut(ValidReady<>& vr)
        : vr_(vr) {}

    bool CanRead() const {
        return vr_.CanRead();
    }

    void Read() const {
        vr_.Read();
    }

    void Peek() const {
        vr_.Peek();
    }

private:
    ValidReady<>& vr_;
};

} // namespace vmodel
