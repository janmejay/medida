//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_METRIC_NAME_H_
#define MEDIDA_METRIC_NAME_H_


#include <memory>
#include <string>
#include <ostream>

namespace medida {
    class MetricName {

    public:
        MetricName(const std::string &domain, const std::string &type, const std::string &name, const std::string &scope = "");

        MetricName(const MetricName& other);

        ~MetricName();

        const std::string& domain() const;

        const std::string& type() const;

        const std::string& name() const;

        const std::string& scope() const;

        const std::string& to_string() const;

        bool has_scope() const;

        bool operator==(const MetricName& other) const;

        bool operator!=(const MetricName& other) const;

        bool operator<(const MetricName& other) const;

        bool operator>(const MetricName& other) const;

    private:
        class Impl;

        std::unique_ptr<Impl> impl_;
    };
}

#endif // MEDIDA_METRIC_NAME_H_
