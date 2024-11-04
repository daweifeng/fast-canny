#include "gaussian_filter.h"
#include <cmath>
#include <iostream>

void GaussianFilter(double *input, double *output, int width, int height,
                    double sigma) {
  // Implementation
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

      // Apply the kernel
      double sum = 0.0;

      for (int k = -halfSize; k <= halfSize; k++) {
        for (int l = -halfSize; l <= halfSize; l++) {
          int x = i + k;
          int y = j + l;
          double pixel;

          // Handle borders with zero padding
          if (x < 0 || x >= height || y < 0 || y >= width) {
            pixel = 0.0;
          } else {
            pixel = input[x * width + y];
          }

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
