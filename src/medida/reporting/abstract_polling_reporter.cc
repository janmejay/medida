//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/reporting/abstract_polling_reporter.h"

#include <atomic>
#include <iostream>
#include <thread>

namespace medida {
    namespace reporting {
        class AbstractPollingReporter::Impl {

        public:
            Impl(AbstractPollingReporter& self);

            ~Impl();

            void stop();

            void start(Clock::duration period = std::chrono::seconds(5));

        private:
            AbstractPollingReporter& self_;

            std::atomic<bool> running_;

            std::thread thread_;

            void loop(Clock::duration period);
        };


        AbstractPollingReporter::AbstractPollingReporter() : impl_ {new AbstractPollingReporter::Impl {*this}} { }


        AbstractPollingReporter::~AbstractPollingReporter() { }


        void AbstractPollingReporter::stop() {
            impl_->stop();
        }


        void AbstractPollingReporter::start(Clock::duration period) {
            impl_->start(period);
        }


        void AbstractPollingReporter::run() { }


// === Implementation ===


        AbstractPollingReporter::Impl::Impl(AbstractPollingReporter& self)
            : self_ (self),
              running_ {false} { }


        AbstractPollingReporter::Impl::~Impl() {
            stop();
        }


        void AbstractPollingReporter::Impl::stop() {
            bool expected_old_value = true;
            if (running_.compare_exchange_strong(expected_old_value, false)) {
                thread_.join();
            }
        }


        void AbstractPollingReporter::Impl::start(Clock::duration period) {
            bool expected_old_value = false;
            if (running_.compare_exchange_strong(expected_old_value, true)) {
                thread_ = std::thread(&AbstractPollingReporter::Impl::loop, this, period);
            }
        }


        void AbstractPollingReporter::Impl::loop(Clock::duration period) {
            while (running_) {
                std::this_thread::sleep_for(period);
                self_.run();
            }
        }
    }
}
