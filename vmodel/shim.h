#pragma once

#include <cassert>
#include <cstdint>

namespace vmodel {

// Simple valid/ready handshake container.
template <typename Req>
struct ValidReady {
    uint8_t valid = 0;
    uint8_t ready = 0;
    Req data{};

    void setValid(bool v) { valid = v ? 1 : 0; }
    void setReady(bool r) { ready = r ? 1 : 0; }
    void setData(Req d) { data = d; }

    bool getValid() const { return valid != 0; }
    bool getReady() const { return ready != 0; }
    Req getData() const { return data; }
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
