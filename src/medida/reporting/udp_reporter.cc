#include "medida/reporting/udp_reporter.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include "util.h"
#include <cstring>
#include <cassert>

extern "C" {
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
}


namespace medida {
    namespace reporting {
        class UdpSender {//_NOT_ thread safe

            std::string host_;
            std::uint16_t port_;
            std::uint32_t reconnect_after_;
            std::uint64_t messages_sent_;
            bool connected_;

            std::vector<int> connections_;

            char buff[256];

            const char* err_str(int err_code) {
                return strerror_r(err_code, buff, sizeof(buff));
            }

            void disconnect() {
                assert(connected_);
                connected_ = false;

                for (int conn : connections_) {
                    close(conn);
                }
                connections_.clear();
            }

            void reconnect() {
                assert(! connected_);

                struct addrinfo hints, *res = NULL, *r;
                memset(&hints, 0, sizeof(hints));
                hints.ai_family = AF_INET;
                hints.ai_socktype = SOCK_DGRAM;
                auto port_str = std::to_string(port_);
                int ret = getaddrinfo(host_.c_str(), port_str.c_str(), &hints, &res);
                if (ret != 0) {
                    std::cerr << "Couldn't get addrinfo: " << err_str(ret) << "\n";
                    return;
                }

                for (r = res; r != NULL; r = r->ai_next) {
                    int sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
                    if (sock < 0) {
                        int err = errno;
                        std::cerr << "Couldn't create socket: " << err_str(err) << "\n";
                        continue;
                    }

                    if (connect(sock, r->ai_addr, r->ai_addrlen) < 0) {
                        int err = errno;
                        std::cerr << "Failed to connect socket: " << sock << " error: " << err_str(err) << "\n";
                        close(sock);
                        continue;
                    }

                    connections_.push_back(sock);
                }

                freeaddrinfo(res);

                connected_ = true;
            }

            void do_send(const char* msg, int len) {
                if (! connected_) return;
                for (auto conn : connections_) {
                    auto sent = ::send(conn, msg, len, 0);
                    if (sent < 0) {
                        int err = errno;
                        std::cerr << "Failed to report metric (errno: " << err << ", error: '"  << err_str(err) << "', fd: " << conn << ")\n";
                    } else assert(sent == len);
                }
            }

        public:
            UdpSender(const std::string& host, std::uint16_t port, std::uint32_t reconnect_after) :
                host_(host), port_(port), reconnect_after_(reconnect_after), messages_sent_(0), connected_(false) { }

            ~UdpSender() { if (connected_) disconnect(); }

            void send(std::string&& msg) {
                if (connected_ && ((messages_sent_++ % reconnect_after_) == 0)) {
                    disconnect();
                }
                if (! connected_) reconnect();
                do_send(msg.c_str(), msg.length());
            }
        };

        class UdpReporter::Impl : public MetricProcessor {

        public:
            Impl(MetricsRegistry &registry, Formatter& formatter, std::uint16_t port, const std::string& hostname, std::uint32_t reconnect_after);

            ~Impl();

            void run();

            using MetricProcessor::process;

            void process(Counter& counter);

            void process(Value& value);

            void process(Meter& meter);

            void process(Histogram& histogram);

            void process(Timer& timer);

            template <typename... Args> void send(Args... args);

        private:
            medida::MetricsRegistry& registry_;

            Formatter& format_;

            UdpSender sender_;
        };


        UdpReporter::UdpReporter(MetricsRegistry &registry, Formatter& formatter, std::uint16_t port, const std::string& hostname, std::uint32_t reconnect_after)
            : AbstractPollingReporter(),
              impl_ {new UdpReporter::Impl {registry, formatter, port, hostname, reconnect_after}} { }


        UdpReporter::~UdpReporter() { }


        void UdpReporter::run() {
            impl_->run();
        }


// === Implementation ===


        UdpReporter::Impl::Impl(MetricsRegistry &registry, Formatter& formatter, std::uint16_t port, const std::string& hostname, std::uint32_t reconnect_after)
            :registry_ (registry), format_ (formatter), sender_(hostname, port, reconnect_after) { }


        UdpReporter::Impl::~Impl() { }


        void UdpReporter::Impl::run() {
            for (auto& kv : registry_.get_all_metrics()) {
                auto name = kv.first;
                auto metric = kv.second;
                format_.set_name(name.to_string());
                metric->process(*this);
            }
        }


        template <typename... Args> void UdpReporter::Impl::send(Args... args) {
            sender_.send(format_(args...));
        }


        void UdpReporter::Impl::process(Counter& counter) {
            send("counter", "count", counter.count());
        }

        void UdpReporter::Impl::process(Value& value) {
            send("tracker", "value", value.value());
        }

        void UdpReporter::Impl::process(Meter& meter) {
            auto event_type = meter.event_type();
            auto unit = format_rate_unit(meter.rate_unit());
            send("meter", "count", meter.count());
            send("meter", event_type, "mean_rate", meter.mean_rate(), unit);
            send("meter", event_type, "1min_rate", meter.one_minute_rate(), unit);
            send("meter", event_type, "5min_rate", meter.five_minute_rate(), unit);
            send("meter", event_type, "15min_rate", meter.fifteen_minute_rate(), unit);
        }


        void UdpReporter::Impl::process(Histogram& histogram) {
            auto snapshot = histogram.snapshot();
            send("histogram", "min", histogram.min());
            send("histogram", "max", histogram.max());
            send("histogram", "mean", histogram.mean());
            send("histogram", "std_dev", histogram.std_dev());
            send("histogram", "median", snapshot.median());
            send("histogram", "75pct", snapshot.percentile_75());
            send("histogram", "95pct", snapshot.percentile_95());
            send("histogram", "98pct", snapshot.percentile_98());
            send("histogram", "99pct", snapshot.percentile_99());
            send("histogram", "999pct", snapshot.percentile_999());
        }


        void UdpReporter::Impl::process(Timer& timer) {
            auto snapshot = timer.snapshot();
            auto event_type = timer.event_type();
            auto rate_unit = format_rate_unit(timer.rate_unit());
            auto duration_unit = format_rate_unit(timer.duration_unit());

            send("timer", "count", timer.count());
            send("timer", event_type, "mean_rate", timer.mean_rate(), rate_unit);
            send("timer", event_type, "1min_rate", timer.one_minute_rate(), rate_unit);
            send("timer", event_type, "5min_rate", timer.five_minute_rate(), rate_unit);
            send("timer", event_type, "15min_rate", timer.fifteen_minute_rate(), rate_unit);
            send("timer", "min", timer.min(), duration_unit);
            send("timer", "max", timer.max(), duration_unit);
            send("timer", "mean", timer.mean(), duration_unit);
            send("timer", "std_dev", timer.std_dev(), duration_unit);
            send("timer", "median", snapshot.median(), duration_unit);
            send("timer", "75pct", snapshot.percentile_75(), duration_unit);
            send("timer", "95pct", snapshot.percentile_95(), duration_unit);
            send("timer", "98pct", snapshot.percentile_98(), duration_unit);
            send("timer", "99pct", snapshot.percentile_99(), duration_unit);
            send("timer", "999pct", snapshot.percentile_999(), duration_unit);
        }
    }
}
