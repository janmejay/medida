//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_REPORTING_ABSTRACT_POLLING_REPORTER_H_
#define MEDIDA_REPORTING_ABSTRACT_POLLING_REPORTER_H_

#include <memory>

#include "medida/types.h"

namespace medida {
    namespace reporting {
        class AbstractPollingReporter {

        public:
            AbstractPollingReporter();

            virtual ~AbstractPollingReporter();

            virtual void stop();

            // start should never be called after calling shutdown on this class
            //    (behavior if start after shutdown is undefined).
            virtual void start(Clock::duration period = std::chrono::seconds(5));

            virtual void run();

        private:
            class Impl;

            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif // MEDIDA_REPORTING_ABSTRACT_POLLING_REPORTER_H_
