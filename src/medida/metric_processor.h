//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_METRIC_PROCESSOR_H_
#define MEDIDA_METRIC_PROCESSOR_H_

namespace medida {
    class Counter;
    class Histogram;
    class Value;
    class Meter;
    class MetricInterface;
    class Timer;

    class MetricProcessor {

    public:
        virtual ~MetricProcessor();

        virtual void process(Counter& counter);

        virtual void process(Histogram& histogram);

        virtual void process(Value& value);

        virtual void process(Meter& meter);

        virtual void process(Timer& timer);

        virtual void process(MetricInterface& metric);
    };
}

#endif // MEDIDA_METRIC_PROCESSOR_H_
