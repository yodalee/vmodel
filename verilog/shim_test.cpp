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

TEST(ShimTest, ValidReadyPulseCarriesName) {
    vmodel::ValidReady<> channel("debug_pulse");

    EXPECT_EQ(channel.name(), "debug_pulse");
}

TEST(ShimTest, ValidReadyPulseTransfersEvent) {
    vmodel::ValidReady<> channel;
    vmodel::ValidReadyIn<> in(channel);
    vmodel::ValidReadyOut<> out(channel);

    EXPECT_TRUE(in.can_write());
    in.write();
    EXPECT_FALSE(in.can_write());
    channel.Seq();

    EXPECT_TRUE(out.can_read());
    out.read();
    channel.Seq();

    EXPECT_FALSE(out.can_read());
    EXPECT_TRUE(in.can_write());
}

TEST(ShimTest, ValidCarriesName) {
    vmodel::Valid<int> channel("valid_channel");

    EXPECT_EQ(channel.name(), "valid_channel");
}

TEST(ShimTest, ValidPulseCarriesName) {
    vmodel::Valid<> channel("valid_pulse");

    EXPECT_EQ(channel.name(), "valid_pulse");
}

TEST(ShimTest, ValidTypedTransfersData) {
    vmodel::Valid<int> channel;
    vmodel::ValidIn<int> in(channel);
    vmodel::ValidOut<int> out(channel);

    EXPECT_TRUE(in.can_write());
    in.write(42);
    channel.Seq();

    EXPECT_TRUE(out.can_read());
    EXPECT_EQ(out.peek(), 42);
    EXPECT_EQ(out.read(), 42);
    channel.Seq();

    EXPECT_FALSE(out.can_read());
}

TEST(ShimTest, ValidPulseTransfersEvent) {
    vmodel::Valid<> channel;
    vmodel::ValidIn<> in(channel);
    vmodel::ValidOut<> out(channel);

    EXPECT_TRUE(in.can_write());
    in.write();
    channel.Seq();

    EXPECT_TRUE(out.can_read());
    out.read();
    channel.Seq();

    EXPECT_FALSE(out.can_read());
}

TEST(ShimTest, SimGraphConnectsValid) {
    vmodel::SimGraph graph;
    auto& channel = graph.CreateChannel<vmodel::Valid<int>>("valid_channel");

    DummyModule upstream;
    DummyModule downstream;

    graph.AddModule(&upstream);
    graph.AddModule(&downstream);
    graph.Connect(channel, &upstream, &downstream);

    EXPECT_EQ(channel.upstream(), &upstream);
    EXPECT_EQ(channel.downstream(), &downstream);
}

TEST(ShimTest, SimGraphConnectsValidPulse) {
    vmodel::SimGraph graph;
    auto& channel = graph.CreateChannel<vmodel::Valid<>>("valid_pulse");

    DummyModule upstream;
    DummyModule downstream;

    graph.AddModule(&upstream);
    graph.AddModule(&downstream);
    graph.Connect(channel, &upstream, &downstream);

    EXPECT_EQ(channel.upstream(), &upstream);
    EXPECT_EQ(channel.downstream(), &downstream);
}

TEST(ShimTest, SimGraphConnectsValidReadyPulse) {
    vmodel::SimGraph graph;
    auto& channel = graph.CreateChannel<vmodel::ValidReady<>>("valid_ready_pulse");

    DummyModule upstream;
    DummyModule downstream;

    graph.AddModule(&upstream);
    graph.AddModule(&downstream);
    graph.Connect(channel, &upstream, &downstream);

    EXPECT_EQ(channel.upstream(), &upstream);
    EXPECT_EQ(channel.downstream(), &downstream);
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
