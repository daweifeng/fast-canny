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

void BenchmarkDoubleThreshold(int width, int height) {
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

    // Generate a random input matrix
    for (int i = 0; i < matrixSize; i++) {
        input[i] = unif(re);
        output[i] = 0.0;
    }

    double lowThreshold = 50;  // Lower threshold
    double highThreshold = 150; // Higher threshold

    cv::Mat src(height, width, CV_64F, input);
    cv::Mat strongEdgesMat(height, width, CV_64F);
    cv::Mat weakEdgesMat(height, width, CV_64F);

    // Verify correctness of the DoubleThreshold function
    DoubleThreshold(input, output, width, height, lowThreshold, highThreshold);

    cv::threshold(src, strongEdgesMat, highThreshold, 255, cv::THRESH_BINARY);
    cv::threshold(src, weakEdgesMat, lowThreshold, 255, cv::THRESH_BINARY);
    weakEdgesMat -= strongEdgesMat;

    for (int i = 0; i < matrixSize; i++) {
        if (std::abs(output[i] - strongEdgesMat.at<double>(i)) > 1e-6) {
            std::cout << "output[" << i << "] = " << output[i]
                      << " expected: " << strongEdgesMat.at<double>(i) << "\n";
            throw std::runtime_error("BenchmarkDoubleThreshold failed: incorrect output from DoubleThreshold");
        }
    }

    // Benchmark custom DoubleThreshold function
    for (int i = 0; i != repeat; ++i) {
        st = rdtsc();
        DoubleThreshold(input, output, width, height, lowThreshold, highThreshold);
        et = rdtsc();
        total += (et - st);
    }

    // Benchmark OpenCV double threshold implementation
    for (int i = 0; i != repeat; ++i) {
        st = rdtsc();
        cv::threshold(src, strongEdgesMat, highThreshold, 255, cv::THRESH_BINARY);
        cv::threshold(src, weakEdgesMat, lowThreshold, 255, cv::THRESH_BINARY);
        weakEdgesMat -= strongEdgesMat;
        et = rdtsc();
        referenceTotal += (et - st);
    }

    unsigned long long createThresholdFLOPSPS = (4 + 6 + 1 + 1 + 1) * 9;
    unsigned long long kernelFLOPSPS = 2 * 9 * width * height + createThresholdFLOPSPS;

    std::cout << "Benchmarking matrix size: " << width << "x" << height << "\n";
    std::cout << "RDTSC Cycles Taken for DoubleThreshold: " << total << "\n";
    std::cout << "RDTSC Cycles Taken for cv::threshold: " << referenceTotal << "\n";
    std::cout << "FLOPS Per Cycle for DoubleThreshold: "
              << repeat * kernelFLOPSPS / (total * MAX_FREQ / BASE_FREQ) << "\n";
    std::cout << "FLOPS Per Cycle for cv::threshold: "
              << repeat * kernelFLOPSPS / (referenceTotal * MAX_FREQ / BASE_FREQ) << "\n";

    delete[] input;
    delete[] output;
}



void TestDoubleThresholdCorrectness(int width, int height) {
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

    double lowThreshold = 50;
    double highThreshold = 150;

    cv::Mat src(height, width, CV_64F, input);
    cv::Mat strongEdgesMat(height, width, CV_64F);
    cv::Mat weakEdgesMat(height, width, CV_64F);

    cv::threshold(src, strongEdgesMat, highThreshold, 255, cv::THRESH_BINARY);
    cv::threshold(src, weakEdgesMat, lowThreshold, 255, cv::THRESH_BINARY);
    weakEdgesMat -= strongEdgesMat;

    DoubleThreshold(input, output, width, height, lowThreshold, highThreshold);

    for (int i = 0; i < matrixSize; i++) {
        if (std::abs(output[i] - strongEdgesMat.at<double>(i)) > 1e-6) {
            std::cout << "output[" << i << "] = " << output[i]
                      << " expected: " << strongEdgesMat.at<double>(i) << "\n";
            std::cout << "width: " << width << " height: " << height << "\n";
            throw std::runtime_error("TestDoubleThresholdCorrectness failed");
        }
    }

    std::cout << "TestDoubleThresholdCorrectness passed successfully for width: " 
              << width << " and height: " << height << ".\n";

    delete[] input;
    delete[] output;
}


int main(int argc, char *argv[]) {
    cv::setNumThreads(0);
    try {
        std::cout << "Running tests...\n";

        std::cout << "...Testing DoubleThreshold correctness...\n";
        TestDoubleThresholdCorrectness(3, 3);
        TestDoubleThresholdCorrectness(8, 8);
        TestDoubleThresholdCorrectness(32, 32);
        TestDoubleThresholdCorrectness(64, 64);
        TestDoubleThresholdCorrectness(128, 128);
        TestDoubleThresholdCorrectness(256, 256);
        TestDoubleThresholdCorrectness(512, 512);
        TestDoubleThresholdCorrectness(1024, 1024);
        std::cout << "DoubleThreshold correctness passed\n";

        std::cout << "...Benchmarking DoubleThreshold...\n";
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

