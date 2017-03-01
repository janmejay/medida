//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_METRICS_SNAPSHOT_H_
#define MEDIDA_METRICS_SNAPSHOT_H_

#include <memory>
#include <vector>

namespace medida {
    namespace stats {
        class Snapshot {

        public:
            Snapshot(const std::vector<double>& values);

            ~Snapshot();

            Snapshot(Snapshot const&) = delete;

            Snapshot& operator=(Snapshot const&) = delete;

            Snapshot(Snapshot&&);

            std::size_t size() const;

            double quantile(double fraction) const;

            double median() const;

            double percentile_75() const;

            double percentile_95() const;

            double percentile_98() const;

            double percentile_99() const;

            double percentile_999() const;

            const std::vector<double>& values() const;

        private:
            class Impl;

            void check_impl() const;

            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif // MEDIDA_METRICS_SNAPSHOT_H_
