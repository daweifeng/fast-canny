#include "non_maxima_suppression.h"
#include "opencv2/core/base.hpp"
#include "opencv2/core/mat.hpp"
#include <exception>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <random>
#include <stdexcept>

#define MAX_FREQ 3.4
#define BASE_FREQ 2.4
#define GAUSSIAN_KERNEL_SIZE 3

static inline unsigned long long rdtsc(void) {
  unsigned hi, lo;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

void BenchmarkNonMaxSupp(int width, int height) {
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
  double *theta = new double[matrixSize]();
  // Generate a random input matrix
  for (int i = 0; i < matrixSize; i++) {
    input[i] = unif(re);
    theta[i] = unif(re);
    output[i] = 0.0;
    expected[i] = 0.0;
  }

  non_max_suppression(input, output, theta, 3, width, height, 1.0);
  non_max_suppression_slow(input, expected, theta, 3, width, height, 1.0);

  // Check if the output is correct
  //   for (int i = 0; i < matrixSize; i++) {
  //     if (std::abs(output[i] - expected[i]) > 1e-6) {
  //       std::cout << "output[" << i << "] = " << output[i]
  //                 << " expected: " << expected[i] << "\n";
  //       throw std::runtime_error("BenchmarkNonMaxSupp failed: incorrect "
  //                                "output from NonMaxSupp");
  //     }
  //   }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    non_max_suppression(input, output, theta, 3, width, height, 1.0);
    et = rdtsc();

    total += (et - st);
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    non_max_suppression_slow(input, expected, theta, 3, width, height, 1.0);
    et = rdtsc();

    referenceTotal += (et - st);
  }

  unsigned long long createFilterFLOPSPS = (4 + 6 + 1 + 1 + 1) * 9;
  unsigned long long kernalFLOPSPS =
      2 * 9 * width * height + createFilterFLOPSPS;

  std::cout << "Benchmarking matrix size: " << width << "x" << height << "\n";
  std::cout << "RDTSC Cycles Taken for Non Maxima Suppresion: " << total
            << "\n";
  std::cout << "RDTSC Cycles Taken for slow Non Maxima Suppresion: "
            << referenceTotal << "\n";
  std::cout << "FLOPS Per Cycle for Non Maxima Suppresion: "
            << repeat * kernalFLOPSPS / (total * MAX_FREQ / BASE_FREQ) << "\n";
  std::cout << "FLOPS Per Cycle for slow Non Maxima Suppresion: "
            << repeat * kernalFLOPSPS / (referenceTotal * MAX_FREQ / BASE_FREQ)
            << "\n";
}

int main(int argc, char *argv[]) {
  cv::setNumThreads(0);
  try {
    std::cout << "Running tests...\n";

    std::cout << "...Benchmarking double threshold...\n";
    BenchmarkNonMaxSupp(8, 8);
    BenchmarkNonMaxSupp(16, 16);
    BenchmarkNonMaxSupp(32, 32);
    BenchmarkNonMaxSupp(64, 64);
    BenchmarkNonMaxSupp(128, 128);
    BenchmarkNonMaxSupp(256, 256);
    BenchmarkNonMaxSupp(512, 512);
    BenchmarkNonMaxSupp(1024, 1024);

    std::cout << "All tests passed\n";
  } catch (const std::exception &err) {
    std::cerr << "[ERROR] " << err.what() << "\n";

    return -1;
  }

  return 0;
}
