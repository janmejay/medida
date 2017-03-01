//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_METRIC_INTERFACE_H_
#define MEDIDA_METRIC_INTERFACE_H_

#include "medida/metric_processor.h"

namespace medida {
    class MetricInterface {

    public:
        virtual ~MetricInterface() {};

        virtual void process(MetricProcessor& processor) = 0;
    };
}

#endif // MEDIDA_METRIC_INTERFACE_H_
