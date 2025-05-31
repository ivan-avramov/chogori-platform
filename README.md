<!--
    (C)opyright Futurewei Technologies Inc, 2019
-->

# chogori-platform
K2 Project is a platform for building low-latency (μs) in-memory distributed persistent OLTP databases.

This repository contains implementations for K2 core services and subsystems (transport, persistence, etc.).

For more interactive discussions, news, planning and other questions, please visit our discussion board here:
https://groups.google.com/forum/#!forum/chogori-dev

## Build instructions

### Install instructions
 * Install chogori-seastar-rd (see instructions in the repo https://github.com/futurewei-cloud/chogori-seastar-rd)
 * run `./install_deps.sh` to install other dependency libraries
 * build and install the cmake subprojects under src/logging and src/skvhttpclient
 * generate cmake and build `mkdir build && cd build && cmake .. && make -j`
 * run tests `cd build/test && ctest`
 * run integration tests `./test/integration/run.sh`

### Broken tests
* test_http_client.sh
* test_http_cpp_client.sh

* ss -tnplp
* 
``` shell
g++ -std=c++20 v.cpp    -I/usr/local/include -Wno-maybe-uninitialized -Wno-error=unused-result -DSEASTAR_P2581R1 -DSEASTAR_API_LEVEL=7 -DSEASTAR_SSTRING -DSEASTAR_DEPRECATED_OSTREAM_FORMATTERS -DSEASTAR_LOGGER_COMPILE_TIME_FMT -DSEASTAR_SCHEDULING_GROUPS_COUNT=16 -DSEASTAR_LOGGER_TYPE_STDOUT -DBOOST_PROGRAM_OPTIONS_NO_LIB -DBOOST_PROGRAM_OPTIONS_DYN_LINK -DBOOST_THREAD_NO_LIB -DBOOST_THREAD_DYN_LINK -DBOOST_ATOMIC_NO_LIB -DBOOST_ATOMIC_DYN_LINK -DFMT_SHARED -I/usr/include/p11-kit-1 /usr/local/lib/libseastar.a /usr/lib/aarch64-linux-gnu/libboost_program_options.so.1.83.0 /usr/lib/aarch64-linux-gnu/libboost_thread.so.1.83.0 /usr/lib/aarch64-linux-gnu/libcares.so /usr/lib/aarch64-linux-gnu/libfmt.so.10.1.0 -ldl /usr/lib/aarch64-linux-gnu/libboost_thread.so.1.83.0 /usr/lib/aarch64-linux-gnu/libsctp.so -latomic -llz4 -lxxhash -lgnutls -lgmp  -latomic -lnettle -lhogweed -lgmp -lnettle -ltasn1 -lidn2 -lp11-kit -lprotobuf -lz -lhwloc -lm -lpthread -lyaml-cpp  -I/usr/include/libnl3 -libverbs -lbnxt_re-rdmav34 -lcxgb4-rdmav34 -lefa -lerdma-rdmav34 -lhns -lirdma-rdmav34 -lmana -lmlx4 -lmlx5 -lmthca-rdmav34 -locrdma-rdmav34 -lqedr-rdmav34 -lvmw_pvrdma-rdmav34 -lhfi1verbs-rdmav34 -lipathverbs-rdmav34 -lrxe-rdmav34 -lsiw-rdmav34 -libverbs -lpthread -lnl-route-3 -lnl-3 -lpthread    -Wno-deprecated-declarations   -o multi_shard_server
```