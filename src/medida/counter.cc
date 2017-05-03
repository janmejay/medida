//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/counter.h"

#include <atomic>
#include <sstream>

namespace medida {
    class Counter::Impl {

    public:
        Impl(std::int64_t init = 0);
        
        ~Impl();
        
        void process(MetricProcessor& processor);
        
        std::int64_t count() const;
        
        void set_count(std::int64_t n);
        
        void inc(std::int64_t n = 1);
        
        void dec(std::int64_t n = 1);
        
        void clear();

        const std::string& attribute_signature() const;
        
    private:
        std::atomic<std::int64_t> count_;

        std::string attr_sig_;
    };


    Counter::Counter(std::int64_t init) : impl_ {new Counter::Impl {init}} { }


    Counter::~Counter() { }


    void Counter::process(MetricProcessor& processor)  {
        processor.process(*this);  // FIXME: pimpl?
    }


    std::int64_t Counter::count() const {
        return impl_->count();
    }


    void Counter::set_count(std::int64_t n) {
        return impl_->set_count(n);
    }


    void Counter::inc(std::int64_t n) {
        impl_->inc(n);
    }


    void Counter::dec(std::int64_t n) {
        impl_->dec(n);
    }


    void Counter::clear() {
        impl_->clear();
    }

    const std::string& Counter::attribute_signature() const {
        return impl_->attribute_signature();
    }


// === Implementation ===


    Counter::Impl::Impl(std::int64_t init) : count_ {init} {
        std::stringstream ss;
        ss << "initial_value='" <<count_ << "'";
        attr_sig_ = ss.str();
    }


    Counter::Impl::~Impl() { }


    std::int64_t Counter::Impl::count() const {
        return count_.load(std::memory_order_relaxed);
    }


    void Counter::Impl::set_count(std::int64_t n) {
        count_.store(n, std::memory_order_relaxed);
    }


    void Counter::Impl::inc(std::int64_t n) {
        count_.fetch_add(n, std::memory_order_relaxed);
    }


    void Counter::Impl::dec(std::int64_t n) {
        count_.fetch_sub(n, std::memory_order_relaxed);
    }


    void Counter::Impl::clear() {
        set_count(0);
    }

    const std::string& Counter::Impl::attribute_signature() const {
        return attr_sig_;
    }
}
