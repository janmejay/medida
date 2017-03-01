//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/reporting/util.h"

#include <cstdint>

namespace medida {
    namespace reporting {

        static const std::int64_t NS_PER_US =     1000,
            NS_PER_MS =     1000 * NS_PER_US,
            NS_PER_SECOND = 1000 * NS_PER_MS,
            NS_PER_MIN =      60 * NS_PER_SECOND,
            NS_PER_HOUR =     60 * NS_PER_MIN,
            NS_PER_DAY =      24 * NS_PER_HOUR;

        static const std::string DAY = "d",
            HOUR = "h",
            MINUTE = "m",
            SECOND = "s",
            MILLI_SECOND = "ms",
            MICRO_SECOND = "us",
            NANO_SECOND = "ns";


        const std::string& format_rate_unit(const std::chrono::nanoseconds& rate_unit) {
            auto nanosecs = rate_unit.count();
            if (nanosecs >= NS_PER_DAY) {
                return DAY;
            } else if (nanosecs >= NS_PER_HOUR) {
                return HOUR;
            } else if (nanosecs >= NS_PER_MIN) {
                return MINUTE;
            } else if (nanosecs >= NS_PER_SECOND) {
                return SECOND;
            } else if (nanosecs >= NS_PER_MS) {
                return MILLI_SECOND;
            } else if (nanosecs >= NS_PER_US) {
                return MICRO_SECOND;
            }
            return NANO_SECOND;
        }
    }
}
