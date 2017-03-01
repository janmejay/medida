//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/stats/uniform_sample.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <random>
#include <vector>

namespace medida {
    namespace stats {
        class UniformSample::Impl {

        public:

            Impl(std::uint32_t reservoirSize);

            ~Impl();

            void clear();

            std::uint64_t size() const;

            void update(std::int64_t value);

            Snapshot snapshot() const;

        private:
            std::atomic<std::uint64_t> count_;

            std::vector<std::int64_t> values_;

            mutable std::mt19937_64 rng_;

            mutable std::mutex mutex_;
        };


        UniformSample::UniformSample(std::uint32_t reservoirSize) : impl_ {new UniformSample::Impl {reservoirSize}} { }


        UniformSample::~UniformSample() { }


        void UniformSample::clear() {
            impl_->clear();
        }


        std::uint64_t UniformSample::size() const {
            return impl_->size();
        }


        void UniformSample::update(std::int64_t value) {
            impl_->update(value);
        }


        Snapshot UniformSample::snapshot() const {
            return impl_->snapshot();
        }


// === Implementation ===


        UniformSample::Impl::Impl(std::uint32_t reservoirSize)
            : count_          {},
              values_         (reservoirSize), // FIXME: Explicit and non-uniform
              rng_            {std::random_device()()},
              mutex_          {} {
                  clear();
              }


        UniformSample::Impl::~Impl() { }


        void UniformSample::Impl::clear() {
            std::lock_guard<std::mutex> lock {mutex_};
            for (auto& v : values_) {
                v = 0;
            }
            count_.store(0, std::memory_order_relaxed);
        }


        std::uint64_t UniformSample::Impl::size() const {
            std::uint64_t count = count_.load(std::memory_order_relaxed);
            std::lock_guard<std::mutex> lock {mutex_};
            std::uint64_t size = values_.size();
            return std::min(count, size);
        }


        void UniformSample::Impl::update(std::int64_t value) {
            auto count = count_.fetch_add(1, std::memory_order_relaxed);
            std::lock_guard<std::mutex> lock {mutex_};
            auto size = values_.size();
            if (count < size) {
                values_[count] = value;
            } else {
                std::uniform_int_distribution<uint64_t> uniform(0, count);
                auto rand = uniform(rng_);
                if (rand < size) {
                    values_[rand] = value;
                }
            }
        }


        Snapshot UniformSample::Impl::snapshot() const {
            std::uint64_t count = count_.load(std::memory_order_relaxed);
            std::lock_guard<std::mutex> lock {mutex_};
            std::uint64_t size = values_.size();
            auto begin = std::begin(values_);
            return Snapshot {{begin, begin + std::min(count, size)}};
        }
    }
}
