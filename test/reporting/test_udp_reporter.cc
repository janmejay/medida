#include "medida/reporting/udp_reporter.h"

#include <thread>

#include <gtest/gtest.h>

#include "medida/metrics_registry.h"

extern "C" {
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>
}

using namespace medida;
using namespace medida::reporting;

#include <thread>
#include <future>
#include <tuple>
#include <string.h>
#include <ftw.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define STR_(x) #x
#define STR(x) STR_(x)

extern "C" int make_non_blocking(int fd) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        std::cerr << "Couldn't get fd %d status-flags for " << fd << "\n";
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        std::cerr << "Couldn't make fd " << fd << " non-blocking\n";
        return -1;
    }

    return 0;
}

extern "C" void run_udp_server(std::function<void(const char*, int)>& payload_handler, std::atomic<bool>& do_stop, std::promise<bool>& result) {
    bool all_good = true;
    char buff[1024];
    struct addrinfo hints, *res = NULL, *r;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    int ret = getaddrinfo("127.0.0.1", "8765", &hints, &res);
    if (ret != 0) {
        std::cerr << "Couldn't get addrinfo: " << strerror_r(ret, buff, sizeof(buff)) << "\n";
        result.set_value(false);
        return;
    }

    int on = 1;

    std::vector<int> sockets;

    for (r = res; r != NULL; r = r->ai_next) {
        int sock = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (sock < 0) {
            std::cerr << "Couldn't create socket: " << strerror_r(errno, buff, sizeof(buff)) << "\n";
            all_good = false;
            break;
        }

        sockets.push_back(sock);

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0) {
            std::cerr << "Couldn't enable reuse-addr on server sock: " << strerror_r(errno, buff, sizeof(buff)) << "\n";
            all_good = false;
            break;
        }

        if (make_non_blocking(sock) != 0) {
            all_good = false;
            break;
        }

        if (bind(sock, r->ai_addr, r->ai_addrlen) < 0) {
            std::cerr << "Failed to bind listener socket: " << sock << " error: " << strerror_r(errno, buff, sizeof(buff)) << "\n";
            all_good = false;
            break;
        }
    }

    freeaddrinfo(res);

    if (all_good) {
        nfds_t sock_count = sockets.size();

        struct pollfd *pfd = static_cast<struct pollfd*>(calloc(sock_count, sizeof(pollfd)));

        for (std::uint32_t i = 0; i < sock_count; i++) {
            pfd[i].fd = sockets[i];
            pfd[i].events |= POLLIN;
        }

        while (! do_stop.load()) {
            ret = poll(pfd, sock_count, 1000);
            if (ret < 0) {
                std::cerr << "Poll call failed: " << strerror_r(errno, buff, sizeof(buff)) << "\n";
                all_good = false;
                break;
            }

            for (std::uint32_t i = 0; i < sock_count; i++) {
                if (pfd[i].revents & POLLIN) {
                    auto read_sz = recv(pfd[i].fd, buff, sizeof(buff), 0);
                    payload_handler(buff, read_sz);
                }
            }
        }

        free(pfd);
    }

    for (auto sock : sockets) {
        close(sock);
    }

    result.set_value(all_good);
}

static double atof(const std::string& str) {
    return std::atof(str.c_str());
}

std::string value_of(const std::vector<std::string>& msgs, const std::string& fragment) {
    for (auto& msg : msgs) {
        if (msg.find(fragment) != std::string::npos) {
            auto idx = msg.rfind(" ");
            return msg.substr(idx + 1, std::string::npos);
        }
    }
    return "NOT FOUND";
}

class TestFormatter : public medida::reporting::UdpReporter::Formatter {
private:
    template <typename V> std::string format(UdpReporter::MetricType type, UdpReporter::PropName prop, V val) const {
        std::stringstream ss;
        ss << *name << "(" << type << " > " << prop << ") " << val;
        return ss.str();
    }

