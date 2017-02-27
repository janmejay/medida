//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_TIMER_CONTEXT_H_
#define MEDIDA_TIMER_CONTEXT_H_

#include <chrono>
#include <memory>

//This must not be shared across threads.
//    It doesn't make sense to make this thread-safe, because
//    sharing it across threads makes no sense.
//    But in case someone comes up with a legit case to share it,
//    the solution would be to make a thread-safe version of this.
namespace medida {
    class Timer;

    class TimerContext {

    public:
        TimerContext(Timer& timer);

        TimerContext(TimerContext &&);

        TimerContext(TimerContext const&) = delete;

        TimerContext& operator=(TimerContext const&) = delete;

        ~TimerContext();

        void Reset();

        std::chrono::nanoseconds Stop();

    private:
        class Impl;

        void checkImpl() const;

        std::unique_ptr<Impl> impl_;
    };

}

#endif // MEDIDA_TIMER_CONTEXT_H_
