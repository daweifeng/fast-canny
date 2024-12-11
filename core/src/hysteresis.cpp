#include "hysteresis.h"
#include "padding.h"
#include <cassert>
#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <omp.h>
#include <queue>

void HysteresisSlow(double *input, double *output, int width, int height,
                    double lowThreshold, double highThreshold) {
  // Direction vectors for the 8-connected neighborhood
  const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
  const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

  // Initialize the edge map
  // for (int y = 0; y < height; y++) {
  //   for (int x = 0; x < width; x++) {
  //     int idx = y * width + x;
  //     double pixelValue = input[idx];

  //     if (pixelValue >= highThreshold) {
  //       // Mark as strong edge
  //       input[idx] = 255.0;
  //     } else if (pixelValue >= lowThreshold) {
  //       // Mark as weak edge
  //       input[idx] = 128.0;
  //     } else {
  //       // Suppress non-edges
  //       input[idx] = 0.0;
  //     }
  //   }
  // }

  int paddedWidth = width + 2;
  int paddedHeight = height + 2;

  double *paddedInput = new double[paddedWidth * paddedHeight];
  // Add 0 padding to the input matrix
  PadMatrix(input, paddedInput, width, height, 1, 0);

  bool changed;
  do {
    changed = false;

    for (int y = 1; y < paddedHeight; y++) {
      for (int x = 1; x < paddedWidth; x++) {
        int idx = y * paddedWidth + x;

        if (paddedInput[idx] == lowThreshold) {
          // Check 8-connected neighbors
          bool hasStrongNeighbor = false;
          for (int i = 0; i < 8; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            int nidx = ny * paddedWidth + nx;

            if (paddedInput[nidx] == highThreshold) {
              hasStrongNeighbor = true;
              break;
            }
          }

          if (hasStrongNeighbor) {
            paddedInput[idx] = highThreshold;
            changed = true;
          }
        }
      }
    }
  } while (changed);

  for (int i = 0; i < paddedWidth * paddedHeight; i++) {
    if (paddedInput[i] != highThreshold) {
      paddedInput[i] = 0.0;
    }
  }

  for (int y = 1; y < paddedHeight; y++) {
    for (int x = 1; x < paddedWidth; x++) {
      int idx = y * paddedWidth + x;
      int outputIdx = (y - 1) * width + (x - 1);

      output[outputIdx] = paddedInput[idx];
    }
  }
}

