//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/reporting/console_reporter.h"

#include <thread>

#include <gtest/gtest.h>

#include "medida/metrics_registry.h"

using namespace medida;
using namespace medida::reporting;


TEST(ConsoleReporterTest, foo) {
    MetricsRegistry registry {};
    auto& counter = registry.new_counter({"test", "console_reporter", "counter"});
    auto& histogram = registry.new_histogram({"test", "console_reporter", "histogram"});
    auto& meter = registry.new_meter({"test", "console_reporter", "meter"}, "cycles");
    auto& timer = registry.new_timer({"test", "console_reporter", "timer"});
    ConsoleReporter reporter {registry};
    counter.inc();
    for (auto i = 1; i <= 1000; i++) {
        auto t = timer.time_scope();
        histogram.update(i);
        meter.mark();
    }
    reporter.start(std::chrono::milliseconds(300));
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
}


