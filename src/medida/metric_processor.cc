//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/metric_processor.h"

namespace medida {

    MetricProcessor::~MetricProcessor() { }

    void MetricProcessor::Process(Counter& ) { }

    void MetricProcessor::Process(Histogram& ) { }

    void MetricProcessor::Process(Value& ) { }

    void MetricProcessor::Process(Meter& ) { }

    void MetricProcessor::Process(Timer& ) { }


    void MetricProcessor::Process(MetricInterface& ) { }
}
