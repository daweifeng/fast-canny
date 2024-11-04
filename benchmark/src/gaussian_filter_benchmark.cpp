#include "gaussian_filter.h"
#include "opencv2/core/base.hpp"
#include "opencv2/core/mat.hpp"
#include <exception>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <random>
#include <stdexcept>

#define MAX_FREQ 3.4
#define BASE_FREQ 2.4
#define NUM_IMAGES_PER_SIZE 10
#define GAUSSIAN_KERNEL_SIZE 3
#define GAUSSIAN_KERNEL_SIGMA 0.5
#define CANNY_GRADIENT_LOWER_THRESHOLD 100
#define CANNY_GRADIENT_UPPER_THRESHOLD 200

static inline unsigned long long rdtsc(void) {
  unsigned hi, lo;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

void BenchmarkGaussianFilterSlow(int width, int height) {
  unsigned long long st;
  unsigned long long et;
  unsigned long long total = 0;
  unsigned long long referenceTotal = 0;
  int repeat = 1000;
  int matrixSize = width * height;

  int lower_bound = 0;
  int upper_bound = 256;

  std::uniform_int_distribution<int> unif(lower_bound, upper_bound);

  std::default_random_engine re;

  double *input = new double[matrixSize]();
  double *output = new double[matrixSize]();

  for (int i = 0; i < matrixSize; i++) {
    input[i] = unif(re);
    output[i] = 0.0;
  }

  cv::Mat src(height, width, CV_64F, input);
  cv::Mat expected;

  GaussianFilterSlow(input, output, GAUSSIAN_KERNEL_SIZE, width, height,
                     GAUSSIAN_KERNEL_SIGMA);
  cv::GaussianBlur(src, expected,
                   cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE),
                   GAUSSIAN_KERNEL_SIGMA, cv::BORDER_CONSTANT, 0);

  for (int i = 0; i < matrixSize; i++) {
    if (std::abs(output[i] - expected.at<double>(i)) > 1e-6) {
      std::cout << "output[" << i << "] = " << output[i]
                << " expected: " << expected.at<double>(i) << "\n";
      throw std::runtime_error("GaussianFilterSlow failed");
    }
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    GaussianFilterSlow(input, output, GAUSSIAN_KERNEL_SIZE, 3, 3,
                       GAUSSIAN_KERNEL_SIGMA);
    et = rdtsc();

    total += (et - st);
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    cv::GaussianBlur(src, expected,
                     cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE),
                     GAUSSIAN_KERNEL_SIGMA, cv::BORDER_CONSTANT, 0);
    et = rdtsc();

    referenceTotal += (et - st);
  }

  std::cout << "Benchmarking matrix size: " << width << "x" << height << "\n";
  std::cout << "RDTSC Cycles Taken for GaussianFilterSlow: " << total << "\n";
  std::cout << "RDTSC Cycles Taken for cv::GaussianBlur: " << referenceTotal
            << "\n";
  // std::cout << "Total FLOPS: " << totalFLOPS << "\n";
  // std::cout << "FLOPS Per Cycle: " << totalFLOPS / (sum * MAX_FREQ /
  // BASE_FREQ)
  //           << "\n";

  delete[] input;
  delete[] output;
}

void TestGaussianFilterSlowCorrectness(int width, int height) {
  int matrixSize = width * height;
  double *input = new double[matrixSize]();
  double *output = new double[matrixSize]();

  int lower_bound = 0;
  int upper_bound = 256;

  std::uniform_int_distribution<int> unif(lower_bound, upper_bound);
  std::default_random_engine re;

  for (int i = 0; i < matrixSize; i++) {
    input[i] = unif(re);
    output[i] = 0.0;
  }

  cv::Mat src(height, width, CV_64F, input);
  cv::Mat expected;

  GaussianFilterSlow(input, output, GAUSSIAN_KERNEL_SIZE, width, height,
                     GAUSSIAN_KERNEL_SIGMA);
  cv::GaussianBlur(src, expected,
                   cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE),
                   GAUSSIAN_KERNEL_SIGMA, cv::BORDER_CONSTANT, 0);

  for (int i = 0; i < matrixSize; i++) {
    if (std::abs(output[i] - expected.at<double>(i)) > 1e-6) {
      std::cout << "output[" << i << "] = " << output[i]
                << " expected: " << expected.at<double>(i) << "\n";
      std::cout << "width: " << width << " height: " << height << "\n";
      throw std::runtime_error("TestGaussianFilterSlowCorrectness failed");
    }
  }

  delete[] input;
  delete[] output;
}

int main(int argc, char *argv[]) {
  try {
    TestGaussianFilterSlowCorrectness(3, 3);
    TestGaussianFilterSlowCorrectness(8, 8);
    TestGaussianFilterSlowCorrectness(32, 32);
    TestGaussianFilterSlowCorrectness(1024, 1024);

    BenchmarkGaussianFilterSlow(8, 8);
    BenchmarkGaussianFilterSlow(16, 16);
    BenchmarkGaussianFilterSlow(32, 32);
    BenchmarkGaussianFilterSlow(64, 64);
    BenchmarkGaussianFilterSlow(1024, 1024);

    std::cout << "All tests passed\n";
  } catch (const std::exception &err) {
    std::cerr << "[ERROR] " << err.what() << "\n";

    return -1;
  }

  return 0;
}
