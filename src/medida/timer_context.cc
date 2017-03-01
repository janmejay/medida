//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/timer_context.h"

#include "medida/timer.h"
#include "medida/types.h"

namespace medida {
    class TimerContext::Impl {

    public:

        Impl(Timer& timer);

        ~Impl();

        void reset();

        std::chrono::nanoseconds stop();

    private:

        Clock::time_point start_time_;

        Timer& timer_;

        bool active_;
    };


    TimerContext::TimerContext(TimerContext&& timer) : impl_ {std::move(timer.impl_)} { }

    TimerContext::TimerContext(Timer& timer) : impl_ {new TimerContext::Impl {timer}} { }


    TimerContext::~TimerContext() { }

    void TimerContext::check_impl() const {
        if (!impl_) {
            throw std::runtime_error("Access to moved TimerContext::impl_");
        }
    }

    void TimerContext::reset() {
        check_impl();
        impl_->reset();
    }

    std::chrono::nanoseconds TimerContext::stop() {
        check_impl();
        return impl_->stop();
    }


// === Implementation ===


    TimerContext::Impl::Impl(Timer& timer) : timer_ (timer) {  // FIXME: GCC Bug 50025 - Uniform initialization of reference members broken
        reset();
    }


    TimerContext::Impl::~Impl() {
        stop();
    }

    void TimerContext::Impl::reset() {
        start_time_ = Clock::now();
        active_ = true;
    }


    std::chrono::nanoseconds TimerContext::Impl::stop() {
        if (active_) {
            auto dur = Clock::now() - start_time_;
            timer_.update(dur);
            active_ = false;
            return dur;
        }
        return std::chrono::nanoseconds(0);
    }
}
