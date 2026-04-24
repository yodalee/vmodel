#pragma once

#include "vmodel/shim.h"
#include "vmodel/vmodel_base.h"

#include <cassert>
#include <queue>
#include <unordered_map>
#include <vector>

namespace vmodel {

// Determines the correct Comb() call order for a set of modules connected by
// ValidReady channels.
//
// Ordering rule: the module holding ValidReadyIn on a channel (the consumer,
// which drives "ready" backward) must call Comb() BEFORE the module holding
// ValidReadyOut on that channel (the producer, which reads "ready").
//
// This is computed once at construction via Kahn's topological sort.
// Comb() calls modules in that sorted order; Seq() calls all modules.
class SimGraph {
public:
    SimGraph(std::vector<IModule*> modules, std::vector<IChannel*> channels)
        : all_(std::move(modules)), comb_order_(toposort(all_, channels)) {}

    void Comb() {
        for (auto* m : comb_order_) m->Comb();
    }

    void Seq() {
        for (auto* m : all_) m->Seq();
    }

private:
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
            IModule* consumer = ch->downstream(); // ValidReadyIn holder, drives ready
            IModule* producer = ch->upstream();   // ValidReadyOut holder, reads ready

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
    std::vector<IModule*> comb_order_;
};

} // namespace vmodel
