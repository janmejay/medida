//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/metrics_registry.h"

#include <gtest/gtest.h>

using namespace medida;

struct MetricsRegistryTest : public ::testing::Test {
    MetricsRegistry registry;
};


TEST_F(MetricsRegistryTest, keysByName) {
    auto& abc = registry.new_counter({"a", "b", "c"});
    auto& abc2 = registry.new_counter({"a", "b", "c"});
    auto& abcd = registry.new_counter({"a", "b", "c", "d"});
    EXPECT_EQ(0, abc.count()) << "Counter a.b.c was not initialied to 0";
    EXPECT_EQ(&abc, &abc2) << "Counter a.b.c was created twice";
    EXPECT_NE(&abc, &abcd) << "Counter a.b.c and a.b.c.d are the same object";
}

TEST_F(MetricsRegistryTest, does_not_mix_up_metric_types) {
    registry.new_counter({"a", "b", "c"});
    try {
        registry.new_value({"a", "b", "c"});
        FAIL();
    }  catch (const medida::Conflict& e) {
        EXPECT_EQ("Name 'a.b.c' of medida::Value() conflicts with an already registered metric medida::Counter(initial_value='0')", std::string(e.what()));
    }
}

TEST_F(MetricsRegistryTest, does_not_mix_up_non_name_attributes__for_meter) {
    registry.new_meter({"a", "b", "c"}, "foo", std::chrono::seconds(1));
    try {
        registry.new_meter({"a", "b", "c"}, "foo", std::chrono::milliseconds(1));
        FAIL();
    }  catch (const medida::Conflict& e) {
        EXPECT_EQ("Name 'a.b.c' of medida::Meter(event_type='foo', rate_unit='1000000 ns') conflicts with an already registered metric medida::Meter(event_type='foo', rate_unit='1000000000 ns')", std::string(e.what()));
    }

    try {
        registry.new_meter({"a", "b", "c"}, "bar", std::chrono::seconds(1));
        FAIL();
    }  catch (const medida::Conflict& e) {
        EXPECT_EQ("Name 'a.b.c' of medida::Meter(event_type='bar', rate_unit='1000000000 ns') conflicts with an already registered metric medida::Meter(event_type='foo', rate_unit='1000000000 ns')", std::string(e.what()));
    }
}

TEST_F(MetricsRegistryTest, does_not_mix_up_non_name_attributes__for_histogram) {
    registry.new_histogram({"a", "b", "c"}, medida::SamplingInterface::kBiased);
    try {
        registry.new_histogram({"a", "b", "c"}, medida::SamplingInterface::kUniform);
        FAIL();
    }  catch (const medida::Conflict& e) {
        EXPECT_EQ("Name 'a.b.c' of medida::Histogram(sampling='uniform') conflicts with an already registered metric medida::Histogram(sampling='biased')", std::string(e.what()));
    }
}

TEST_F(MetricsRegistryTest, does_not_mix_up_non_name_attributes__for_timer) {
    registry.new_timer({"a", "b", "c"}, std::chrono::microseconds(1), std::chrono::microseconds(2));
    try {
        registry.new_timer({"a", "b", "c"}, std::chrono::microseconds(1), std::chrono::microseconds(1));
        FAIL();
    }  catch (const medida::Conflict& e) {
        EXPECT_EQ("Name 'a.b.c' of medida::Timer(duration_unit='1000 ns', rate_unit='1000 ns') conflicts with an already registered metric medida::Timer(duration_unit='1000 ns', rate_unit='2000 ns')", std::string(e.what()));
    }

    try {
        registry.new_timer({"a", "b", "c"}, std::chrono::microseconds(2), std::chrono::microseconds(2));
        FAIL();
    }  catch (const medida::Conflict& e) {
        EXPECT_EQ("Name 'a.b.c' of medida::Timer(duration_unit='2000 ns', rate_unit='2000 ns') conflicts with an already registered metric medida::Timer(duration_unit='1000 ns', rate_unit='2000 ns')", std::string(e.what()));
    }
}

TEST_F(MetricsRegistryTest, does_not_mix_up_non_name_attributes__for_counter) {
    registry.new_counter({"a", "b", "c"}, 10);
    try {
        registry.new_counter({"a", "b", "c"}, 15);
        FAIL();
    }  catch (const medida::Conflict& e) {
        EXPECT_EQ("Name 'a.b.c' of medida::Counter(initial_value='15') conflicts with an already registered metric medida::Counter(initial_value='10')", std::string(e.what()));
    }
}

TEST_F(MetricsRegistryTest, should_allow____conflict_free____duplicate____definition_of_metrics) {
    //second line in each fragment shouldn't fail
    registry.new_counter({"a", "b", "c"}, 10);
    registry.new_counter({"a", "b", "c"}, 10);

    registry.new_meter({"a", "b", "m"}, "foo", std::chrono::seconds(1));
    registry.new_meter({"a", "b", "m"}, "foo", std::chrono::seconds(1));

    registry.new_histogram({"a", "b", "h"}, medida::SamplingInterface::kBiased);
    registry.new_histogram({"a", "b", "h"}, medida::SamplingInterface::kBiased);

    registry.new_timer({"a", "b", "t"}, std::chrono::microseconds(1), std::chrono::microseconds(2));
    registry.new_timer({"a", "b", "t"}, std::chrono::microseconds(1), std::chrono::microseconds(2));

    registry.new_value({"a", "b", "v"});
    registry.new_value({"a", "b", "v"});

    auto all_metrics = registry.get_all_metrics();

    EXPECT_EQ(static_cast<std::uint32_t>(5), all_metrics.size());
}
