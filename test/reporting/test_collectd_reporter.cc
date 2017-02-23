//
// Copyright (c) 2012 Daniel Lundin
//

#include "medida/reporting/collectd_reporter.h"

#include <thread>

#include <gtest/gtest.h>

#include "medida/metrics_registry.h"

using namespace medida;
using namespace medida::reporting;

#ifdef COLLECTD_BIN_PATH
#include <fstream>
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

int delete_file(const char* path, const struct stat*, int typeflag, struct FTW* ) {
    if (typeflag == FTW_F) {
        if (unlink(path) != 0) {
            std::cerr << "Collectd csv-out-dir cleanup failed ( path was: " << STR(COLLECTD_CSV_OUTDIR) << " ), error : " << strerror(errno) << "\n";
            return FTW_STOP;
        }
    }
    return FTW_CONTINUE;
}

void run_collectd_server(std::future<void>& stop, std::promise<bool>& result) {
    struct stat stat_csv_outdir;
    if (stat(STR(COLLECTD_CSV_OUTDIR), &stat_csv_outdir) == 0) {
        if (nftw(STR(COLLECTD_CSV_OUTDIR), delete_file, 10, 0) != 0) {
            std::cerr << "Collectd csv-out-dir cleanup failed ( path was: " << STR(COLLECTD_CSV_OUTDIR) << " ), error : " << strerror(errno) << "\n";
            result.set_value(false);
            return;
        }
    } else {
        std::cerr << "Had trouble stat-ing csv-outdir (" << STR(COLLECTD_CSV_OUTDIR) << "), err: '" << strerror(errno) << "', ignoring it\n";

    }
    bool all_good = true;
    auto pid = fork();
    if (pid == 0) {
        char* const prms[] = {strdup(STR(COLLECTD_BIN_PATH)), strdup("-C"), strdup(STR(COLLECTD_CONFIG_PATH)), strdup("-f"), nullptr};
        if (execve(STR(COLLECTD_BIN_PATH), prms, nullptr) != 0) {
            std::cerr << "Collectd exec failed ( path was: " << STR(COLLECTD_BIN_PATH) << " ), error : " << strerror(errno) << "\n";
        }
    } else {
        stop.wait();
        all_good &= (kill(pid, SIGKILL) == 0);
        if (all_good) {
            auto wait_ret_pid = waitpid(pid, NULL, 0);
            if (wait_ret_pid != pid) {
                all_good = false;
                std::cerr << "Wait for collectd process failed, error: " << strerror(errno) << "\n";
            }
        } else {
            std::cerr << "Failed to kill collectd process (pid: " << pid << ")\n";
        }
        result.set_value(all_good);
    }
}

static std::string date_now() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    char buffer[120];
    strftime(buffer, sizeof(buffer),"%Y-%m-%d", &tm);
    return std::string {buffer};
}

static void feed_comma_separated(char* buff, std::vector<std::string>& cells) {
    int i = 0, j = 0;
    for (; *(buff + i) != '\0'; i++) {
        if (*(buff + i) == ',') {
            *(buff + i) = '\0';
            cells.emplace_back(buff + j);
            *(buff + i) = ',';
            j = i + 1;
        }
    }
    cells.emplace_back(buff + j);
}

static void cells_of(const char* type_and_name, const std::string& date_str, std::vector<std::string>& cells) {
    char host[1024];
    EXPECT_EQ(0, gethostname(host, sizeof(host)));

    std::stringstream f_name;
    f_name << STR(COLLECTD_CSV_OUTDIR) << '/' << host << '/' << "test.reporter" << '/' << type_and_name << '-' << date_str;

    cells.clear();

    std::ifstream ifs(f_name.str(), std::ios_base::in);
    char cell[512];
    while (true) {
        ifs.getline(cell, sizeof(cell));
        if (ifs.good()) {
            feed_comma_separated(cell, cells);
        } else {
            break;
        }
    }
}

class CellReader {
    const std::vector<std::string>& cells;
    int col_count;
public:
    
    CellReader(const std::vector<std::string>& _cells, int _col_count) : cells(_cells), col_count(_col_count) {}
    ~CellReader() {}

    const std::string& operator[](std::pair<int, int> cell_id) {
        return cells[cell_id.first * col_count + cell_id.second];
    }
};


template <typename T> std::ostream& operator<<(std::ostream& os, const std::vector<T>& vals) {
    int first_time = true;
    os << "[";
    for (auto& v : vals) {
        if (first_time) {
            first_time = false;
        } else {
            os << ", ";
        }
        os << v;
    }
    os << "]";
    return os;
}

double atof(const std::string& str) {
    return std::atof(str.c_str());
}

#endif

