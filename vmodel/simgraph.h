#pragma once

#include "vmodel/shim.h"
#include "vmodel/vmodel_base.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace vmodel {

// Determines the correct Comb() call order for a set of modules connected by
// channels.
//
// Ordering rule: a channel consumer must call Comb() BEFORE its producer.
//
// This is computed once at Compile() via Kahn's topological sort.
// Comb() calls modules in that sorted order; Seq() calls all modules.
class SimGraph {
public:
    SimGraph() = default;

    SimGraph(std::vector<IModule*> modules, std::vector<IChannel*> channels)
        : all_(std::move(modules)), channels_(std::move(channels)) {}

    void AddModule(IModule* module) {
        assert(module != nullptr);
        all_.push_back(module);
        compiled_ = false;
    }

    void AddChannel(IChannel* channel) {
        assert(channel != nullptr);
        RegisterChannel(channel);
        compiled_ = false;
    }

    template <typename ChannelT, typename... Args>
    ChannelT& CreateChannel(Args&&... args) {
        static_assert(std::is_base_of<IChannel, ChannelT>::value,
                      "ChannelT must inherit vmodel::IChannel");

        auto holder = std::make_unique<OwnedObjectHolder<ChannelT>>(std::forward<Args>(args)...);
        auto* channel = &holder->channel;
        owned_channels_.push_back(std::move(holder));
        channels_.push_back(channel);
        compiled_ = false;
        return *channel;
    }

    void Connect(IChannel& channel, IModule* upstream, IModule* downstream) {
        assert(upstream != nullptr);
        assert(downstream != nullptr);
        channel.setUpstream(upstream);
        channel.setDownstream(downstream);
        RegisterChannel(&channel);
        compiled_ = false;
    }

    void Compile() {
        comb_order_ = toposort(all_, channels_);
        compiled_ = true;
    }

    void Comb() {
        assert(compiled_ && "Call SimGraph::Compile() before Comb()");
        for (auto* m : comb_order_) m->Comb();
    }

    void Seq() {
        for (auto* m : all_) m->Seq();
        for (auto* ch : channels_) ch->Seq();
    }

private:
    class IOwnedObject {
    public:
        virtual ~IOwnedObject() = default;
    };

    template <typename ChannelT>
    class OwnedObjectHolder final : public IOwnedObject {
    public:
        template <typename... Args>
        explicit OwnedObjectHolder(Args&&... args)
            : channel(std::forward<Args>(args)...) {}

        ChannelT channel;
    };

    void RegisterChannel(IChannel* channel) {
        auto it = std::find(channels_.begin(), channels_.end(), channel);
        if (it == channels_.end()) {
            channels_.push_back(channel);
        }
    }

    static std::vector<IModule*> toposort(
        const std::vector<IModule*>& modules,
        const std::vector<IChannel*>& channels)
    {
        const int n = static_cast<int>(modules.size());

        std::unordered_map<IModule*, int> idx;
        for (int i = 0; i < n; ++i) idx[modules[i]] = i;

        // adj[a] contains b means: a must come before b in Comb order.
        std::vector<std::vector<int>> adj(n);
        std::vector<int> indegree(n, 0);

        for (auto* ch : channels) {
            IModule* consumer = ch->downstream(); // Channel sink/consumer
            IModule* producer = ch->upstream();   // Channel source/producer

            if (!consumer || !producer) continue;

            auto cit = idx.find(consumer);
            auto pit = idx.find(producer);
            if (cit == idx.end() || pit == idx.end()) continue;

            // consumer must Comb before producer.
            adj[cit->second].push_back(pit->second);
            indegree[pit->second]++;
        }

        // Kahn's BFS topological sort.
        std::queue<int> q;
        for (int i = 0; i < n; ++i) {
            if (indegree[i] == 0) q.push(i);
        }

        std::vector<IModule*> order;
        order.reserve(n);
        while (!q.empty()) {
            const int cur = q.front();
            q.pop();
            order.push_back(modules[cur]);
            for (int next : adj[cur]) {
                if (--indegree[next] == 0) q.push(next);
            }
        }

        assert(static_cast<int>(order.size()) == n &&
               "Cycle detected in channel dependency graph");
        return order;
    }

    std::vector<IModule*> all_;
    std::vector<IChannel*> channels_;
    std::vector<IModule*> comb_order_;
    std::vector<std::unique_ptr<IOwnedObject>> owned_channels_;
    bool compiled_ = false;
};

} // namespace vmodel
