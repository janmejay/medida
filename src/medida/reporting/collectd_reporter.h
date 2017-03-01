//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_REPORTING_COLLECTD_REPORTER_H_
#define MEDIDA_REPORTING_COLLECTD_REPORTER_H_

#include <cstdint>
#include <memory>
#include <string>

#include "medida/metrics_registry.h"
#include "medida/metric_processor.h"
#include "medida/reporting/abstract_polling_reporter.h"

namespace medida {
    namespace reporting {
        class CollectdReporter : public AbstractPollingReporter, MetricProcessor {

        public:
            CollectdReporter(MetricsRegistry &registry, const std::string& hostname = "127.0.0.1", std::uint16_t port = 25826);

            virtual ~CollectdReporter();

            virtual void run();

            virtual void process(Counter& counter);

            virtual void process(Meter& meter);

            virtual void process(Histogram& histogram);

            virtual void process(Value& value);

            virtual void process(Timer& timer);

        private:
            class Impl;

            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif // MEDIDA_REPORTING_COLLECTD_REPORTER_H_
