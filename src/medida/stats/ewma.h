//
// Copyright (c) 2012 Daniel Lundin
//

#ifndef MEDIDA_EWMA_H_
#define MEDIDA_EWMA_H_

#include <chrono>
#include <cstdint>
#include <memory>

namespace medida {
    namespace stats {
        // This class is _not_ thread safe.
        //    The user must ensure exclusive-access when calling into this.
        class EWMA {

        public:
            EWMA() = delete;

            EWMA(double alpha, std::chrono::nanoseconds interval);

            EWMA(EWMA &&other);

            ~EWMA();

            static EWMA one_minute_ewma();

            static EWMA five_minute_ewma();

            static EWMA fifteen_minute_ewma();

            void update(std::int64_t n);

            void tick();

            double rate(std::chrono::nanoseconds duration = std::chrono::seconds {1}) const;

        private:
            class Impl;

            std::unique_ptr<Impl> impl_;
        };

    }
}

#endif // MEDIDA_EWMA_H_
