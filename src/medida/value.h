#ifndef MEDIDA_VALUE_H_
#define MEDIDA_VALUE_H_

#include <cstdint>
#include <memory>
#include <atomic>
#include <cmath>
#include <mutex>

#include "medida/metric_interface.h"
#include "medida/sampling_interface.h"
#include "medida/summarizable_interface.h"
#include "medida/stats/sample.h"

namespace medida {
    class Value : public MetricInterface {

    public:
        Value();

        ~Value();

        void update(double value);

        double value() const;

        void process(MetricProcessor &processor);

        void clear();

        const std::string& attribute_signature() const;

    private:
        class Impl;

        std::unique_ptr<Impl> impl_;
    };
}

#endif // MEDIDA_VALUE_H_
