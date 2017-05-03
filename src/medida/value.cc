//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/value.h"

namespace medida {
    class Value::Impl {

    public:
        Impl();

        ~Impl();

        void update(double value);

        double value() const;

        void clear();

        const std::string& attribute_signature() const;

    private:
        const std::string attr_sig_;

        std::atomic<double> value_;
    };

    Value::Value() : impl_(new Value::Impl()) { }


    Value::~Value() { }


    void Value::process(MetricProcessor &processor) {
        processor.process(*this);
    }


    void Value::clear() {
        impl_->clear();
    }


    double Value::value() const {
        return impl_->value();
    }


    void Value::update(double value) {
        impl_->update(value);
    }


    const std::string& Value::attribute_signature() const {
        return impl_->attribute_signature();
    }


// === Implementation ===


    Value::Impl::Impl() : attr_sig_(""), value_(0) {}


    Value::Impl::~Impl() {}


    void Value::Impl::update(double value) {
        value_.store(value, std::memory_order_relaxed);
    }


    double Value::Impl::value() const {
        return value_.load(std::memory_order_relaxed);
    }


    void Value::Impl::clear() {
        value_.store(0, std::memory_order_relaxed);
    }


    const std::string& Value::Impl::attribute_signature() const {
        return attr_sig_;
    }
}
