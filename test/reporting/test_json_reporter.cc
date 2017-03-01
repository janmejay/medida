//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/reporting/json_reporter.h"

#include <gtest/gtest.h>

#include "medida/metrics_registry.h"

using namespace medida;
using namespace medida::reporting;


TEST(JsonReporterTest, foo) {
    MetricsRegistry registry {};
    auto& counter = registry.new_counter({"test", "console_reporter", "counter"});
    auto& histogram = registry.new_histogram({"test", "console_reporter", "histogram"});
    auto& meter = registry.new_meter({"test", "console_reporter", "meter"}, "cycles");
    auto& timer = registry.new_timer({"test", "console_reporter", "timer"});
    JsonReporter reporter {registry};
    counter.inc();
    for (auto i = 1; i <= 1000; i++) {
        auto t = timer.time_scope();
        histogram.update(i);
        meter.mark();
    }
    auto json = reporter.report();
    std::cerr << json << std::endl;
}


