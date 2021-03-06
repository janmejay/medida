//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/stats/ewma.h"

#include <cmath>

namespace medida {
    namespace stats {
        static const int kINTERVAL = 5;
        static const double kSECONDS_PER_MINUTE = 60.0;
        static const int kONE_MINUTE = 1;
        static const int kFIVE_MINUTES = 5;
        static const int kFIFTEEN_MINUTES = 15;
        static const double kM1_ALPHA = 1 - std::exp(-kINTERVAL / kSECONDS_PER_MINUTE / kONE_MINUTE);
        static const double kM5_ALPHA = 1 - std::exp(-kINTERVAL / kSECONDS_PER_MINUTE / kFIVE_MINUTES);
        static const double kM15_ALPHA = 1 - std::exp(-kINTERVAL / kSECONDS_PER_MINUTE / kFIFTEEN_MINUTES);

        class EWMA::Impl {

        public:

            Impl(double alpha, std::chrono::nanoseconds interval);

            Impl(Impl &other);

            ~Impl();

            void update(std::int64_t n);

            void tick();

            double rate(std::chrono::nanoseconds duration = std::chrono::seconds {1}) const;

        private:
            bool initialized_;

            double rate_;

            std::int64_t uncounted_;

            const double alpha_;

            const std::int64_t interval_nanos_;
        };


        EWMA::EWMA(double alpha, std::chrono::nanoseconds interval) : impl_ {new EWMA::Impl {alpha, interval}} { }


        EWMA::EWMA(EWMA &&other) : impl_ {new EWMA::Impl{*other.impl_}} { }


        EWMA::~EWMA() { }


        EWMA EWMA::one_minute_ewma() {
            return {kM1_ALPHA, std::chrono::seconds{5}};
        }


        EWMA EWMA::five_minute_ewma() {
            return {kM5_ALPHA, std::chrono::seconds{5}};
        }


        EWMA EWMA::fifteen_minute_ewma() {
            return {kM15_ALPHA, std::chrono::seconds{5}};
        }


        void EWMA::update(std::int64_t n) {
            impl_->update(n);
        }


        void EWMA::tick() {
            impl_->tick();
        }


        double EWMA::rate(std::chrono::nanoseconds duration) const {
            return impl_->rate(duration);
        }


// === Implementation ===

        EWMA::Impl::Impl(double alpha, std::chrono::nanoseconds interval)
            : initialized_    {false},
              rate_           {0.0},
              uncounted_      {0},
              alpha_          {alpha},
              interval_nanos_ {interval.count()} { }


        EWMA::Impl::Impl(Impl &other)
            : initialized_    {other.initialized_},
              rate_           {other.rate_},
              uncounted_      {other.uncounted_},
              alpha_          {other.alpha_},
              interval_nanos_ {other.interval_nanos_} { }


        EWMA::Impl::~Impl() { }


        void EWMA::Impl::update(std::int64_t n) {
            uncounted_ += n;
        }


        void EWMA::Impl::tick() {
            double count = uncounted_;
            uncounted_ = 0;
            auto instant_rate = count / interval_nanos_;
            if (initialized_) {
                rate_ += (alpha_ * (instant_rate - rate_));
            } else {
                rate_ = instant_rate;
                initialized_ = true;
            }
        }


        double EWMA::Impl::rate(std::chrono::nanoseconds duration) const {
            return rate_ * duration.count();
        }
    }
}
