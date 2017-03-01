//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/metric_name.h"

#include <stdexcept>

namespace medida {
    class MetricName::Impl {

    public:
        Impl(const std::string &domain, const std::string &type, const std::string &name, const std::string &scope = "");

        ~Impl();

        const std::string& domain() const;

        const std::string& type() const;

        const std::string& name() const;

        const std::string& scope() const;

        const std::string& to_string() const;

        bool has_scope() const;

        bool operator==(const Impl& other) const;

        bool operator!=(const Impl& other) const;

        bool operator<(const Impl& other) const;

        bool operator>(const Impl& other) const;

    private:
        const std::string domain_;

        const std::string type_;

        const std::string name_;

        const std::string scope_;

        const std::string repr_;
    };


    MetricName::MetricName(const std::string &domain, const std::string &type,
                           const std::string &name, const std::string &scope)
        : impl_ {new MetricName::Impl {domain, type, name, scope}} { }


    MetricName::MetricName(const MetricName& other) : impl_ {new MetricName::Impl(*other.impl_)} { }


    MetricName::~MetricName() { }


    const std::string& MetricName::domain() const {
        return impl_->domain();
    }


    const std::string& MetricName::type() const {
        return impl_->type();
    }


    const std::string& MetricName::name() const {
        return impl_->name();
    }


    const std::string& MetricName::scope() const {
        return impl_->scope();
    }


    const std::string& MetricName::to_string() const {
        return impl_->to_string();
    }


    bool MetricName::has_scope() const {
        return impl_->has_scope();
    }


    bool MetricName::operator==(const MetricName &other) const {
        return *impl_ == *other.impl_;
    }


    bool MetricName::operator!=(const MetricName &other) const {
        return *impl_ != *other.impl_;
    }


    bool MetricName::operator<(const MetricName& other) const {
        return *impl_ < *other.impl_;
    }


    bool MetricName::operator>(const MetricName& other) const {
        return *impl_ > *other.impl_;
    }


// === Implementation ===


    MetricName::Impl::Impl(const std::string &domain, const std::string &type,
                           const std::string &name, const std::string &scope)
        : domain_ (domain),
          type_   (type),
          name_   (name),
          scope_  (scope),
          repr_   (domain + "." + type + "." + name  + (scope.empty() ? "" : "." + scope)) {
        if (domain.empty()) {
            throw std::invalid_argument("domain must be non-empty");
        }
        if (type.empty()) {
            throw std::invalid_argument("type must be non-empty");
        }
        if (name.empty()) {
            throw std::invalid_argument("name must be non-empty");
        }
    }


    MetricName::Impl::~Impl() { }


    const std::string& MetricName::Impl::domain() const {
        return domain_;
    }


    const std::string& MetricName::Impl::type() const {
        return type_;
    }


    const std::string& MetricName::Impl::name() const {
        return name_;
    }


    const std::string& MetricName::Impl::scope() const {
        return scope_;
    }


    const std::string& MetricName::Impl::to_string() const {
        return repr_;
    }


    bool MetricName::Impl::has_scope() const {
        return !scope_.empty();
    }


    bool MetricName::Impl::operator==(const Impl &other) const {
        return repr_ == other.repr_;
    }


    bool MetricName::Impl::operator!=(const Impl &other) const {
        return repr_ != other.repr_;
    }


    bool MetricName::Impl::operator<(const Impl& other) const {
        return repr_ < other.repr_;
    }


    bool MetricName::Impl::operator>(const Impl& other) const {
        return repr_ > other.repr_;
    }
}
