#pragma once

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
    virtual void setUpstream(IModule* m) = 0;
    virtual void setDownstream(IModule* m) = 0;
    virtual IModule* upstream() const = 0;
    virtual IModule* downstream() const = 0;
};

}; // namespace vmodel