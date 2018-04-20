#pragma once

#include <iostream>
#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

class TimeMeasurer {
public:
  TimeMeasurer() {}
  ~TimeMeasurer() {}

  void tic() {
    start_time_ = high_resolution_clock::now();
  }

  void toc() {
    end_time_ = high_resolution_clock::now();
  }

  long long time_ms() const {
    return std::chrono::duration_cast<milliseconds>(end_time_ - start_time_).count();
  }

  long long time_us() const {
    return std::chrono::duration_cast<microseconds>(end_time_ - start_time_).count();
  }

  long long time_ns() const {
    return std::chrono::duration_cast<nanoseconds>(end_time_ - start_time_).count();
  }

  void print_ms() const {
    std::cout << std::chrono::duration_cast<milliseconds>(end_time_ - start_time_).count() << " ms" << std::endl;
  }

  void print_us() const {
    std::cout << std::chrono::duration_cast<microseconds>(end_time_ - start_time_).count() << " us" << std::endl;
  }

  void print_ns() const {
    std::cout << std::chrono::duration_cast<nanoseconds>(end_time_ - start_time_).count() << " ns" << std::endl;
  }


private:
  TimeMeasurer(const TimeMeasurer&);
  TimeMeasurer& operator=(const TimeMeasurer&);

private:
  high_resolution_clock::time_point start_time_;
  high_resolution_clock::time_point end_time_;
};
