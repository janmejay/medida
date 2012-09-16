//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/stats/exp_decay_sample.h"

#include <algorithm>
#include <cmath>
#include <functional>

#include "glog/logging.h"

#include "medida/stats/snapshot.h"

namespace medida {
namespace stats {

const Clock::duration ExpDecaySample::kRESCALE_THRESHOLD = std::chrono::hours{1};

ExpDecaySample::ExpDecaySample(std::uint32_t reservoirSize, double alpha)
    : alpha_         {alpha},
      reservoirSize_ {reservoirSize},
      count_         {},
      rng_           {std::random_device()()},
      dist_          (0, 1) {
  Clear();
}

ExpDecaySample::~ExpDecaySample() {
  DLOG(INFO) << "ExpDecaySample " << this << " destroyed";
}

void ExpDecaySample::Clear() {
  std::lock_guard<std::mutex> lock {mutex_};
  values_.clear();
  count_ = 0;
  startTime_ = Clock::now();
  nextScaleTime_ = startTime_ + kRESCALE_THRESHOLD;
}

std::uint64_t ExpDecaySample::size() const {
  return std::min(reservoirSize_, count_.load());
}

void ExpDecaySample::Update(std::int64_t value) {
  Update(value, Clock::now());
}


void ExpDecaySample::Update(std::int64_t value, Clock::time_point timestamp) {
  {
    if (timestamp >= nextScaleTime_) {
      Rescale(timestamp);
    }

    std::lock_guard<std::mutex> lock {mutex_};
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - startTime_);
    auto priority = std::exp(alpha_ * dur.count()) / dist_(rng_);
    auto count = ++count_;

    if (count <= reservoirSize_) {
      values_[priority] = value;
    } else {
      auto first = std::begin(values_)->first;
      if (first < priority && values_.insert({priority, value}).second) {
        while (values_.erase(first) == 0) {
          first = std::begin(values_)->first;
        }
      }
    }
  }
}


void ExpDecaySample::Rescale(const Clock::time_point& when) {
  std::lock_guard<std::mutex> lock {mutex_};
  nextScaleTime_ = when + kRESCALE_THRESHOLD;
  auto oldStartTime = startTime_;
  startTime_ = when;
  auto size = values_.size();
  std::vector<double> keys;
  std::vector<std::int64_t> values;

  keys.reserve(size);
  values.reserve(size);
  for (auto& kv : values_) {
    keys.push_back(kv.first);
    values.push_back(kv.second);
  }

  values_.clear();

  for (auto i = 0; i < size; i++) {
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(when - oldStartTime);
    auto key = keys[i] * std::exp(-alpha_ * dur.count());
    values_[key] = values[i];
  }

  count_ = values_.size();
}

Snapshot ExpDecaySample::MakeSnapshot() const {
  // auto begin = std::begin(values_);
  // auto end = begin + std::min(count_.load(), values_.size());
  return {values_};
}

} // namespace stats
} // namespace medida