#include "medida/medida.h"
#include <thread>

int main() {
  medida::MetricsRegistry registry;
  auto& counter = registry.NewCounter({"counter", "bar", "baz"});
  auto& histogram = registry.NewHistogram({"histogram", "bar", "baz"}, medida::SamplingInterface::kUniform);
  auto &timer = registry.NewTimer({"timer", "bar", "baz"});
  auto &meter = registry.NewMeter({"meter", "bar", "baz"}, "things");
  auto &value = registry.NewValue({"value", "bar", "baz"});

  medida::reporting::CollectdReporter collectdReporter(registry);
  collectdReporter.Start(std::chrono::seconds(1));

  for (int i = 0; i < 10000000; ++i) {
      long ts = rand() % 556;
      counter.inc();
      histogram.Update(ts);
      value.Update(ts);
      std::this_thread::sleep_for(std::chrono::microseconds(ts));
      meter.Mark(ts);
      auto context = timer.TimeScope();
      std::this_thread::sleep_for(std::chrono::microseconds(ts));
  }
  
  collectdReporter.Shutdown();
  return 0;
}
