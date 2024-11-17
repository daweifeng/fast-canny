
#include <hysteresis.h>
#include <iostream>
#include <stdexcept>

#define MAX_FREQ 3.4
#define BASE_FREQ 2.4

static inline unsigned long long rdtsc(void) {
  unsigned hi, lo;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

void TestHysteresisFilledWithEdges(int width, int height) {
  int size = width * height;
  double *input = new double[size];
  double *output = new double[(width + 1) * (height + 1)];
  int low = 50;
  int high = 100;

  for (int i = 0; i < size; i++) {
    input[i] = 50;
  }

  input[0] = 100;

  HysteresisQueue(input, output, width, height, low, high);

  for (int i = 0; i < size; i++) {
    if (output[i] != 255) {
      std::cout << "Invalid value at index " << i << ", expected " << 255
                << ", get " << output[i] << "\n";
      throw std::runtime_error("Invalid hysteresis result");
    }
  }
}

void TestHysteresisOneEdge(int width, int height) {
  int size = width * height;
  double *input = new double[size];
  double *output = new double[(width + 1) * (height + 1)];
  int low = 50;
  int high = 100;

  for (int i = 0; i < size; i++) {
    input[i] = 0;
  }

  input[0] = high;
  input[1] = low;
  input[2] = low;
  input[3] = low;

  HysteresisQueue(input, output, width, height, low, high);

  if (output[0] != 255 || output[1] != 255 || output[2] != 255 ||
      output[3] != 255) {
    throw std::runtime_error("Edge not connected");
  }

  for (int i = 4; i < size; i++) {
    if (output[i] != 0) {
      std::cout << "Invalid value at index " << i << ", expected " << 255
                << ", get " << output[i] << "\n";
      throw std::runtime_error("Invalid hysteresis result");
    }
  }
}

void BenchmarkHysteresisFilledWithEdges(int width, int height) {
  int size = width * height;
  double *input = new double[size];
  double *output = new double[(width + 1) * (height + 1)];
  int low = 50;
  int high = 100;
  int repeat = 1000;
  unsigned long long st;
  unsigned long long et;
  unsigned long long total = 0;

  for (int i = 0; i != repeat; i++) {
    for (int i = 0; i < size; i++) {
      input[i] = low;
    }

    input[0] = high;

    st = rdtsc();

    Hysteresis(input, output, width, height, low, high);

    et = rdtsc();

    total += (et - st);
  }

  unsigned long long kernalFLOPSPS = width * height * 8 + width * height;

  std::cout << "Benchmarking matrix size: " << width << "x" << height << "\n";
  std::cout << "RDTSC Cycles Taken for Hysteresis: " << total << "\n";
  std::cout << "FLOPS Per Cycle for Hysteresis: "
            << repeat * kernalFLOPSPS / (total * MAX_FREQ / BASE_FREQ) << "\n";
}

int main() {
  try {
    TestHysteresisFilledWithEdges(8, 8);
    TestHysteresisOneEdge(8, 8);

    BenchmarkHysteresisFilledWithEdges(64, 64);

    std::cout << "All tests passed" << "\n";
  } catch (const std::exception &err) {
    std::cerr << "[ERROR] " << err.what() << "\n";

    return -1;
  }
}
