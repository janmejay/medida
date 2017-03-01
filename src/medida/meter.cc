//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/meter.h"

#include <atomic>
#include <mutex>

namespace medida {

    static const auto kTickInterval = Clock::duration(std::chrono::seconds(5)).count();

    class Meter::Impl {

    public:
        Impl(const std::string& event_type, std::chrono::nanoseconds rate_unit = std::chrono::seconds(1));

        ~Impl();

        std::chrono::nanoseconds rate_unit() const;

        const std::string& event_type() const;

        std::uint64_t count() const;

        double fifteen_minute_rate();

        double five_minute_rate();

        double one_minute_rate();

        double mean_rate();

        void mark(std::uint64_t n = 1);

        void process(MetricProcessor& processor);

    private:
        const std::string event_type_;

        const std::chrono::nanoseconds rate_unit_;

        std::atomic<std::uint64_t> count_;

        const Clock::time_point start_time_;

        std::atomic<std::int64_t> last_tick_;

        stats::EWMA m1_rate_;

        stats::EWMA m5_rate_;

        stats::EWMA m15_rate_;

        mutable std::mutex mutex_;

        void tick_if_necessary();
    };


    Meter::Meter(const std::string& event_type, std::chrono::nanoseconds rate_unit) : impl_ {new Meter::Impl {event_type, rate_unit}} { }


    Meter::~Meter() { }


    std::chrono::nanoseconds Meter::rate_unit() const {
        return impl_->rate_unit();
    }


    const std::string& Meter::event_type() const {
        return impl_->event_type();
    }


    std::uint64_t Meter::count() const {
        return impl_->count();
    }


    double Meter::fifteen_minute_rate() {
        return impl_->fifteen_minute_rate();
    }


    double Meter::five_minute_rate() {
        return impl_->five_minute_rate();
    }


    double Meter::one_minute_rate() {
        return impl_->one_minute_rate();
    }


    double Meter::mean_rate() {
        return impl_->mean_rate();
    }


    void Meter::mark(std::uint64_t n) {
        impl_->mark(n);
    }


    void Meter::process(MetricProcessor& processor) {
        processor.process(*this);  // FIXME: pimpl?
    }


// === Implementation ===


    Meter::Impl::Impl(const std::string& event_type, std::chrono::nanoseconds rate_unit) 
        : event_type_ (event_type),
          rate_unit_  (rate_unit),
          count_      (0),
          start_time_ (Clock::now()),
          last_tick_  (std::chrono::duration_cast<std::chrono::nanoseconds>(start_time_.time_since_epoch()).count()),
          m1_rate_    (stats::EWMA::one_minute_ewma()),
          m5_rate_    (stats::EWMA::five_minute_ewma()),
          m15_rate_   (stats::EWMA::fifteen_minute_ewma()),
          mutex_      {} { }


    Meter::Impl::~Impl() { }


    std::chrono::nanoseconds Meter::Impl::rate_unit() const {
        return rate_unit_;
    }


    const std::string& Meter::Impl::event_type() const {
        return event_type_;
    }


    std::uint64_t Meter::Impl::count() const {
        return count_.load();
    }


    double Meter::Impl::fifteen_minute_rate() {
        std::lock_guard<std::mutex> lock {mutex_};
        tick_if_necessary();
        return m15_rate_.rate();
    }


    double Meter::Impl::five_minute_rate() {
        std::lock_guard<std::mutex> lock {mutex_};
        tick_if_necessary();
        return m5_rate_.rate();
    }


    double Meter::Impl::one_minute_rate() {
        std::lock_guard<std::mutex> lock {mutex_};
        tick_if_necessary();
        return m1_rate_.rate();
    }


    double Meter::Impl::mean_rate() {
        double c = count_.load();
        if (c > 0) {
            std::chrono::nanoseconds elapsed = Clock::now() - start_time_;
            return c * rate_unit_.count() / elapsed.count();
        }
        return 0.0;
    }


    void Meter::Impl::mark(std::uint64_t n) {
        std::lock_guard<std::mutex> lock {mutex_};
        tick_if_necessary();
        count_ += n;
        m1_rate_.update(n);
        m5_rate_.update(n);
        m15_rate_.update(n);
    }


    //must always be called after acquiring exclusive lock
    void Meter::Impl::tick_if_necessary() {
        auto old_tick = last_tick_.load();
        auto new_tick = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
        auto age = new_tick - old_tick;
        if (age > kTickInterval) {
            last_tick_ = new_tick;
            auto required_ticks = age / kTickInterval;
            for (auto i = 0; i < required_ticks; i ++) {
                m1_rate_.tick();
                m5_rate_.tick();
                m15_rate_.tick();
            }
        }
    }

}
