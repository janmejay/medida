#include <cstdint>
#include <memory>
#include <atomic>
#include <cmath>
#include <mutex>

#include "medida/metric_interface.h"
#include "medida/sampling_interface.h"
#include "medida/summarizable_interface.h"
#include "medida/stats/sample.h"

namespace medida {

    class Value : public MetricInterface {
    public:
        Value();

        ~Value();

        void Update(std::int64_t value);

        std::int64_t value() const;

        void Process(MetricProcessor &processor);

        void clear();

    private:
        std::atomic<std::int64_t> value_;
    };

} // namespace medida
