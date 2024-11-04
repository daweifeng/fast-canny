#include "gaussian_filter.h"
#include <cmath>
#include <cstring>
#include <immintrin.h>

/**
 * @brief Add Padding to a matrix with a given value
 */
void PadMatrix(double *input, double *output, int width, int height,
               int padSize, int padValue) {
  int paddedWidth = width + 2 * padSize;
  int paddedHeight = height + 2 * padSize;

  std::memset(output, padValue, paddedWidth * paddedHeight * sizeof(double));

  // TODO: Should we use SIMD here?
  for (int i = 0; i < height; i++) {
    std::memcpy(output + (i + padSize) * paddedWidth + padSize,
                input + i * width, width * sizeof(double));
  }
}

/**
 * @brief Apply a Gaussian filter to an image using SIMD
 */
void GaussianFilter(double *input, double *output, int kernalSize, int width,
                    int height, double sigma) {
  int halfSize = kernalSize / 2;
  double *kernel = new double[kernalSize * kernalSize];

  // TODO: We can try SIMD here
  GenerateGaussianKernel(kernel, kernalSize, kernalSize, sigma);

  double *paddedInput =
      new double[(width + 2 * halfSize) * (height + 2 * halfSize)];
  // Add 0 padding to the input matrix
  PadMatrix(input, paddedInput, width, height, halfSize, 0);

  int paddedWidth = width + 2 * halfSize;
  int paddedHeight = height + 2 * halfSize;

  // TODO: Consider cache aware optimization
  for (int i = halfSize; i < height + halfSize; i += 4) {
    for (int j = halfSize; j < width + halfSize; j += 4) {
      __m256d sum1 = _mm256_setzero_pd();
      __m256d sum2 = _mm256_setzero_pd();
      __m256d sum3 = _mm256_setzero_pd();
      __m256d sum4 = _mm256_setzero_pd();

      for (int k = -halfSize; k <= halfSize; k++) {
        for (int l = -halfSize; l <= halfSize; l++) {
          __m256d pixels1 =
              _mm256_loadu_pd(&paddedInput[(i + k) * paddedWidth + (j + l)]);
          __m256d pixels2 = _mm256_loadu_pd(
              &paddedInput[(i + k + 1) * paddedWidth + (j + l)]);
          __m256d pixels3 = _mm256_loadu_pd(
              &paddedInput[(i + k + 2) * paddedWidth + (j + l)]);
          __m256d pixels4 = _mm256_loadu_pd(
              &paddedInput[(i + k + 3) * paddedWidth + (j + l)]);
          __m256d kernelValue = _mm256_set1_pd(
              kernel[(k + halfSize) * kernalSize + (l + halfSize)]);
          sum1 = _mm256_fmadd_pd(pixels1, kernelValue, sum1);
          sum2 = _mm256_fmadd_pd(pixels2, kernelValue, sum2);
          sum3 = _mm256_fmadd_pd(pixels3, kernelValue, sum3);
          sum4 = _mm256_fmadd_pd(pixels4, kernelValue, sum4);
        }
      }

      _mm256_storeu_pd(&output[(i - halfSize) * width + (j - halfSize)], sum1);
      _mm256_storeu_pd(&output[(i + 1 - halfSize) * width + (j - halfSize)],
                       sum2);
      _mm256_storeu_pd(&output[(i + 2 - halfSize) * width + (j - halfSize)],
                       sum3);
      _mm256_storeu_pd(&output[(i + 3 - halfSize) * width + (j - halfSize)],
                       sum4);
    }
  }

  delete[] paddedInput;
  delete[] kernel;
};

/**
 * @brief Apply a Gaussian filter to an image. This function is a slow
 * implementation of the Gaussian filter. It is used to compare the performance
 */
void GaussianFilterSlow(double *input, double *output, int kernalSize,
                        int width, int height, double sigma) {

  int halfSize = kernalSize / 2;
  double *kernel = new double[kernalSize * kernalSize];

  GenerateGaussianKernel(kernel, kernalSize, kernalSize, sigma);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      double sum = 0.0;

      // Apply the kernel by iterating over the kernel elements
      for (int k = -halfSize; k <= halfSize; k++) {
        for (int l = -halfSize; l <= halfSize; l++) {
          int x = i + k;
          int y = j + l;
          double pixel;

          // Handle borders with zero padding
          // We can also use PadMatrix() here
          if (x < 0 || x >= height || y < 0 || y >= width) {
            pixel = 0.0;
          } else {
            pixel = input[x * width + y];
          }

          // Calculated the weighted sum
          sum += pixel * kernel[(k + halfSize) * kernalSize + (l + halfSize)];
        }
      }

      output[i * width + j] = sum;
    }
  }

  delete[] kernel;
};

/**
 * @brief Generate a Gaussian kernel
 */
void GenerateGaussianKernel(double *kernel, int width, int height,
                            double sigma) {

  int halfWidth = width / 2;
  double sum = 0.0;

  for (int x = -halfWidth; x <= halfWidth; x++) {
    for (int y = -halfWidth; y <= halfWidth; y++) {
      // https://en.wikipedia.org/wiki/Gaussian_filter
      double value = std::exp(-(x * x + y * y) / (2 * sigma * sigma)) /
                     (2 * M_PI * sigma * sigma);
      kernel[(x + halfWidth) * width + (y + halfWidth)] = value;
      sum += value;
    }
  }

  for (int i = 0; i < width * height; i++) {
    kernel[i] /= sum;
  }
};
