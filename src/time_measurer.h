#pragma once

#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

class TimeMeasurer {
public:
  TimeMeasurer(){}
  ~TimeMeasurer(){}

  void tic(){
    start_time_ = high_resolution_clock::now();
  }

  void toc(){
    end_time_ = high_resolution_clock::now();
  }

  long long time_ms(){
    return std::chrono::duration_cast<milliseconds>(end_time_ - start_time_).count();
  }

  long long time_us(){
    return std::chrono::duration_cast<microseconds>(end_time_ - start_time_).count();
  }

  long long time_ns(){
    return std::chrono::duration_cast<nanoseconds>(end_time_ - start_time_).count();
  }


private:
  TimeMeasurer(const TimeMeasurer&);
  TimeMeasurer& operator=(const TimeMeasurer&);

private:
  high_resolution_clock::time_point start_time_;
  high_resolution_clock::time_point end_time_;
};
