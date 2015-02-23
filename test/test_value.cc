//
// Copyright (c) 2012 Daniel Lundin
//

#include <gtest/gtest.h>

#include "medida/metrics_registry.h"

using namespace medida;

TEST(ValueTest, anEmptyValue) {
    MetricsRegistry registry{};
    auto &value = registry.NewValue({"a", "b", "c"});

    EXPECT_EQ(0, value.value());
}

TEST(ValueTest, aValueWithUpdate) {
    MetricsRegistry registry{};
    auto &value = registry.NewValue({"a", "b", "c"});

    for (auto i = 1; i <= 1000; i++) {
        value.Update(i);
    }

    EXPECT_EQ(1000, value.value());
}
