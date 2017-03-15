# Build gtest from source
add_subdirectory (gtest-1.6.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(COLLECTD_BIN_PATH "NONE" CACHE STRING "'Path to collectd binary' which allows collectd integration-test to verify collectd accepted and handled the reported values correctly, defaults to 'NONE', which disables end-to-end testing of collectd reporting.")
set(COLLECTD_PLUGIN_DIR_PATH "NONE" CACHE STRING "Path to plugin-directory that has collectd plugins")
set(COLLECTD_WORK_DIR "/tmp/medida-collectd" CACHE STRING "Path to working-directory of collectd, defaults to /tmp/medida-collectd")
if (NOT ${COLLECTD_BIN_PATH} MATCHES "NONE")
  get_filename_component(COLLECTD_TYPES_DB "../etc/collectd/medida-types.db" ABSOLUTE)
  file(MAKE_DIRECTORY ${COLLECTD_WORK_DIR})
  set(COLLECTD_CONFIG_PATH "${COLLECTD_WORK_DIR}/collectd.conf")
  set(COLLECTD_CSV_OUTDIR "${COLLECTD_WORK_DIR}/csv.d")
  configure_file(reporting/collectd.conf.tmpl ${COLLECTD_CONFIG_PATH})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOLLECTD_BIN_PATH=${COLLECTD_BIN_PATH} -DCOLLECTD_CONFIG_PATH=${COLLECTD_CONFIG_PATH} -DCOLLECTD_CSV_OUTDIR=${COLLECTD_CSV_OUTDIR}")
endif()

set(test_sources
  test_counter.cc
  test_histogram.cc
  test_meter.cc
  test_metric_name.cc
  test_metrics_registry.cc
  test_timer.cc
  test_value.cc
  reporting/test_collectd_reporter.cc
  reporting/test_udp_reporter.cc
  reporting/test_console_reporter.cc
  reporting/test_json_reporter.cc
  stats/test_ewma.cc
  stats/test_exp_decay_sample.cc
  stats/test_snapshot.cc
  stats/test_uniform_sample.cc
)

add_executable(test-medida ${test_sources})

target_link_libraries(test-medida
  gtest
  gtest_main
  medida
)

add_test(test-medida test-medida --gtest_output=xml)
