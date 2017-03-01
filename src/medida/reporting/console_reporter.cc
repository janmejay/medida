//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/reporting/console_reporter.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include "util.h"

namespace medida {
    namespace reporting {
        class ConsoleReporter::Impl {

        public:
            Impl(ConsoleReporter& self, MetricsRegistry &registry, std::ostream& out = std::cerr);

            ~Impl();

            void run();

            void process(Counter& counter);

            void process(Meter& meter);

            void process(Histogram& histogram);

            void process(Timer& timer);

        private:
            ConsoleReporter& self_;

            medida::MetricsRegistry& registry_;

            std::ostream& out_;
        };


        ConsoleReporter::ConsoleReporter(MetricsRegistry &registry, std::ostream& out)
            : AbstractPollingReporter(),
              impl_ {new ConsoleReporter::Impl {*this, registry, out}} { }


        ConsoleReporter::~ConsoleReporter() { }


        void ConsoleReporter::run() {
            impl_->run();
        }


        void ConsoleReporter::process(Counter& counter) {
            impl_->process(counter);
        }


        void ConsoleReporter::process(Meter& meter) {
            impl_->process(meter);
        }


        void ConsoleReporter::process(Histogram& histogram) {
            impl_->process(histogram);
        }


        void ConsoleReporter::process(Timer& timer) {
            impl_->process(timer);
        }


// === Implementation ===


        ConsoleReporter::Impl::Impl(ConsoleReporter& self, MetricsRegistry &registry, std::ostream& out)
            : self_     (self),
              registry_ (registry),
              out_      (out) { }


        ConsoleReporter::Impl::~Impl() { }


        void ConsoleReporter::Impl::run() {
            for (auto& kv : registry_.get_all_metrics()) {
                auto name = kv.first;
                auto metric = kv.second;
                out_ << name.to_string() << ":" << std::endl;
                metric->process(self_);
            }
            out_ << std::endl;
        }


        void ConsoleReporter::Impl::process(Counter& counter) {
            out_ << "  count = " << counter.count() << std::endl;
        }


        void ConsoleReporter::Impl::process(Meter& meter) {
            auto event_type = meter.event_type();
            auto unit = format_rate_unit(meter.rate_unit());
            out_ << "           count = " << meter.count() << std::endl
                 << "       mean rate = " << meter.mean_rate() << " " << event_type << "/" << unit << std::endl
                 << "   1-minute rate = " << meter.one_minute_rate() << " " << event_type << "/" << unit << std::endl
                 << "   5-minute rate = " << meter.five_minute_rate() << " " << event_type << "/" << unit << std::endl
                 << "  15-minute rate = " << meter.fifteen_minute_rate() << " " << event_type << "/" << unit << std::endl;
        }


        void ConsoleReporter::Impl::process(Histogram& histogram) {
            auto snapshot = histogram.snapshot();
            out_ << "             min = " << histogram.min() << std::endl
                 << "             max = " << histogram.max() << std::endl
                 << "            mean = " << histogram.mean() << std::endl
                 << "          stddev = " << histogram.std_dev() << std::endl
                 << "          median = " << snapshot.median() << std::endl
                 << "             75% = " << snapshot.percentile_75() << std::endl
                 << "             95% = " << snapshot.percentile_95() << std::endl
                 << "             98% = " << snapshot.percentile_98() << std::endl
                 << "             99% = " << snapshot.percentile_99() << std::endl
                 << "           99.9% = " << snapshot.percentile_999() << std::endl;
        }


        void ConsoleReporter::Impl::process(Timer& timer) {
            auto snapshot = timer.snapshot();
            auto event_type = timer.event_type();
            auto unit = format_rate_unit(timer.duration_unit());
            out_ << "           count = " << timer.count() << std::endl
                 << "       mean rate = " << timer.mean_rate() << " " << event_type << "/" << unit << std::endl
                 << "   1-minute rate = " << timer.one_minute_rate() << " " << event_type << "/" << unit << std::endl
                 << "   5-minute rate = " << timer.five_minute_rate() << " " << event_type << "/" << unit << std::endl
                 << "  15-minute rate = " << timer.fifteen_minute_rate() << " " << event_type << "/" << unit << std::endl
                 << "             min = " << timer.min() << unit << std::endl
                 << "             max = " << timer.max() << unit << std::endl
                 << "            mean = " << timer.mean() << unit << std::endl
                 << "          stddev = " << timer.std_dev() << unit << std::endl
                 << "          median = " << snapshot.median() << unit << std::endl
                 << "             75% = " << snapshot.percentile_75() << unit << std::endl
                 << "             95% = " << snapshot.percentile_95() << unit << std::endl
                 << "             98% = " << snapshot.percentile_98() << unit << std::endl
                 << "             99% = " << snapshot.percentile_99() << unit << std::endl
                 << "           99.9% = " << snapshot.percentile_999() << unit << std::endl;
        }
    }
}
