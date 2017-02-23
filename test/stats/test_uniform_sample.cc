//
// Copyright (c) 2012 Daniel Lundin
//


#include "medida/stats/uniform_sample.h"

#include <gtest/gtest.h>

using namespace medida::stats;

TEST(UniformSampleTest, aSampleOf100OutOf1000Elements) {
  UniformSample sample {100};

  for (auto i = 0; i < 1000; i++) {
    sample.Update(i);
  }

  EXPECT_EQ(static_cast<std::size_t>(100), sample.size());

  auto vals = sample.MakeSnapshot().getValues();
  EXPECT_EQ(static_cast<std::size_t>(100), vals.size());

  for (auto& v : vals) {
    EXPECT_LT(v, 1000.0);
    EXPECT_GE(v, 0.0);
  }
}


TEST(UniformSampleTest, clear) {
  UniformSample sample {100};
  for (auto i = 0; i < 10; i++) {
    sample.Update(i);
  }

  EXPECT_EQ(static_cast<std::size_t>(10), sample.size());

  sample.Clear();

  EXPECT_EQ(static_cast<std::size_t>(0), sample.size());
}