void HysteresisIteration(double *input, double *output, int width, int height,
                         double lowThreshold, double highThreshold) {
  // Threshold values
  const double STRONG_EDGE = highThreshold;
  const double WEAK_EDGE = lowThreshold;
  const double NON_EDGE = 0.0;

  // Define padded dimensions
  int paddedWidth = width + 2;
  int paddedHeight = height + 2;

  // Allocate aligned memory for padded input (32-byte alignment for AVX2)
  double *paddedInput =
      (double *)_mm_malloc(paddedWidth * paddedHeight * sizeof(double), 32);
  if (!paddedInput) {
    std::cerr << "Error: Memory allocation failed." << std::endl;
    return;
  }

  // Add 0 padding to the input matrix
  PadMatrix(input, paddedInput, width, height, 1, 0);

  // Define neighbor offsets based on paddedWidth
  const int numNeighbors = 8;
  int neighborOffsets[numNeighbors] = {
      -paddedWidth - 1, -paddedWidth, -paddedWidth + 1, -1, 1,
      paddedWidth - 1,  paddedWidth,  paddedWidth + 1};

  // SIMD constants
  const int simdWidth = 8; // Total pixels processed per iteration (unrolled)
  const int simdWidthPerVector = 4; // Pixels per SIMD vector (__m256d)
  __m256d strongEdgeValue = _mm256_set1_pd(STRONG_EDGE);
  __m256d weakEdgeValue = _mm256_set1_pd(WEAK_EDGE);

  bool changed;
  int count = 0;
  do {
    count++;
    changed = false;

#pragma omp parallel for schedule(static)
    for (int y = 1; y < paddedHeight - 1; y++) {
      int x;
      for (x = 1; x <= paddedWidth - 1 - simdWidth; x += simdWidth) {
        int idx = y * paddedWidth + x;

        // Load center pixels for low and high parts
        __m256d centerPixelsLo = _mm256_loadu_pd(&paddedInput[idx]);
        __m256d centerPixelsHi =
            _mm256_loadu_pd(&paddedInput[idx + simdWidthPerVector]);

        // Compare with weak edge value
        __m256d isWeakEdgeLo =
            _mm256_cmp_pd(centerPixelsLo, weakEdgeValue, _CMP_EQ_OQ);
        __m256d isWeakEdgeHi =
            _mm256_cmp_pd(centerPixelsHi, weakEdgeValue, _CMP_EQ_OQ);

        // Create masks
        int weakEdgeMaskLo = _mm256_movemask_pd(isWeakEdgeLo);
        int weakEdgeMaskHi = _mm256_movemask_pd(isWeakEdgeHi);

        if (weakEdgeMaskLo == 0 && weakEdgeMaskHi == 0) {
          // No weak edges in this group
          continue;
        }

        // Initialize promotion masks
        __m256d promoteMaskLo = _mm256_setzero_pd();
        __m256d promoteMaskHi = _mm256_setzero_pd();

        // Check all 8 neighbors
        for (int n = 0; n < numNeighbors; n++) {
          int neighborOffset = neighborOffsets[n];

          // Load neighbor pixels for low and high parts
          __m256d neighborPixelsLo =
              _mm256_loadu_pd(&paddedInput[idx + neighborOffset]);
          __m256d neighborPixelsHi = _mm256_loadu_pd(
              &paddedInput[idx + simdWidthPerVector + neighborOffset]);

          // Compare neighbor pixels with strong edge value
          __m256d isStrongNeighborLo =
              _mm256_cmp_pd(neighborPixelsLo, strongEdgeValue, _CMP_EQ_OQ);
          __m256d isStrongNeighborHi =
              _mm256_cmp_pd(neighborPixelsHi, strongEdgeValue, _CMP_EQ_OQ);

          // Accumulate promotion masks
          promoteMaskLo = _mm256_or_pd(promoteMaskLo, isStrongNeighborLo);
          promoteMaskHi = _mm256_or_pd(promoteMaskHi, isStrongNeighborHi);
        }

        // Determine final promotion masks for weak edges
        __m256d finalPromotionMaskLo =
            _mm256_and_pd(promoteMaskLo, isWeakEdgeLo);
        __m256d finalPromotionMaskHi =
            _mm256_and_pd(promoteMaskHi, isWeakEdgeHi);

        // Update center pixels: promote to strong edge where applicable
        centerPixelsLo = _mm256_blendv_pd(centerPixelsLo, strongEdgeValue,
                                          finalPromotionMaskLo);
        centerPixelsHi = _mm256_blendv_pd(centerPixelsHi, strongEdgeValue,
                                          finalPromotionMaskHi);

        // Store updated center pixels
        _mm256_storeu_pd(&paddedInput[idx], centerPixelsLo);
        _mm256_storeu_pd(&paddedInput[idx + simdWidthPerVector],
                         centerPixelsHi);

        // Check if any promotions occurred
        int promotionMaskLo = _mm256_movemask_pd(finalPromotionMaskLo);
        int promotionMaskHi = _mm256_movemask_pd(finalPromotionMaskHi);
        if (promotionMaskLo != 0 || promotionMaskHi != 0) {
          changed = true;
        }
      }

      // Handle remaining pixels at the end of the row
      // for (; x < paddedWidth - 1; x++) {
      //   int idx = y * paddedWidth + x;
      //   double centerPixel = paddedInput[idx];

      //   if (centerPixel == WEAK_EDGE) { // Weak edge
      //     bool hasStrongNeighbor = false;
      //     for (int n = 0; n < numNeighbors; n++) {
      //       int neighborOffset = neighborOffsets[n];
      //       double neighborPixel = paddedInput[idx + neighborOffset];
      //       if (neighborPixel == STRONG_EDGE) {
      //         hasStrongNeighbor = true;
      //         break;
      //       }
      //     }
      //     if (hasStrongNeighbor) {
      //       paddedInput[idx] = STRONG_EDGE;
      //       changed = true;
      //     }
      //   }
      // }
    }
  } while (changed);

// Suppress remaining weak edges
#pragma omp parallel for schedule(static)
  for (int i = 0; i < paddedWidth * paddedHeight; i++) {
    if (paddedInput[i] != STRONG_EDGE) {
      paddedInput[i] = NON_EDGE;
    }
  }

  // Copy data back to output array (excluding padding)
#pragma omp parallel for schedule(static)
  for (int y = 1; y < paddedHeight - 1; y++) {
    int paddedIdx = y * paddedWidth + 1;
    int outputIdx = (y - 1) * width;
    std::memcpy(&output[outputIdx], &paddedInput[paddedIdx],
                width * sizeof(double));
  }

  // Free allocated memory
  _mm_free(paddedInput);
}

