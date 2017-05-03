//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/histogram.h"

#include <atomic>
#include <cmath>
#include <mutex>

#include "medida/stats/exp_decay_sample.h"
#include "medida/stats/uniform_sample.h"

namespace medida {

    static const double kDefaultAlpha = 0.015;

    class Histogram::Impl {

    public:
        Impl(SampleType sample_type = kUniform);

        ~Impl();

        stats::Snapshot snapshot() const;

        double sum() const;

        double max() const;

        double min() const;

        double mean() const;

        double std_dev() const;

        void update(std::int64_t value);

        std::uint64_t count() const;

        double variance() const;

        void process(MetricProcessor& processor);

        void clear();

        const std::string& attribute_signature() const;

    private:
        static const std::uint64_t kDefaultSampleSize = 1028;

        std::unique_ptr<stats::Sample> sample_;

        std::atomic<std::int64_t> min_;

        std::atomic<std::int64_t> max_;

        std::atomic<std::int64_t> sum_;

        std::atomic<std::uint64_t> count_;

        double variance_m_;

        double variance_s_;

        mutable std::mutex variance_mutex_;

        std::string attr_sig_;
    };


    Histogram::Histogram(SampleType sample_type) : impl_ {new Histogram::Impl {sample_type}} { }


    Histogram::~Histogram() { }


    void Histogram::process(MetricProcessor& processor) {
        processor.process(*this);  // FIXME: pimpl?
    }


    void Histogram::clear() {
        impl_->clear();
    }


    std::uint64_t Histogram::count() const {
        return impl_->count();
    }


    double Histogram::sum() const {
        return impl_->sum();
    }


    double Histogram::max() const {
        return impl_->max();
    }


    double Histogram::min() const {
        return impl_->min();
    }


    double Histogram::mean() const {
        return impl_->mean();
    }


    double Histogram::std_dev() const {
        return impl_->std_dev();
    }


    void Histogram::update(std::int64_t value) {
        impl_->update(value);
    }


    stats::Snapshot Histogram::snapshot() const {
        return impl_->snapshot();
    }


    double Histogram::variance() const {
        return impl_->variance();
    }


    const std::string& Histogram::attribute_signature() const {
        return impl_->attribute_signature();
    }


// === Implementation ===


    Histogram::Impl::Impl(SampleType sample_type) {
        attr_sig_ = "sampling='";
        if (sample_type == kUniform) {
            sample_ = std::unique_ptr<stats::Sample>(new stats::UniformSample(kDefaultSampleSize));
            attr_sig_ += "uniform";
        } else if (sample_type == kBiased) {
            sample_ = std::unique_ptr<stats::Sample>(new stats::ExpDecaySample(kDefaultSampleSize, kDefaultAlpha));
            attr_sig_ += "biased";
        } else {
            throw std::invalid_argument("invalid sample_type");
        }
        attr_sig_ += "'";
        clear();
    }


    Histogram::Impl::~Impl() { }


    void Histogram::Impl::clear() {
        min_ = 0;
        max_ = 0;
        sum_ = 0;
        count_ = 0;
        variance_m_ = 0.0;
        variance_s_ = 0.0;
        sample_->clear();
    }


    std::uint64_t Histogram::Impl::count() const {
        return count_;
    }


    double Histogram::Impl::sum() const {
        return sum_.load();
    }


    double Histogram::Impl::max() const {
        if (count_ > 0) {
            return max_.load();
        }
        return 0.0;
    }


    double Histogram::Impl::min() const {
        if (count_ > 0) {
            return min_.load();
        }
        return 0.0;
    }


    double Histogram::Impl::mean() const {
        auto c = count();
        if (c > 0) {
            return sum_ / static_cast<double>(c);
        }
        return 0.0;
    }


    double Histogram::Impl::std_dev() const {
        if (count_ > 0) {
            return std::sqrt(variance());
        }
        return 0.0;
    }


    double Histogram::Impl::variance() const {
        auto c = count();
        if (c > 1) {
            std::lock_guard<std::mutex> lock {variance_mutex_};
            return variance_s_ / (c - 1.0);
        }
        return 0.0;
    }


    stats::Snapshot Histogram::Impl::snapshot() const {
        return sample_->snapshot();
    }


    void Histogram::Impl::update(std::int64_t value) {
        sample_->update(value);
        auto cur_max = max_.load();
        auto cur_min = min_.load();
        while (((count_ == 0) || (cur_max < value)) &&
               !max_.compare_exchange_weak(cur_max, value));
        while (((count_ == 0) || (cur_min > value)) &&
               !min_.compare_exchange_weak(cur_min, value));
        sum_ += value;
        auto new_count = ++count_;
        std::lock_guard<std::mutex> lock {variance_mutex_};
        auto old_vm = variance_m_;
        auto old_vs = variance_s_;
        if (new_count > 1) {
            variance_m_ = old_vm + (value - old_vm) / new_count;
            variance_s_ = old_vs + (value - old_vm) * (value - variance_m_);
        } else {
            variance_m_ = value;
        }
    }

    const std::string& Histogram::Impl::attribute_signature() const {
        return attr_sig_;
    }
}
