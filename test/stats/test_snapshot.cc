//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/stats/snapshot.h"

#include <gtest/gtest.h>

using namespace medida::stats;

struct SnapshotTest : public ::testing::Test {
    SnapshotTest()
        : values {5, 1, 2, 3, 4},
          snapshot {values} {
          };
    const std::vector<double> values;
    const Snapshot snapshot;
};


TEST_F(SnapshotTest, smallQuantilesAreTheFirstValue) {
    EXPECT_DOUBLE_EQ(1, snapshot.quantile(0.0));
}


TEST_F(SnapshotTest, bigQuantilesAreTheLastValue) {
    EXPECT_DOUBLE_EQ(5, snapshot.quantile(1.0));
}


TEST_F(SnapshotTest, hasAMedian) {
    EXPECT_DOUBLE_EQ(3, snapshot.median());
}


TEST_F(SnapshotTest, hasAp75) {
    EXPECT_DOUBLE_EQ(4.5, snapshot.percentile_75());
}


TEST_F(SnapshotTest, hasAp95) {
    EXPECT_DOUBLE_EQ(5.0, snapshot.percentile_95());
}


TEST_F(SnapshotTest, hasAp98) {
    EXPECT_DOUBLE_EQ(5.0, snapshot.percentile_98());
}


TEST_F(SnapshotTest, hasAp99) {
    EXPECT_DOUBLE_EQ(5.0, snapshot.percentile_99());
}


TEST_F(SnapshotTest, hasAp999) {
    EXPECT_DOUBLE_EQ(5.0, snapshot.percentile_999());
}


TEST_F(SnapshotTest, hasValues) {
    auto ref = {1, 2, 3, 4, 5};
    EXPECT_TRUE(std::equal(ref.begin(), ref.end(), snapshot.values().begin()));
}


TEST_F(SnapshotTest, hasASize) {
    EXPECT_EQ(static_cast<std::size_t>(5), snapshot.size());
}