void HysteresisQueue(double *input, double *output, int width, int height,
                     double lowThreshold, double highThreshold) {
  // Direction vectors for the 8-connected neighborhood
  const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
  const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

  std::queue<std::pair<int, int>> edgeQueue;

  // Initialize the edge map and queue
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int idx = y * width + x;
      double pixelValue = input[idx];

      if (pixelValue >= highThreshold) {
        // Mark as strong edge
        input[idx] = 255.0;
        edgeQueue.push(std::make_pair(x, y));
      } else if (pixelValue >= lowThreshold) {
        // Mark as weak edge
        input[idx] = 128.0;
      } else {
        // Suppress non-edges
        input[idx] = 0.0;
      }
    }
  }

  // Perform BFS to track edges
  while (!edgeQueue.empty()) {
    int x = edgeQueue.front().first;
    int y = edgeQueue.front().second;
    edgeQueue.pop();

    // Check all 8 neighbors
    for (int i = 0; i < 8; i++) {
      int nx = x + dx[i];
      int ny = y + dy[i];

      // Check if neighbor is within image bounds
      if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
        int nidx = ny * width + nx;
        double neighborValue = input[nidx];

        if (neighborValue == 128.0) {
          // Promote weak edge to strong edge
          input[nidx] = 255.0;
          edgeQueue.push(std::make_pair(nx, ny));
        }
      }
    }
  }

  // Suppress remaining weak edges
  for (int i = 0; i < width * height; i++) {
    if (input[i] != 255.0) {
      input[i] = 0.0;
    }
  }

  // Copy input to output using SIMD
  int n = width * height;
  int i = 0;

  for (; i <= n - 16; i += 16) {
    __m256d data0 = _mm256_loadu_pd(&input[i]);
    __m256d data1 = _mm256_loadu_pd(&input[i + 4]);
    __m256d data2 = _mm256_loadu_pd(&input[i + 8]);
    __m256d data3 = _mm256_loadu_pd(&input[i + 12]);

    _mm256_storeu_pd(&output[i], data0);
    _mm256_storeu_pd(&output[i + 4], data1);
    _mm256_storeu_pd(&output[i + 8], data2);
    _mm256_storeu_pd(&output[i + 12], data3);
  }

  // Handle any remaining elements
  for (; i < n; i++) {
    output[i] = input[i];
  }
}

void Hysteresis(double *input, double *output, int width, int height,
                double lowThreshold, double highThreshold) {
  HysteresisIteration(input, output, width, height, lowThreshold,
                      highThreshold);
};
