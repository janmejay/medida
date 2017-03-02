//
// Copyright (c) 2012 Daniel Lundin
//

#include <gtest/gtest.h>

#include "medida/metrics_registry.h"

using namespace medida;

TEST(ValueTest, anEmptyValue) {
    MetricsRegistry registry{};
    auto &value = registry.new_value({"a", "b", "c"});

    EXPECT_NEAR(0, value.value(), 0.0001);
}

TEST(ValueTest, aValueWithUpdate) {
    MetricsRegistry registry{};
    auto &value = registry.new_value({"a", "b", "c"});

    for (auto i = 1; i <= 1000; i++) {
        value.update(i);
        EXPECT_NEAR(i, value.value(), 0.0001);
    }
}