    template <typename V> std::string format(UdpReporter::MetricType type, UdpReporter::PropName prop, V val, UdpReporter::Unit unit) const {
        std::stringstream ss;
        ss << *name << "(" << type << " > " << prop << ")[" << unit << "] " << val;
        return ss.str();
    }

    template <typename V> std::string format(UdpReporter::MetricType type, UdpReporter::EventType evt_type, UdpReporter::PropName prop, V val, UdpReporter::Unit unit) const {
        std::stringstream ss;
        ss << *name << "(" << type << " > " << prop << ")[" << evt_type << "/" << unit << "] " << val;
        return ss.str();
    }

public:
    std::string operator()(UdpReporter::MetricType type, UdpReporter::PropName prop, std::uint64_t val) const {
        return format(type, prop, val);
    }
    std::string operator()(UdpReporter::MetricType type, UdpReporter::PropName prop, std::int64_t val) const {
        return format(type, prop, val);
    }
    std::string operator()(UdpReporter::MetricType type, UdpReporter::PropName prop, double val) const {
        return format(type, prop, val);
    }
    std::string operator()(UdpReporter::MetricType type, UdpReporter::PropName prop, std::uint64_t val, UdpReporter::Unit unit) const {
        return format(type, prop, val, unit);
    }
    std::string operator()(UdpReporter::MetricType type, UdpReporter::PropName prop, double val, UdpReporter::Unit unit) const {
        return format(type, prop, val, unit);
    }
    std::string operator()(UdpReporter::MetricType type, UdpReporter::EventType evt_typ, UdpReporter::PropName prop, std::uint64_t val, UdpReporter::Unit unit) const {
        return format(type, evt_typ, prop, val, unit);
    }
    std::string operator()(UdpReporter::MetricType type, UdpReporter::EventType evt_typ, UdpReporter::PropName prop, double val, UdpReporter::Unit unit) const {
        return format(type, evt_typ, prop, val, unit);
    }
};

