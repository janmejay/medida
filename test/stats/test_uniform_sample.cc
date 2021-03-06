//
// Copyright (c) 2012 Daniel Lundin
//


#include "medida/stats/uniform_sample.h"

#include <gtest/gtest.h>

using namespace medida::stats;

TEST(UniformSampleTest, aSampleOf100OutOf1000Elements) {
    UniformSample sample {100};

    for (auto i = 0; i < 1000; i++) {
        sample.update(i);
    }

    EXPECT_EQ(static_cast<std::size_t>(100), sample.size());

    auto vals = sample.snapshot().values();
    EXPECT_EQ(static_cast<std::size_t>(100), vals.size());

    for (auto& v : vals) {
        EXPECT_LT(v, 1000.0);
        EXPECT_GE(v, 0.0);
    }
}


TEST(UniformSampleTest, clear) {
    UniformSample sample {100};
    for (auto i = 0; i < 10; i++) {
        sample.update(i);
    }

    EXPECT_EQ(static_cast<std::size_t>(10), sample.size());

    sample.clear();

    EXPECT_EQ(static_cast<std::size_t>(0), sample.size());
}


TEST(UniformSampleTest, samplingAccuracy_InNoDropScenario) {
    for (auto j = 0; j < 100; j++) {
        UniformSample sample {10};
        for (auto i = 0; i < 10; i++) {
            sample.update(i);
        }

        auto snapshot = sample.snapshot();
    
        EXPECT_NEAR(7.25, snapshot.percentile_75(), 0.01);
    }
}
