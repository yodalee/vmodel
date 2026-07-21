#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "vmodel/shim.h"
#include "vmodel/simgraph.h"

namespace {

class DummyModule : public vmodel::IModule {
public:
    void Comb() override {}
    void Seq() override {}
};

TEST(ShimTest, ValidReadyCarriesName) {
    vmodel::ValidReady<int> channel("debug_channel");

    EXPECT_EQ(channel.name(), "debug_channel");
}

TEST(ShimTest, SimGraphRejectsSecondWriter) {
    vmodel::SimGraph graph;
    auto& channel = graph.CreateChannel<vmodel::ValidReady<int>>("debug_channel");

    DummyModule upstream1;
    DummyModule upstream2;
    DummyModule downstream;

    graph.AddModule(&upstream1);
    graph.AddModule(&upstream2);
    graph.AddModule(&downstream);

    graph.Connect(channel, &upstream1, &downstream);

    EXPECT_THROW(graph.Connect(channel, &upstream2, &downstream), std::runtime_error);
}

TEST(ShimTest, SimGraphRejectsSecondReader) {
    vmodel::SimGraph graph;
    auto& channel = graph.CreateChannel<vmodel::ValidReady<int>>("debug_channel");

    DummyModule upstream;
    DummyModule downstream1;
    DummyModule downstream2;

    graph.AddModule(&upstream);
    graph.AddModule(&downstream1);
    graph.AddModule(&downstream2);

    graph.Connect(channel, &upstream, &downstream1);

    EXPECT_THROW(graph.Connect(channel, &upstream, &downstream2), std::runtime_error);
}

} // namespace