TEST(UdpReporterTest, reporting) {
    MetricsRegistry registry {};
    auto& counter = registry.new_counter({"test", "reporter", "mycounter"});
    auto& histogram = registry.new_histogram({"test", "reporter", "myhistogram"});
    auto& meter = registry.new_meter({"test", "reporter", "mymeter"}, "cycles");
    auto& timer = registry.new_timer({"test", "reporter", "mytimer"});
    TestFormatter f;
    UdpReporter reporter {registry, f, 8765};
    for (auto i = 1; i <= 100; i++) {
        auto t = timer.time_scope();
        counter.inc();
        histogram.update(i);
        meter.mark();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::atomic<bool> do_stop { false };
    std::promise<bool> all_good;
    std::mutex msgs_mut;
    std::vector<std::string> msgs;
    std::function<void(const char*, int)> x = [&msgs, &msgs_mut](const char* buff, int sz) {
        std::lock_guard<std::mutex> l(msgs_mut);
        msgs.emplace_back(buff, sz);
    };
    std::thread udp_server(run_udp_server, std::ref(x), std::ref(do_stop), std::ref(all_good));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    reporter.run();

    auto meter_mean_rate = meter.mean_rate();
    auto meter_min1_rate = meter.one_minute_rate();
    auto meter_min5_rate = meter.five_minute_rate();
    auto meter_min15_rate = meter.fifteen_minute_rate();

    auto timer_mean_rate = timer.mean_rate();
    auto timer_min1_rate = timer.one_minute_rate();
    auto timer_min5_rate = timer.five_minute_rate();
    auto timer_min15_rate = timer.fifteen_minute_rate();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    do_stop.store(true);
    udp_server.join();
    EXPECT_EQ(true, all_good.get_future().get());

    // //debug aid
    // std::cout << "Messages...\n";
    // for (auto& msg : msgs) {
    //     std::cout << ">>>     " << msg << "\n";
    // }

    EXPECT_EQ("100", value_of(msgs, "test.reporter.mycounter(counter > count)"));

    EXPECT_NEAR(1.000000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > min)")), 0.5);
    EXPECT_NEAR(100.000000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > max)")), 0.5);
    EXPECT_NEAR(50.000000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > mean)")), 0.5);
    EXPECT_NEAR(29.011492, atof(value_of(msgs, "test.reporter.myhistogram(histogram > std_dev)")), 0.5);
    EXPECT_NEAR(50.500000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > median)")), 0.5);
    EXPECT_NEAR(75.750000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > 75pct)")), 0.5);
    EXPECT_NEAR(95.950000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > 95pct)")), 0.5);
    EXPECT_NEAR(98.980000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > 98pct)")), 0.5);
    EXPECT_NEAR(99.990000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > 99pct)")), 0.5);
    EXPECT_NEAR(100.000000, atof(value_of(msgs, "test.reporter.myhistogram(histogram > 999pct)")), 0.5);

    auto ts = timer.snapshot();
    EXPECT_NEAR(timer.min(), atof(value_of(msgs, "test.reporter.mytimer(timer > min)[ms]")), 0.1);
    EXPECT_NEAR(timer.max(), atof(value_of(msgs, "test.reporter.mytimer(timer > max)[ms]")), 0.1);
    EXPECT_NEAR(timer.mean(), atof(value_of(msgs, "test.reporter.mytimer(timer > mean)[ms]")), 0.1);
    EXPECT_NEAR(timer.std_dev(), atof(value_of(msgs, "test.reporter.mytimer(timer > std_dev)[ms]")), 0.1);
    EXPECT_NEAR(ts.median(), atof(value_of(msgs, "test.reporter.mytimer(timer > median)[ms]")), 0.1);
    EXPECT_NEAR(ts.percentile_75(), atof(value_of(msgs, "test.reporter.mytimer(timer > 75pct)[ms]")), 0.1);
    EXPECT_NEAR(ts.percentile_95(), atof(value_of(msgs, "test.reporter.mytimer(timer > 95pct)[ms]")), 0.1);
    EXPECT_NEAR(ts.percentile_98(), atof(value_of(msgs, "test.reporter.mytimer(timer > 98pct)[ms]")), 0.1);
    EXPECT_NEAR(ts.percentile_99(), atof(value_of(msgs, "test.reporter.mytimer(timer > 99pct)[ms]")), 0.1);
    EXPECT_NEAR(ts.percentile_999(), atof(value_of(msgs, "test.reporter.mytimer(timer > 999pct)[ms]")), 0.1);
    EXPECT_NEAR(timer_mean_rate, atof(value_of(msgs, "test.reporter.mytimer(timer > mean_rate)[calls/s]")), 0.5);
    EXPECT_NEAR(timer_min1_rate, atof(value_of(msgs, "test.reporter.mytimer(timer > 1min_rate)[calls/s]")), 0.5);
    EXPECT_NEAR(timer_min5_rate, atof(value_of(msgs, "test.reporter.mytimer(timer > 5min_rate)[calls/s]")), 0.5);
    EXPECT_NEAR(timer_min15_rate, atof(value_of(msgs, "test.reporter.mytimer(timer > 15min_rate)[calls/s]")), 0.5);

    EXPECT_NEAR(100.0, atof(value_of(msgs, "test.reporter.mymeter(meter > count)")), 0.001);
    EXPECT_NEAR(meter_mean_rate, atof(value_of(msgs, "test.reporter.mymeter(meter > mean_rate)[cycles/s]")), 0.5);
    EXPECT_NEAR(meter_min1_rate, atof(value_of(msgs, "test.reporter.mymeter(meter > 1min_rate)[cycles/s]")), 0.5);
    EXPECT_NEAR(meter_min5_rate, atof(value_of(msgs, "test.reporter.mymeter(meter > 5min_rate)[cycles/s]")), 0.5);
    EXPECT_NEAR(meter_min15_rate, atof(value_of(msgs, "test.reporter.mymeter(meter > 15min_rate)[cycles/s]")), 0.5);
}


