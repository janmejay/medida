//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/metrics_registry.h"

#include <algorithm>
#include <mutex>
#include <thread>
#include <sstream>
#include <cxxabi.h>

#include "medida/metric_name.h"

namespace medida {
    class MetricsRegistry::Impl {
    public:
        Impl();
    
        ~Impl();
    
        Counter& new_counter(const MetricName &name, std::int64_t init_value = 0);
    
        Histogram& new_histogram(const MetricName &name, SamplingInterface::SampleType sample_type = SamplingInterface::kUniform);
    
        Value& new_value(const MetricName &name);
    
        Meter& new_meter(const MetricName &name, std::string event_type, Clock::duration rate_unit = std::chrono::seconds(1));
    
        Timer& new_timer(const MetricName &name,
                        std::chrono::nanoseconds duration_unit = std::chrono::milliseconds(1),
                        std::chrono::nanoseconds rate_unit = std::chrono::seconds(1));
    
        std::map<MetricName, std::shared_ptr<MetricInterface>> get_all_metrics() const;
    
        void process_all(MetricProcessor& processor);
    
    private:
        std::map<MetricName, std::shared_ptr<MetricInterface>> metrics_;
    
        mutable std::mutex mutex_;
    
        template<typename T, typename... Args> T& new_metric(const MetricName& name, Args... args);
    };


    Conflict::Conflict(const std::string& msg) : runtime_error(msg) {}


    Conflict::~Conflict() {}


    MetricsRegistry::MetricsRegistry() : impl_ {new MetricsRegistry::Impl} {
    }


    MetricsRegistry::~MetricsRegistry() {
    }


    Counter& MetricsRegistry::new_counter(const MetricName &name, std::int64_t init_value) {
        return impl_->new_counter(name, init_value);
    }


    Histogram& MetricsRegistry::new_histogram(const MetricName &name, SamplingInterface::SampleType sample_type) {
        return impl_->new_histogram(name, sample_type);
    }


    Value& MetricsRegistry::new_value(const MetricName &name) {
        return impl_->new_value(name);
    }


    Meter& MetricsRegistry::new_meter(const MetricName &name, std::string event_type, Clock::duration rate_unit) {
        return impl_->new_meter(name, event_type, rate_unit);
    }


    Timer& MetricsRegistry::new_timer(const MetricName &name, std::chrono::nanoseconds duration_unit, std::chrono::nanoseconds rate_unit) {
        return impl_->new_timer(name, duration_unit, rate_unit);
    }


    std::map<MetricName, std::shared_ptr<MetricInterface>> MetricsRegistry::get_all_metrics() const {
        return impl_->get_all_metrics();
    }

// === Implementation ===

    MetricsRegistry::Impl::Impl() { }


    MetricsRegistry::Impl::~Impl() { }


    Counter& MetricsRegistry::Impl::new_counter(const MetricName &name, std::int64_t init_value) {
        return new_metric<Counter>(name, init_value);
    }


    Histogram& MetricsRegistry::Impl::new_histogram(const MetricName &name, SamplingInterface::SampleType sample_type) {
        return new_metric<Histogram>(name, sample_type);
    }

    
    Value& MetricsRegistry::Impl::new_value(const MetricName &name) {
        return new_metric<Value>(name);
    }

    
    Meter& MetricsRegistry::Impl::new_meter(const MetricName &name, std::string event_type, Clock::duration rate_unit) {
        return new_metric<Meter>(name, event_type, rate_unit);
    }


    Timer& MetricsRegistry::Impl::new_timer(const MetricName &name, std::chrono::nanoseconds duration_unit, std::chrono::nanoseconds rate_unit) {
        return new_metric<Timer>(name, duration_unit, rate_unit);
    }


    void fail_with_conflict(const MetricInterface& existing, const MetricInterface& attempted, const MetricName& name) {
        std::stringstream ss;
        int status;
        char* attempted_type = abi::__cxa_demangle(typeid(attempted).name(), 0, 0, &status);
        char* existing_type = abi::__cxa_demangle(typeid(existing).name(), 0, 0, &status);

        ss << "Name '" << name.to_string() << "'"
           << " of " << attempted_type << '(' << attempted.attribute_signature() << ')'
           << " conflicts with an already registered metric "
           << existing_type << '(' << existing.attribute_signature() << ')';

        free(attempted_type);
        free(existing_type);

        throw Conflict(ss.str());
    }


    template<typename MetricType, typename... Args>
    MetricType& MetricsRegistry::Impl::new_metric(const MetricName& name, Args... args) {
        std::lock_guard<std::mutex> lock {mutex_};
        if (metrics_.find(name) == std::end(metrics_)) {
            // GCC 4.6: Bug 44436 emplace* not implemented. Use ::reset instead.
            // metrics_[name].reset(new MetricType(args...));
            metrics_[name] = std::make_shared<MetricType>(args...);
        }

        MetricInterface& metric = *metrics_[name];
        std::unique_ptr<MetricType> tmp(new MetricType(args...));

        if ((typeid(metric) != typeid(MetricType)) ||
            (metric.attribute_signature() != tmp->attribute_signature())) {
            fail_with_conflict(metric, *tmp, name);
        }

        return dynamic_cast<MetricType&>(metric);
    }


    std::map<MetricName, std::shared_ptr<MetricInterface>> MetricsRegistry::Impl::get_all_metrics() const {
        std::lock_guard<std::mutex> lock {mutex_};
        return {metrics_};
    }
}
