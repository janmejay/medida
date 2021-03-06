//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_REPORTING_JSON_REPORTER_H_
#define MEDIDA_REPORTING_JSON_REPORTER_H_

#include <memory>
#include <string>

#include "medida/metric_processor.h"
#include "medida/metrics_registry.h"

namespace medida {
    namespace reporting {
        class JsonReporter : MetricProcessor {

        public:
            JsonReporter(MetricsRegistry &registry);

            virtual ~JsonReporter();

            using MetricProcessor::process;

            virtual void process(Counter& counter);

            virtual void process(Meter& meter);

            virtual void process(Histogram& histogram);

            virtual void process(Timer& timer);

            virtual std::string report();

        private:
            class Impl;

            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif // MEDIDA_REPORTING_JSON_REPORTER_H_
