#pragma once

#include <string>

namespace vmodel {

// Common interface for all simulation modules.
class IModule {
public:
    virtual ~IModule() = default;
    virtual void Comb() = 0;
    virtual void Seq() = 0;
};

class IChannel {
public:
    virtual ~IChannel() = default;
    virtual void Seq() = 0;
    virtual const std::string& name() const = 0;
    virtual void setUpstream(IModule* m) = 0;
    virtual void setDownstream(IModule* m) = 0;
    virtual IModule* upstream() const = 0;
    virtual IModule* downstream() const = 0;
};

template <typename Req>
class IChannelWrite {
public:
    virtual ~IChannelWrite() = default;
    virtual bool can_write() const = 0;
    virtual void write(Req d) = 0;
};

template <typename Req>
class IChannelRead {
public:
    virtual ~IChannelRead() = default;
    virtual bool can_read() const = 0;
    virtual Req read() = 0;
    virtual Req peek() const = 0;
};

}; // namespace vmodel
