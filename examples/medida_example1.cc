#include "medida/medida.h"
#include <thread>

int main() {
  medida::MetricsRegistry registry;
  auto& counter = registry.new_counter({"counter", "bar", "baz"});
  auto& histogram = registry.new_histogram({"histogram", "bar", "baz"}, medida::SamplingInterface::kUniform);
  auto &timer = registry.new_timer({"timer", "bar", "baz"});
  auto &meter = registry.new_meter({"meter", "bar", "baz"}, "things");
  auto &value = registry.new_value({"value", "bar", "baz"});

  medida::reporting::CollectdReporter collectdReporter(registry);
  collectdReporter.start(std::chrono::seconds(1));

  for (int i = 0; i < 10000000; ++i) {
      long ts = rand() % 556;
      counter.inc();
      histogram.update(ts);
      value.update(ts);
      std::this_thread::sleep_for(std::chrono::microseconds(ts));
      meter.mark(ts);
      auto context = timer.time_scope();
      std::this_thread::sleep_for(std::chrono::microseconds(ts));
  }
  
  collectdReporter.stop();
  return 0;
}
