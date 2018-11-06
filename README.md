# IndexZoo

[![GitHub license](https://img.shields.io/badge/license-apache-green.svg?style=flat)](https://www.apache.org/licenses/LICENSE-2.0)
<!-- [![Build Status](https://travis-ci.org/yingjunwu/IndexZoo.svg?branch=master)](https://travis-ci.org/yingjunwu/IndexZoo) -->

## Overview
The IndexZoo project aims at developing a general platform for performance evaluation of index structures in modern main-memory database systems.



## Installation

IndexZoo has been tested under the following platforms:

- Ubuntu 16.04

To use the platform, please run the following scripts:
```
sudo apt-get install libjemalloc-dev
git clone --recursive https://github.com/yingjunwu/IndexZoo.git
mkdir build
cmake ..
make -j
cd build
./src/index_benchmark -h
```

## Benchmarks

Currently, IndexZoo supports both numeric- and string-based workloads.

To run numeric-based workloads, please try:

```
./src/index_benchmark -h
```

To run string-based workloads, please try:

```
./src/generic_index_benchmark -h
```

## Supported Index Structures

### Multi-thread Dynamic Index Structures (for OLTP workloads)

| Index Structure | Description | Original Implementation | Note  |
|:---------------:|:------:|:-----------------------:|:-----:|
| Masstree Index  | [J. Rao, et al.](https://dl.acm.org/citation.cfm?id=2168855) | [kohler](https://github.com/kohler/masstree-beta)                   | |
| ArtTree Index   | [V. Leis, et al.](https://dl.acm.org/citation.cfm?id=2933349.2933352) | [flode](https://github.com/flode/ARTSynchronized) | |
| BwTree Index    | [J. Levandoski, et al.](https://dl.acm.org/citation.cfm?id=2510649.2511251) | [wangziqi2013](https://github.com/wangziqi2013/BwTree) | |
| Libcuckoo Index | [X. Li, et al.](https://dl.acm.org/citation.cfm?id=2592820) | [efficient](https://github.com/efficient/libcuckoo) | Hash-based index |

### Single-thread Dynamic Index Structures (for OLTP workloads)

| Index Structure | Description | Original Implementation | Note  |
|:---------------:|:------:|:-----------------------:|:-----:|
| Stx-Btree Index |  | [bingmann](https://github.com/bingmann/stx-btree) | Standard B+-tree index |
| ArtTree Index   | [V. Leis, et al.](https://db.in.tum.de/~leis/papers/ART.pdf) | [armon](https://github.com/armon/libart) | |

### Static Index Structures (for OLAP workloads)

| Index Structure     | Description | Original Implementation | Note  |
|:-------------------:|:------:|:-----------------------:|:-----:|
| Binary Index        | | [yingjunwu]() | Optimized binary search tree |
| KAry Index          | [B. Schlegel, et al.](https://dl.acm.org/citation.cfm?id=1565705) | [yingjunwu]() | |
| Interpolation Index | | [yingjunwu]() | |
| FAST Index          | [C. Kim, et al.](https://dl.acm.org/citation.cfm?id=1807206) | [yingjunwu]() | |




## License

Copyright (c) 2018 [Yingjun Wu](https://yingjunwu.github.io/)

Licensed Under the Apache 2.0 License.
