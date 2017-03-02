//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/value.h"

namespace medida {
    Value::Value() {
        clear();
    }

    Value::~Value() {
    }

    void Value::process(MetricProcessor &processor) {
        processor.process(*this);
    }

    void Value::clear() {
        value_.store(0);
    }

    double Value::value() const {
        return value_;
    }

    void Value::update(double value) {
        value_.store(value, std::memory_order_relaxed);
    }
}
