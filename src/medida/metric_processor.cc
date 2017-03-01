//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/metric_processor.h"

namespace medida {

    MetricProcessor::~MetricProcessor() { }

    void MetricProcessor::process(Counter& ) { }

    void MetricProcessor::process(Histogram& ) { }

    void MetricProcessor::process(Value& ) { }

    void MetricProcessor::process(Meter& ) { }

    void MetricProcessor::process(Timer& ) { }

    void MetricProcessor::process(MetricInterface& ) { }
}
