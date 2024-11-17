#include "hysteresis.h"
#include "padding.h"
#include <cassert>
#include <immintrin.h>
#include <iostream>
#include <queue>

void HysteresisSlow(double *input, double *output, int width, int height,
                    double lowThreshold, double highThreshold) {
  // Direction vectors for the 8-connected neighborhood
  const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
  const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

  // Initialize the edge map
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int idx = y * width + x;
      double pixelValue = input[idx];

      if (pixelValue >= highThreshold) {
        // Mark as strong edge
        input[idx] = 255.0;
      } else if (pixelValue >= lowThreshold) {
        // Mark as weak edge
        input[idx] = 128.0;
      } else {
        // Suppress non-edges
        input[idx] = 0.0;
      }
    }
  }

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

        if (paddedInput[idx] == 128.0) {
          // Check 8-connected neighbors
          bool hasStrongNeighbor = false;
          for (int i = 0; i < 8; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            int nidx = ny * paddedWidth + nx;

            if (paddedInput[nidx] == 255.0) {
              hasStrongNeighbor = true;
              break;
            }
          }

          if (hasStrongNeighbor) {
            paddedInput[idx] = 255.0;
            changed = true;
          }
        }
      }
    }
  } while (changed);

  for (int i = 0; i < paddedWidth * paddedHeight; i++) {
    if (paddedInput[i] != 255.0) {
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
  // Direction vectors for the 8-connected neighborhood
  const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
  const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

  // Initialize the edge map
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int idx = y * width + x;
      double pixelValue = input[idx];

      if (pixelValue >= highThreshold) {
        // Mark as strong edge
        input[idx] = 255.0;
      } else if (pixelValue >= lowThreshold) {
        // Mark as weak edge
        input[idx] = 128.0;
      } else {
        // Suppress non-edges
        input[idx] = 0.0;
      }
    }
  }

  int paddedWidth = width + 2;
  int paddedHeight = height + 2;

  double *paddedInput = new double[paddedWidth * paddedHeight];
  // Add 0 padding to the input matrix
  PadMatrix(input, paddedInput, width, height, 1, 0);

  bool changed;
  do {
    changed = false;

    const int simdWidth = 4; // Number of pixels processed simultaneously

    for (int y = 1; y < paddedHeight - 1; y++) {
      for (int x = 1; x < paddedWidth - 1; x += simdWidth) {
        int idx = y * paddedWidth + x;

        // Load 4 pixels
        __m256d currentPixels = _mm256_load_pd(&paddedInput[idx]);

        // Compare with 128.0 (weak edge)
        __m256d weakEdgeValue = _mm256_set1_pd(128.0);
        __m256d cmpWeakEdges =
            _mm256_cmp_pd(currentPixels, weakEdgeValue, _CMP_EQ_OQ);

        // Create a mask of weak edges
        int mask = _mm256_movemask_pd(cmpWeakEdges);

        if (mask == 0) {
          // No weak edges in this group, continue to next iteration
          continue;
        }

        // For weak edges, check neighbors
        for (int i = 0; i < simdWidth; i++) {
          if ((mask & (1 << i)) == 0) {
            // Not a weak edge, skip
            continue;
          }

          int xi = x + i;
          int idx_i = y * paddedWidth + xi;
          bool hasStrongNeighbor = false;

          // Check 8-connected neighbors
          for (int n = 0; n < 8; n++) {
            int nx = xi + dx[n];
            int ny = y + dy[n];
            int nidx = ny * paddedWidth + nx;

            if (paddedInput[nidx] == 255.0) {
              hasStrongNeighbor = true;
              break;
            }
          }

          if (hasStrongNeighbor) {
            paddedInput[idx_i] = 255.0;
            changed = true;
          }
        }
      }
    }

  } while (changed);

  // Suppress remaining weak edges
  for (int i = 0; i < paddedWidth * paddedHeight; i++) {
    if (paddedInput[i] != 255.0) {
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
  HysteresisQueue(input, output, width, height, lowThreshold, highThreshold);
};
