//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_REPORTING_CONSOLE_REPORTER_H_
#define MEDIDA_REPORTING_CONSOLE_REPORTER_H_

#include <iostream>

#include "medida/metric_processor.h"
#include "medida/metrics_registry.h"
#include "medida/reporting/abstract_polling_reporter.h"

namespace medida {
    namespace reporting {
        class ConsoleReporter : public AbstractPollingReporter, MetricProcessor {

        public:
            ConsoleReporter(MetricsRegistry &registry, std::ostream& out = std::cerr);

            virtual ~ConsoleReporter();

            virtual void run();

            using MetricProcessor::process;

            virtual void process(Counter& counter);

            virtual void process(Meter& meter);

            virtual void process(Histogram& histogram);

            virtual void process(Timer& timer);

        private:
            class Impl;

            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif // MEDIDA_REPORTING_CONSOLE_REPORTER_H_
