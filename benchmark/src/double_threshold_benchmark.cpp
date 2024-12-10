#include "double_threshold.h"
#include "opencv2/core/base.hpp"
#include "opencv2/core/mat.hpp"
#include <exception>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <random>
#include <stdexcept>

#define MAX_FREQ 3.4
#define BASE_FREQ 2.4

static inline unsigned long long rdtsc(void) {
  unsigned hi, lo;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

void BenchmarkDoubleThreshold(int width, int height) {
  unsigned long long st;
  unsigned long long et;
  unsigned long long total = 0;
  unsigned long long referenceTotal = 0;
  int repeat = 1000;
  int matrixSize = width * height;

  int lower_bound = 0;
  int upper_bound = 256;
  double low_thres = 50;
  double high_thres = 100;

  std::uniform_int_distribution<int> unif(lower_bound, upper_bound);
  std::default_random_engine re;

  double *input = new double[matrixSize]();
  double *output = new double[matrixSize]();
  double *expected = new double[matrixSize]();
  // Generate a random input matrix
  for (int i = 0; i < matrixSize; i++) {
    input[i] = unif(re);
    output[i] = 0.0;
    expected[i] = 0.0;
  }

  double_threshold(input, output, width, height, low_thres, high_thres);
  double_threshold_slow(input, expected, width, height, low_thres, high_thres);

  // Check if the output is correct
  for (int i = 0; i < matrixSize; i++) {
    if (std::abs(output[i] - expected[i]) > 1e-6) {
      std::cout << "output[" << i << "] = " << output[i]
                << " expected: " << expected[i] << "\n";
      throw std::runtime_error("BenchmarkDoubleThreshold failed: incorrect "
                               "output from GaussianFilter");
    }
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    double_threshold(input, output, width, height, low_thres, high_thres);
    ;
    et = rdtsc();

    total += (et - st);
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    double_threshold_slow(input, expected, width, height, low_thres,
                          high_thres);
    et = rdtsc();

    referenceTotal += (et - st);
  }

  unsigned long long createFilterFLOPSPS = (4 + 6 + 1 + 1 + 1) * 9;
  unsigned long long kernalFLOPSPS =
      2 * 9 * width * height + createFilterFLOPSPS;

  std::cout << "Benchmarking matrix size: " << width << "x" << height << "\n";
  std::cout << "RDTSC Cycles Taken for double threshold: " << total << "\n";
  std::cout << "RDTSC Cycles Taken for slow double threshold: "
            << referenceTotal << "\n";
  std::cout << "FLOPS Per Cycle for double threshold: "
            << repeat * kernalFLOPSPS / (total * MAX_FREQ / BASE_FREQ) << "\n";
  std::cout << "FLOPS Per Cycle for slow double threshol: "
            << repeat * kernalFLOPSPS / (referenceTotal * MAX_FREQ / BASE_FREQ)
            << "\n";
}

int main(int argc, char *argv[]) {
  cv::setNumThreads(0);
  try {
    std::cout << "Running tests...\n";

    std::cout << "...Benchmarking double threshold...\n";
    BenchmarkDoubleThreshold(8, 8);
    BenchmarkDoubleThreshold(16, 16);
    BenchmarkDoubleThreshold(32, 32);
    BenchmarkDoubleThreshold(64, 64);
    BenchmarkDoubleThreshold(128, 128);
    BenchmarkDoubleThreshold(256, 256);
    BenchmarkDoubleThreshold(512, 512);
    BenchmarkDoubleThreshold(1024, 1024);

    std::cout << "All tests passed\n";
  } catch (const std::exception &err) {
    std::cerr << "[ERROR] " << err.what() << "\n";

    return -1;
  }

  return 0;
}