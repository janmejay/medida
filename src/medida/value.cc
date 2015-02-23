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

    void Value::Process(MetricProcessor &processor) {
        processor.Process(*this);
    }

    void Value::clear() {
        value_.store(0);
    }

    std::int64_t Value::value() const {
        return value_;
    }

    void Value::Update(std::int64_t value) {
        value_.store(value);
    }
} // namespace medida
