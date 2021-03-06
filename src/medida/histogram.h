//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_HISTOGRAM_H_
#define MEDIDA_HISTOGRAM_H_

#include <cstdint>
#include <memory>

#include "medida/metric_interface.h"
#include "medida/sampling_interface.h"
#include "medida/summarizable_interface.h"
#include "medida/stats/sample.h"

namespace medida {
    class Histogram : public MetricInterface, SamplingInterface, SummarizableInterface {

    public:
        Histogram(SampleType sample_type = kUniform);

        ~Histogram();

        virtual stats::Snapshot snapshot() const;

        virtual double sum() const;

        virtual double max() const;

        virtual double min() const;

        virtual double mean() const;

        virtual double std_dev() const;

        void update(std::int64_t value);

        std::uint64_t count() const;

        double variance() const;

        void process(MetricProcessor& processor);

        void clear();

        const std::string& attribute_signature() const;

    private:
        class Impl;

        std::unique_ptr<Impl> impl_;
    };
}

#endif // MEDIDA_HISTOGRAM_H_
