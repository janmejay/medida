//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_METRICS_REGISTRY_H_
#define MEDIDA_METRICS_REGISTRY_H_

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "medida/counter.h"
#include "medida/histogram.h"
#include "medida/meter.h"
#include "medida/metric_interface.h"
#include "medida/metric_name.h"
#include "medida/metric_processor.h"
#include "medida/timer.h"
#include "medida/value.h"

namespace medida {
    class Conflict : public std::runtime_error {

    public:
        Conflict(const std::string& msg);

        virtual ~Conflict();
    };

    class MetricsRegistry {

    public:
        MetricsRegistry();
        
        ~MetricsRegistry();
        
        Counter& new_counter(const MetricName &name, std::int64_t init_value = 0);
        
        Histogram& new_histogram(const MetricName &name, SamplingInterface::SampleType sample_type = SamplingInterface::kUniform);
        
        Value& new_value(const MetricName &name);
        
        Meter& new_meter(const MetricName &name, std::string event_type, Clock::duration rate_unit = std::chrono::seconds(1));
        
        Timer& new_timer(const MetricName &name,
                        std::chrono::nanoseconds duration_unit = std::chrono::milliseconds(1),
                        std::chrono::nanoseconds rate_unit = std::chrono::seconds(1));
        
        std::map<MetricName, std::shared_ptr<MetricInterface>> get_all_metrics() const;
        
    private:
        class Impl;
        
        std::unique_ptr<Impl> impl_;
    };
}

#endif // MEDIDA_METRICS_REGISTRY_H_
