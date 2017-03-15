#ifndef MEDIDA_REPORTING_UDP_REPORTER_H_
#define MEDIDA_REPORTING_UDP_REPORTER_H_

#include <cstdint>
#include <memory>
#include <string>

#include "medida/metrics_registry.h"
#include "medida/metric_processor.h"
#include "medida/reporting/abstract_polling_reporter.h"

namespace medida {
    namespace reporting {
        class UdpReporter : public AbstractPollingReporter {

        public:
            typedef const std::string& MetricName;

            typedef const std::string& MetricType;
            typedef const std::string& PropName;
            typedef const std::string& Unit;
            typedef const std::string& EventType;

            class Formatter {
            protected:
                mutable const std::string* name;
            public:
                void set_name(MetricName _name) const { name = &_name; }
                virtual std::string operator()(MetricType type, PropName prop, std::uint64_t val) const = 0;
                virtual std::string operator()(MetricType type, PropName prop, std::int64_t val) const = 0;
                virtual std::string operator()(MetricType type, PropName prop, double val) const = 0;
                virtual std::string operator()(MetricType type, PropName prop, std::uint64_t val, Unit unit) const = 0;
                virtual std::string operator()(MetricType type, PropName prop, double val, Unit unit) const = 0;
                virtual std::string operator()(MetricType type, EventType evt_typ, PropName prop, std::uint64_t val, Unit unit) const = 0;
                virtual std::string operator()(MetricType type, EventType evt_typ, PropName prop, double val, Unit unit) const = 0;
            };

            UdpReporter(MetricsRegistry &registry, const Formatter& formatter, std::uint16_t port, const std::string& hostname = "127.0.0.1", const std::uint32_t reconnect_after = 8192);

            virtual ~UdpReporter();

            virtual void run();

        private:
            class Impl;

            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif // MEDIDA_REPORTING_UDP_REPORTER_H_