TEST(CollectdReporterTest, foo) {
    MetricsRegistry registry {};
    auto& counter = registry.NewCounter({"test", "reporter", "mycounter"});
    auto& histogram = registry.NewHistogram({"test", "reporter", "myhistogram"});
    auto& meter = registry.NewMeter({"test", "reporter", "mymeter"}, "cycles");
    auto& timer = registry.NewTimer({"test", "reporter", "mytimer"});
    CollectdReporter reporter {registry, "localhost", 25826};
    for (auto i = 1; i <= 100; i++) {
        auto t = timer.TimeScope();
        counter.inc();
        histogram.Update(i);
        meter.Mark();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

#ifdef COLLECTD_BIN_PATH
    auto date_start = date_now();
    std::promise<void> do_stop;
    auto stop_fut = do_stop.get_future();
    std::promise<bool> all_good;
    std::thread collectd_server(run_collectd_server, std::ref(stop_fut), std::ref(all_good));
    std::this_thread::sleep_for(std::chrono::seconds(1));
#endif

    reporter.Run();
  
#ifdef COLLECTD_BIN_PATH
    auto meter_mean_rate = meter.mean_rate();
    auto min1_rate = meter.one_minute_rate();
    auto min5_rate = meter.five_minute_rate();
    auto min15_rate = meter.fifteen_minute_rate();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    do_stop.set_value();
    EXPECT_EQ(true, all_good.get_future().get());
    collectd_server.join();

    EXPECT_EQ(date_now(), date_start);

    std::vector<std::string> csv_cells;
    cells_of("medida_counter-mycounter.count", date_start, csv_cells);
    CellReader count_rdr(csv_cells, 2);
    EXPECT_EQ("count", (count_rdr[{0, 1}]));
    EXPECT_EQ("100.000000", (count_rdr[{1, 1}]));

    cells_of("medida_histogram-myhistogram", date_start, csv_cells);
    CellReader hist_rdr(csv_cells, 11);

    EXPECT_EQ("min", (hist_rdr[{0, 1}]));
    EXPECT_EQ("1.000000", (hist_rdr[{1, 1}]));

    EXPECT_EQ("max", (hist_rdr[{0, 2}]));
    EXPECT_EQ("100.000000", (hist_rdr[{1, 2}]));

    EXPECT_EQ("mean", (hist_rdr[{0, 3}]));
    EXPECT_EQ("50.500000", (hist_rdr[{1, 3}]));

    EXPECT_EQ("std_dev", (hist_rdr[{0, 4}]));
    EXPECT_EQ("29.011492", (hist_rdr[{1, 4}]));

    EXPECT_EQ("median", (hist_rdr[{0, 5}]));
    EXPECT_EQ("50.500000", (hist_rdr[{1, 5}]));

    EXPECT_EQ("75pct", (hist_rdr[{0, 6}]));
    EXPECT_EQ("75.750000", (hist_rdr[{1, 6}]));

    EXPECT_EQ("95pct", (hist_rdr[{0, 7}]));
    EXPECT_EQ("95.950000", (hist_rdr[{1, 7}]));

    EXPECT_EQ("98pct", (hist_rdr[{0, 8}]));
    EXPECT_EQ("98.980000", (hist_rdr[{1, 8}]));

    EXPECT_EQ("99pct", (hist_rdr[{0, 9}]));
    EXPECT_EQ("99.990000", (hist_rdr[{1, 9}]));

    EXPECT_EQ("999pct", (hist_rdr[{0, 10}]));
    EXPECT_EQ("100.000000", (hist_rdr[{1, 10}]));


    cells_of("medida_timer-mytimer.ms", date_start, csv_cells);
    CellReader tm_rdr(csv_cells, 11);

    auto ts = timer.GetSnapshot();

    EXPECT_EQ("min", (tm_rdr[{0, 1}]));
    EXPECT_NEAR(timer.min(), atof(tm_rdr[{1, 1}]), 0.001);

    EXPECT_EQ("max", (tm_rdr[{0, 2}]));
    EXPECT_NEAR(timer.max(), atof(tm_rdr[{1, 2}]), 0.001);

    EXPECT_EQ("mean", (tm_rdr[{0, 3}]));
    EXPECT_NEAR(timer.mean(), atof(tm_rdr[{1, 3}]), 0.001);

    EXPECT_EQ("std_dev", (tm_rdr[{0, 4}]));
    EXPECT_NEAR(timer.std_dev(), atof(tm_rdr[{1, 4}]), 0.001);

    EXPECT_EQ("median", (tm_rdr[{0, 5}]));
    EXPECT_NEAR(ts.getMedian(), atof(tm_rdr[{1, 5}]), 0.001);

    EXPECT_EQ("75pct", (tm_rdr[{0, 6}]));
    EXPECT_NEAR(ts.get75thPercentile(), atof(tm_rdr[{1, 6}]), 0.001);

    EXPECT_EQ("95pct", (tm_rdr[{0, 7}]));
    EXPECT_NEAR(ts.get95thPercentile(), atof(tm_rdr[{1, 7}]), 0.001);

    EXPECT_EQ("98pct", (tm_rdr[{0, 8}]));
    EXPECT_NEAR(ts.get98thPercentile(), atof(tm_rdr[{1, 8}]), 0.001);

    EXPECT_EQ("99pct", (tm_rdr[{0, 9}]));
    EXPECT_NEAR(ts.get99thPercentile(), atof(tm_rdr[{1, 9}]), 0.001);

    EXPECT_EQ("999pct", (tm_rdr[{0, 10}]));
    EXPECT_NEAR(ts.get999thPercentile(), atof(tm_rdr[{1, 10}]), 0.001);

    cells_of("medida_meter-mymeter.cycles_per_s", date_start, csv_cells);
    CellReader m_rdr(csv_cells, 6);

    EXPECT_EQ("count", (m_rdr[{0, 1}]));
    EXPECT_NEAR(100.0, atof(m_rdr[{1, 1}]), 0.001);

    EXPECT_EQ("mean_rate", (m_rdr[{0, 2}]));
    EXPECT_NEAR(meter_mean_rate, atof(m_rdr[{1, 2}]), 0.5);

    EXPECT_EQ("1min_rate", (m_rdr[{0, 3}]));
    EXPECT_NEAR(min1_rate, atof(m_rdr[{1, 3}]), 0.5);

    EXPECT_EQ("5min_rate", (m_rdr[{0, 4}]));
    EXPECT_NEAR(min5_rate, atof(m_rdr[{1, 4}]), 0.5);

    EXPECT_EQ("15min_rate", (m_rdr[{0, 5}]));
    EXPECT_NEAR(min15_rate, atof(m_rdr[{1, 5}]), 0.5);
#endif
  
}


