#include "gradient.h"
#include "opencv2/core.hpp"
#include "opencv2/core/base.hpp"
#include "opencv2/core/mat.hpp"
#include <cstdlib>
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
};

void BenchmarkGradientSlow(int width, int height) {
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
  double *theta = new double[matrixSize]();

  for (int i = 0; i < matrixSize; i++) {
    input[i] = unif(re);
    output[i] = 0.0;
  }

  // initialize arrays
  cv::Mat expected, grad_x, grad_y, absGradX, absGradY;
  cv::Mat src(height, width, CV_64F, input);

  // calculate magnitude
  cv::Sobel(src, grad_x, CV_64F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
  cv::Sobel(src, grad_y, CV_64F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

  cv::Mat magnitude, angle;
  cv::cartToPolar(grad_x, grad_y, magnitude, angle, false);

  for (int y = 0; y < angle.rows; y++) {
    for (int x = 0; x < angle.cols; x++) {
      double currentAngle = angle.at<double>(y, x);
      if (currentAngle >= CV_PI) {
        angle.at<double>(y, x) -= 2 * CV_PI;
      }
    }
  }

  GradientSlow(input, output, theta, width, height);

  for (int i = 0; i < matrixSize; i++) {
    if (std::abs(output[i] - magnitude.at<double>(i)) > 1e-4) {
      std::cout << "output[" << i << "] = " << output[i]
                << " expected: " << magnitude.at<double>(i) << "\n";
      throw std::runtime_error("BenchmarkGradientSlow  failed: incorrect "
                               "output from GradientSlow");
    }
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    cv::Mat expected, grad_x, grad_y, absGradX, absGradY;
    cv::Mat src(height, width, CV_64F, input);

    // calculate magnitude
    cv::Sobel(src, grad_x, CV_64F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
    cv::Sobel(src, grad_y, CV_64F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

    cv::Mat magnitude, angle;
    cv::cartToPolar(grad_x, grad_y, magnitude, angle, false);

    for (int y = 0; y < angle.rows; y++) {
      for (int x = 0; x < angle.cols; x++) {
        double currentAngle = angle.at<double>(y, x);
        if (currentAngle >= CV_PI) {
          angle.at<double>(y, x) -= 2 * CV_PI;
        }
      }
    }
    et = rdtsc();

    referenceTotal += (et - st);
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    GradientSlow(input, output, theta, width, height);
    et = rdtsc();

    total += (et - st);
  }

  unsigned long long createFilterFLOPSPS = (4 + 6 + 1 + 1 + 1) * 9;
  unsigned long long kernalFLOPSPS =
      2 * 9 * width * height + createFilterFLOPSPS;

  std::cout << "Benchmarking matrix size: " << width << "x" << height << "\n";
  std::cout << "RDTSC Cycles Taken for GradientSlow: " << total << "\n";
  std::cout << "RDTSC Cycles Taken for cv::Sobel: " << referenceTotal << "\n";
  std::cout << "FLOPS Per Cycle for GradientSlow: "
            << repeat * kernalFLOPSPS / (total * MAX_FREQ / BASE_FREQ) << "\n";
  std::cout << "FLOPS Per Cycle for cv::Sobel: "
            << repeat * kernalFLOPSPS / (referenceTotal * MAX_FREQ / BASE_FREQ)
            << "\n";

  delete[] input;
  delete[] output;
  delete[] theta;
}

void BenchmarkGradient(int width, int height) {
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
  double *theta = new double[matrixSize]();

  // Generate a random input matrix
  for (int i = 0; i < matrixSize; i++) {
    input[i] = unif(re);
    output[i] = 0.0;
  }

  // initialize arrays
  cv::Mat expected, grad_x, grad_y, absGradX, absGradY;
  cv::Mat src(height, width, CV_64F, input);

  // calculate magnitude
  cv::Sobel(src, grad_x, CV_64F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
  cv::Sobel(src, grad_y, CV_64F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

  cv::Mat magnitude, angle;
  cv::cartToPolar(grad_x, grad_y, magnitude, angle, false);

  for (int y = 0; y < angle.rows; y++) {
    for (int x = 0; x < angle.cols; x++) {
      double currentAngle = angle.at<double>(y, x);
      if (currentAngle >= CV_PI) {
        angle.at<double>(y, x) -= 2 * CV_PI;
      }
    }
  }

  // TODO: change this to fast call
  Gradient(input, output, theta, width, height);

  // Check if the output is correct
  for (int i = 0; i < matrixSize; i++) {
    if (std::abs(output[i] - magnitude.at<double>(i)) > 1e-3) {
      std::cout << "output[" << i << "] = " << output[i]
                << " expected: " << magnitude.at<double>(i) << "\n";

      std::cout << "width: " << width << " height: " << height << "\n";
      throw std::runtime_error("BenchmarkGradient failed: incorrect "
                               "output from Gradient");
    }
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    Gradient(input, output, theta, width, height);
    et = rdtsc();

    total += (et - st);
  }

  for (int i = 0; i != repeat; ++i) {
    st = rdtsc();
    // initialize arrays
    cv::Mat expected, grad_x, grad_y, absGradX, absGradY;
    cv::Mat src(height, width, CV_64F, input);

    // calculate magnitude
    cv::Sobel(src, grad_x, CV_64F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
    cv::Sobel(src, grad_y, CV_64F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

    cv::Mat magnitude, angle;
    cv::cartToPolar(grad_x, grad_y, magnitude, angle, false);

    for (int y = 0; y < angle.rows; y++) {
      for (int x = 0; x < angle.cols; x++) {
        double currentAngle = angle.at<double>(y, x);
        if (currentAngle >= CV_PI) {
          angle.at<double>(y, x) -= 2 * CV_PI;
        }
      }
    }
    et = rdtsc();

    referenceTotal += (et - st);
  }

  unsigned long long createFilterFLOPSPS = (4 + 6 + 1 + 1 + 1) * 9;
  unsigned long long kernalFLOPSPS =
      2 * 9 * width * height + createFilterFLOPSPS;

  std::cout << "Benchmarking matrix size: " << width << "x" << height << "\n";
  std::cout << "RDTSC Cycles Taken for Gradient: " << total << "\n";
  std::cout << "RDTSC Cycles Taken for cv::Sobel: " << referenceTotal << "\n";
  std::cout << "FLOPS Per Cycle for Gradient: "
            << repeat * kernalFLOPSPS / (total * MAX_FREQ / BASE_FREQ) << "\n";
  std::cout << "FLOPS Per Cycle for cv::Sobel: "
            << repeat * kernalFLOPSPS / (referenceTotal * MAX_FREQ / BASE_FREQ)
            << "\n";

  delete[] input;
  delete[] output;
  delete[] theta;
}

void TestGradientSlowCorrectness(int width, int height) {
  int matrixSize = width * height;
  double *input = new double[matrixSize]();
  double *output = new double[matrixSize]();
  double *theta = new double[matrixSize]();

  int lower_bound = 0;
  int upper_bound = 256;

  std::uniform_int_distribution<int> unif(lower_bound, upper_bound);
  std::default_random_engine re;

  for (int i = 0; i < matrixSize; i++) {
    input[i] = unif(re);
    output[i] = 0.0;
  }

  // initialize arrays
  cv::Mat expected, grad_x, grad_y, absGradX, absGradY;
  cv::Mat src(height, width, CV_64F, input);

  cv::Sobel(src, grad_x, CV_64F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
  cv::Sobel(src, grad_y, CV_64F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

  cv::Mat magnitude, angle;
  cv::cartToPolar(grad_x, grad_y, magnitude, angle, false);

  for (int y = 0; y < angle.rows; y++) {
    for (int x = 0; x < angle.cols; x++) {
      double currentAngle = angle.at<double>(y, x);
      if (currentAngle >= CV_PI) {
        angle.at<double>(y, x) -= 2 * CV_PI;
      }
    }
  }

  GradientSlow(input, output, theta, width, height);

  for (int i = 0; i < matrixSize; i++) {
    if (std::abs(output[i] - magnitude.at<double>(i)) > 1e-3) {
      std::cout << "output[" << i << "] = " << output[i]
                << " expected: " << magnitude.at<double>(i)
                << "diff: " << std::abs(output[i] - magnitude.at<double>(i))
                << "\n";
      std::cout << "width: " << width << " height: " << height << "\n";
      throw std::runtime_error("TestGradientSlowCorrectness failed");
    }

    if (std::abs(theta[i] - angle.at<double>(i)) > 1e-3) {
      std::cout << "theta[" << i << "] = " << theta[i]
                << " expected: " << angle.at<double>(i) << "\n";
      std::cout << "width: " << width << " height: " << height << "\n";
      throw std::runtime_error("TestGradientSlowCorrectness failed");
    }
  }

  delete[] input;
  delete[] output;
  delete[] theta;
}

void TestGradientCorrectness(int width, int height) {
  int matrixSize = width * height;
  double *input = new double[matrixSize]();
  double *output = new double[matrixSize]();
  double *theta = new double[matrixSize]();

  int lower_bound = 0;
  int upper_bound = 256;

  std::uniform_int_distribution<int> unif(lower_bound, upper_bound);
  std::default_random_engine re;

  for (int i = 0; i < matrixSize; i++) {
    input[i] = unif(re);
    output[i] = 0.0;
  }

  // initialize arrays
  cv::Mat expected, grad_x, grad_y, absGradX, absGradY;
  cv::Mat src(height, width, CV_64F, input);

  // calculate magnitude
  cv::Sobel(src, grad_x, CV_64F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
  cv::Sobel(src, grad_y, CV_64F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

  cv::Mat magnitude, angle;
  cv::cartToPolar(grad_x, grad_y, magnitude, angle, false);

  for (int y = 0; y < angle.rows; y++) {
    for (int x = 0; x < angle.cols; x++) {
      double currentAngle = angle.at<double>(y, x);
      if (currentAngle >= CV_PI) {
        angle.at<double>(y, x) -= 2 * CV_PI;
      }
    }
  }

  Gradient(input, output, theta, width, height);

  for (int i = 0; i < matrixSize; i++) {
    if (std::abs(output[i] - magnitude.at<double>(i)) > 1e-4) {
      std::cout << "output[" << i << "] = " << output[i]
                << " expected: " << magnitude.at<double>(i) << "\n";
      std::cout << "width: " << width << " height: " << height << "\n";
      throw std::runtime_error("TestGradientCorrectness failed");
    }
  }

  delete[] input;
  delete[] output;
  delete[] theta;
}

int main(int argc, char *argv[]) {
  cv::setNumThreads(0);
  try {
    std::cout << "Running tests...\n";

    std::cout << "...Testing GradientSlow correctness...\n";
    TestGradientSlowCorrectness(3, 3);
    TestGradientSlowCorrectness(8, 8);
    TestGradientSlowCorrectness(32, 32);
    TestGradientSlowCorrectness(1024, 1024);
    std::cout << "GradientSlow correctness passed\n";

    std::cout << "...Benchmarking GradientSlow...\n";
    BenchmarkGradientSlow(8, 8);
    BenchmarkGradientSlow(16, 16);
    BenchmarkGradientSlow(32, 32);
    BenchmarkGradientSlow(64, 64);

    std::cout << "...Testing Gradient correctness...\n";
    TestGradientCorrectness(4, 4);
    TestGradientCorrectness(8, 8);
    TestGradientCorrectness(32, 32);
    TestGradientCorrectness(64, 64);
    std::cout << "Gradient correctness passed\n";

    std::cout << "...Benchmarking Gradient...\n";
    BenchmarkGradient(4, 4);
    BenchmarkGradient(8, 8);
    BenchmarkGradient(16, 16);
    BenchmarkGradient(32, 32);
    BenchmarkGradient(64, 64);

    std::cout << "All tests passed\n";
  } catch (const std::exception &err) {
    std::cerr << "[ERROR] " << err.what() << "\n";

    return -1;
  }

  return 0;
}
