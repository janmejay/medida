Medida - Simple metrics/runtime-stats for C++ programs
======================================================

This library aims to make it easy for applications to manage and publish runtime-stats / metrics that allow easy monitoring of application health and behavior.

It supports the following data-types:
        
* Counter
* Histogram
* Meter
* Timer
* Value

It supports the following reporters as of now:

* CollectD: Pushes data to collectd over UDP, needs network plugin loaded on the collectd side.
* Console:  Prints data to stdout.
* JSON:     Returns json-serialized data, lets caller choose how to export it to out.
* UDP:      Pushes data over UDP with externalized formatting.

Please check <project root>/examples/medida_example1.cc for a brief intro to it.

