//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/stats/snapshot.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace medida {
    namespace stats {

        static const double kMEDIAN_Q = 0.5;
        static const double kP75_Q = 0.75;
        static const double kP95_Q = 0.95;
        static const double kP98_Q = 0.98;
        static const double kP99_Q = 0.99;
        static const double kP999_Q = 0.999;

        class Snapshot::Impl {

        public:

            Impl(const std::vector<double>& values);

            ~Impl();

            std::size_t size() const;

            double quantile(double fraction) const;

            double median() const;

            double percentile_75() const;

            double percentile_95() const;

            double percentile_98() const;

            double percentile_99() const;

            double percentile_999() const;

            const std::vector<double>& values() const;

        private:
            std::vector<double> values_;
        };


        Snapshot::Snapshot(const std::vector<double>& values) : impl_ {new Snapshot::Impl {values}} { }


        Snapshot::Snapshot(Snapshot&& other) : impl_ {std::move(other.impl_)} { }


        Snapshot::~Snapshot() { }


        void Snapshot::check_impl() const {
            if (!impl_) {
                throw std::runtime_error("Access to moved Snapshot::impl_");
            }
        }


        std::size_t Snapshot::size() const {
            check_impl();
            return impl_->size();
        }


        const std::vector<double>& Snapshot::values() const {
            check_impl();
            return impl_->values();
        }


        double Snapshot::quantile(double fraction) const {
            check_impl();
            return impl_->quantile(fraction);
        }


        double Snapshot::median() const {
            check_impl();
            return impl_->median();
        }


        double Snapshot::percentile_75() const {
            check_impl();
            return impl_->percentile_75();
        }


        double Snapshot::percentile_95() const {
            check_impl();
            return impl_->percentile_95();
        }


        double Snapshot::percentile_98() const {
            check_impl();
            return impl_->percentile_98();
        }


        double Snapshot::percentile_99() const {
            check_impl();
            return impl_->percentile_99();
        }


        double Snapshot::percentile_999() const {
            check_impl();
            return impl_->percentile_999();
        }

        
// === Implementation ===


        Snapshot::Impl::Impl(const std::vector<double>& values) : values_ (values) {
            std::sort(std::begin(this->values_), std::end(this->values_));
        }


        Snapshot::Impl::~Impl() { }


        std::size_t Snapshot::Impl::size() const {
            return values_.size();
        }


        const std::vector<double>& Snapshot::Impl::values() const {
            return values_;
        }


        double Snapshot::Impl::quantile(double fraction) const {
            if (fraction < 0.0 || fraction > 1.0) {
                throw std::invalid_argument("quantile is not in [0..1]");
            }

            if (values_.empty()) {
                return 0.0;
            }

            auto pos = fraction * (values_.size() + 1);

            if (pos < 1) {
                return values_.front();
            }

            if (pos >= values_.size()) {
                return values_.back();
            }

            double lower = values_[pos - 1];
            double upper = values_[pos];
            return lower + (pos - std::floor(pos)) * (upper - lower);
        }


        double Snapshot::Impl::median() const {
            return quantile(kMEDIAN_Q);
        }


        double Snapshot::Impl::percentile_75() const {
            return quantile(kP75_Q);
        }


        double Snapshot::Impl::percentile_95() const {
            return quantile(kP95_Q);
        }


        double Snapshot::Impl::percentile_98() const {
            return quantile(kP98_Q);
        }


        double Snapshot::Impl::percentile_99() const {
            return quantile(kP99_Q);
        }


        double Snapshot::Impl::percentile_999() const {
            return quantile(kP999_Q);
        }
    }
}
