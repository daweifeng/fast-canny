/**
 * @brief Add Padding to a matrix with a given value
 */
#include <cstring>
#include <iostream>
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
