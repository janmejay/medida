//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_METRIC_INTERFACE_H_
#define MEDIDA_METRIC_INTERFACE_H_

#include "medida/metric_processor.h"

namespace medida {
    const std::string blank = "";
    class MetricInterface {

    public:
        virtual ~MetricInterface() {};

        virtual void process(MetricProcessor& processor) = 0;

        virtual const std::string& attribute_signature() const { return blank; };
    };
}

#endif // MEDIDA_METRIC_INTERFACE_H_
