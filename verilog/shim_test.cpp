#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <type_traits>

#include "vmodel/shim.h"
#include "vmodel/simgraph.h"

namespace {

template <typename T, typename = void>
struct HasRegisterWriteCallback : std::false_type {};

template <typename T>
struct HasRegisterWriteCallback<T, std::void_t<decltype(&T::RegisterWriteCallback)>> : std::true_type {};

template <typename T, typename = void>
struct HasRegisterReadCallback : std::false_type {};

template <typename T>
struct HasRegisterReadCallback<T, std::void_t<decltype(&T::RegisterReadCallback)>> : std::true_type {};

class DummyModule : public vmodel::IModule {
public:
    void Reset() override {}
    void Comb() override {}
    void Seq() override {}
};

TEST(ShimTest, IModuleResetIsCallableThroughInterface) {
    DummyModule module;
    vmodel::IModule& iface = module;

    iface.Reset();
}

TEST(ShimTest, ValidReadyCarriesName) {
    vmodel::ValidReady<int> channel("debug_channel");

    EXPECT_EQ(channel.name(), "debug_channel");
}

TEST(ShimTest, ChannelImplementationsDoNotExposeCallbackRegistration) {
    EXPECT_FALSE((HasRegisterWriteCallback<vmodel::Signal<int>>::value));
    EXPECT_FALSE((HasRegisterReadCallback<vmodel::Signal<int>>::value));
    EXPECT_FALSE((HasRegisterWriteCallback<vmodel::Valid<int>>::value));
    EXPECT_FALSE((HasRegisterReadCallback<vmodel::Valid<int>>::value));
    EXPECT_FALSE((HasRegisterWriteCallback<vmodel::Valid<>>::value));
    EXPECT_FALSE((HasRegisterReadCallback<vmodel::Valid<>>::value));
    EXPECT_FALSE((HasRegisterWriteCallback<vmodel::ValidReady<int>>::value));
    EXPECT_FALSE((HasRegisterReadCallback<vmodel::ValidReady<int>>::value));
    EXPECT_FALSE((HasRegisterWriteCallback<vmodel::ValidReady<>>::value));
    EXPECT_FALSE((HasRegisterReadCallback<vmodel::ValidReady<>>::value));
}

TEST(ShimTest, ValidReadyPulseCarriesName) {
    vmodel::ValidReady<> channel("debug_pulse");

    EXPECT_EQ(channel.name(), "debug_pulse");
}

TEST(ShimTest, ValidReadyPulseTransfersEvent) {
    vmodel::ValidReady<> channel;
    vmodel::ValidReadyIn<> in(channel);
    vmodel::ValidReadyOut<> out(channel);

    EXPECT_TRUE(in.CanWrite());
    in.Write();
    EXPECT_FALSE(in.CanWrite());
    channel.Seq();

    EXPECT_TRUE(out.CanRead());
    out.Read();
    channel.Seq();

    EXPECT_FALSE(out.CanRead());
    EXPECT_TRUE(in.CanWrite());
}

TEST(ShimTest, ValidReadyTypedCallbacksFireOnWriteAndRead) {
    vmodel::ValidReady<int> channel;
    vmodel::ValidReadyIn<int> in(channel);
    vmodel::ValidReadyOut<int> out(channel);

    int wrote = 0;
    int read = 0;
    in.RegisterWriteCallback([&](int value) { wrote = value; });
    out.RegisterReadCallback([&](int value) { read = value; });

    in.Write(17);
    EXPECT_EQ(wrote, 17);

    channel.Seq();
    EXPECT_EQ(out.Read(), 17);
    EXPECT_EQ(read, 17);
}

TEST(ShimTest, ValidReadyPulseCallbacksFireOnWriteAndRead) {
    vmodel::ValidReady<> channel;
    vmodel::ValidReadyIn<> in(channel);
    vmodel::ValidReadyOut<> out(channel);

    int writes = 0;
    int reads = 0;
    in.RegisterWriteCallback([&]() { ++writes; });
    out.RegisterReadCallback([&]() { ++reads; });

    in.Write();
    EXPECT_EQ(writes, 1);

    channel.Seq();
    out.Read();
    EXPECT_EQ(reads, 1);
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

    EXPECT_TRUE(in.CanWrite());
    in.Write(42);
    channel.Seq();

    EXPECT_TRUE(out.CanRead());
    EXPECT_EQ(out.Peek(), 42);
    EXPECT_EQ(out.Read(), 42);
    channel.Seq();

    EXPECT_FALSE(out.CanRead());
}

TEST(ShimTest, ValidTypedCallbacksFireOnWriteAndRead) {
    vmodel::Valid<int> channel;
    vmodel::ValidIn<int> in(channel);
    vmodel::ValidOut<int> out(channel);

    int wrote = 0;
    int read = 0;
    in.RegisterWriteCallback([&](int value) { wrote = value; });
    out.RegisterReadCallback([&](int value) { read = value; });

    in.Write(42);
    EXPECT_EQ(wrote, 42);

    channel.Seq();
    EXPECT_EQ(out.Read(), 42);
    EXPECT_EQ(read, 42);
}

TEST(ShimTest, ValidPulseTransfersEvent) {
    vmodel::Valid<> channel;
    vmodel::ValidIn<> in(channel);
    vmodel::ValidOut<> out(channel);

    EXPECT_TRUE(in.CanWrite());
    in.Write();
    channel.Seq();

    EXPECT_TRUE(out.CanRead());
    out.Read();
    channel.Seq();

    EXPECT_FALSE(out.CanRead());
}

TEST(ShimTest, ValidPulseCallbacksFireOnWriteAndRead) {
    vmodel::Valid<> channel;
    vmodel::ValidIn<> in(channel);
    vmodel::ValidOut<> out(channel);

    int writes = 0;
    int reads = 0;
    in.RegisterWriteCallback([&]() { ++writes; });
    out.RegisterReadCallback([&]() { ++reads; });

    in.Write();
    EXPECT_EQ(writes, 1);

    channel.Seq();
    out.Read();
    EXPECT_EQ(reads, 1);
}

TEST(ShimTest, SignalCarriesName) {
    vmodel::Signal<int> channel("signal_channel");

    EXPECT_EQ(channel.name(), "signal_channel");
}

TEST(ShimTest, SignalTransfersData) {
    vmodel::Signal<int> channel;
    vmodel::SignalIn<int> in(channel);
    vmodel::SignalOut<int> out(channel);

    EXPECT_TRUE(in.CanWrite());
    in.Write(11);
    channel.Seq();

    EXPECT_TRUE(out.CanRead());
    EXPECT_EQ(out.Peek(), 11);
    EXPECT_EQ(out.Read(), 11);

    in.Write(33);
    channel.Seq();
    EXPECT_EQ(out.Peek(), 33);
}

TEST(ShimTest, SignalCallbacksFireOnWriteAndRead) {
    vmodel::Signal<int> channel;
    vmodel::SignalIn<int> in(channel);
    vmodel::SignalOut<int> out(channel);

    int wrote = 0;
    int read = 0;
    in.RegisterWriteCallback([&](int value) { wrote = value; });
    out.RegisterReadCallback([&](int value) { read = value; });

    in.Write(11);
    EXPECT_EQ(wrote, 11);

    channel.Seq();
    EXPECT_EQ(out.Read(), 11);
    EXPECT_EQ(read, 11);
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

TEST(ShimTest, SimGraphConnectsSignal) {
    vmodel::SimGraph graph;
    auto& channel = graph.CreateChannel<vmodel::Signal<int>>("signal_channel");

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
