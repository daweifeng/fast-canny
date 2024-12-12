#include <cmath>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

void NonMaxSuppressionSlow(double *input, double *output, double *theta,
                           int kernalSize, int width, int height) {
  int padd = kernalSize / 2;
  double radianToDegree = 180.0 / M_PI;

  for (int i = padd; i < height - padd; i++) {
    for (int j = padd; j < width - padd; j++) {
      int idx = i * width + j;

      // Convert angle from radians to degrees
      double angle = theta[idx] * radianToDegree;

      // Normalize the angle to be in the range [0, 180)
      if (angle < 0) {
        angle += 180;
      }

      double q, r;
      // Determine which pixels to compare (q and r) based on the angle range
      if ((0 <= angle && angle < 22.5) || (157.5 <= angle && angle <= 180)) {
        // Horizontal direction
        q = input[idx + 1];
        r = input[idx - 1];
      } else if (22.5 <= angle && angle < 67.5) {
        // Diagonal (top-left to bottom-right)
        q = input[(i + 1) * width + (j - 1)];
        r = input[(i - 1) * width + (j + 1)];
      } else if (67.5 <= angle && angle < 112.5) {
        // Vertical direction
        q = input[(i + 1) * width + j];
        r = input[(i - 1) * width + j];
      } else if (112.5 <= angle && angle < 157.5) {
        // Diagonal (bottom-left to top-right)
        q = input[(i - 1) * width + (j - 1)];
        r = input[(i + 1) * width + (j + 1)];
      } else {
        // std::cerr << "Error: Invalid angle value: " << angle << std::endl;
      }

      // Keep the pixel value if it is a local maximum; otherwise, suppress it
      if (input[idx] >= q && input[idx] >= r) {
        output[idx] = input[idx];
      } else {
        output[idx] = 0.0;
      }
    }
  }
}

void NonMaxSuppression(double *input, double *output, double *theta,
                       int kernalSize, int width, int height) {
  int padd = kernalSize / 2;
  const double radianToDegree = 180.0 / M_PI;
  const __m256d vec_radianToDegree = _mm256_set1_pd(radianToDegree);
  const __m256d vec_180 = _mm256_set1_pd(180.0);
  const __m256d vec_22_5 = _mm256_set1_pd(22.5);
  const __m256d vec_67_5 = _mm256_set1_pd(67.5);
  const __m256d vec_112_5 = _mm256_set1_pd(112.5);
  const __m256d vec_157_5 = _mm256_set1_pd(157.5);
  const __m256d vec_zero = _mm256_set1_pd(0.0);

#pragma omp parallel for
  for (int i = padd; i < height - padd; i++) {
    int j = padd;
    for (; j <= width - padd - 4; j += 4) { // Process 4 pixels at a time
      int idx = i * width + j;

      // Load theta values
      __m256d theta_vals = _mm256_loadu_pd(&theta[idx]);

      // Convert to degrees
      __m256d angles = _mm256_mul_pd(theta_vals, vec_radianToDegree);

      // Normalize angles to [0, 180)
      __m256d mask_neg = _mm256_cmp_pd(angles, vec_zero, _CMP_LT_OS);
      __m256d angles_norm =
          _mm256_blendv_pd(angles, _mm256_add_pd(angles, vec_180), mask_neg);

      // Determine direction masks
      __m256d mask_horizontal = _mm256_or_pd(
          _mm256_and_pd(_mm256_cmp_pd(angles_norm, vec_zero, _CMP_GE_OS),
                        _mm256_cmp_pd(angles_norm, vec_22_5, _CMP_LT_OS)),
          _mm256_and_pd(_mm256_cmp_pd(angles_norm, vec_157_5, _CMP_GE_OS),
                        _mm256_cmp_pd(angles_norm, vec_180, _CMP_LE_OS)));

      __m256d mask_diagonal1 =
          _mm256_and_pd(_mm256_cmp_pd(angles_norm, vec_22_5, _CMP_GE_OS),
                        _mm256_cmp_pd(angles_norm, vec_67_5, _CMP_LT_OS));

      __m256d mask_vertical =
          _mm256_and_pd(_mm256_cmp_pd(angles_norm, vec_67_5, _CMP_GE_OS),
                        _mm256_cmp_pd(angles_norm, vec_112_5, _CMP_LT_OS));

      __m256d mask_diagonal2 =
          _mm256_and_pd(_mm256_cmp_pd(angles_norm, vec_112_5, _CMP_GE_OS),
                        _mm256_cmp_pd(angles_norm, vec_157_5, _CMP_LT_OS));

      // Load input values for q and r based on direction
      // Horizontal
      __m256d q_horizontal = _mm256_loadu_pd(&input[idx + 1]);
      __m256d r_horizontal = _mm256_loadu_pd(&input[idx - 1]);

      // Diagonal1 (top-left to bottom-right)
      __m256d q_diagonal1 = _mm256_loadu_pd(&input[(i + 1) * width + (j - 1)]);
      __m256d r_diagonal1 = _mm256_loadu_pd(&input[(i - 1) * width + (j + 1)]);

      // Vertical
      __m256d q_vertical = _mm256_loadu_pd(&input[(i + 1) * width + j]);
      __m256d r_vertical = _mm256_loadu_pd(&input[(i - 1) * width + j]);

      // Diagonal2 (bottom-left to top-right)
      __m256d q_diagonal2 = _mm256_loadu_pd(&input[(i - 1) * width + (j - 1)]);
      __m256d r_diagonal2 = _mm256_loadu_pd(&input[(i + 1) * width + (j + 1)]);

      // Initialize q and r to zero
      __m256d q = _mm256_setzero_pd();
      __m256d r = _mm256_setzero_pd();

      // Select q based on masks
      q = _mm256_blendv_pd(q, q_horizontal, mask_horizontal);
      q = _mm256_blendv_pd(q, q_diagonal1, mask_diagonal1);
      q = _mm256_blendv_pd(q, q_vertical, mask_vertical);
      q = _mm256_blendv_pd(q, q_diagonal2, mask_diagonal2);

      // Select r based on masks
      r = _mm256_blendv_pd(r, r_horizontal, mask_horizontal);
      r = _mm256_blendv_pd(r, r_diagonal1, mask_diagonal1);
      r = _mm256_blendv_pd(r, r_vertical, mask_vertical);
      r = _mm256_blendv_pd(r, r_diagonal2, mask_diagonal2);

      // Load input pixel values
      __m256d input_vals = _mm256_loadu_pd(&input[idx]);

      // Compare input >= q and input >= r
      __m256d cmp1 = _mm256_cmp_pd(input_vals, q, _CMP_GE_OS);
      __m256d cmp2 = _mm256_cmp_pd(input_vals, r, _CMP_GE_OS);
      __m256d mask_keep = _mm256_and_pd(cmp1, cmp2);

      // Set output: input if mask_keep, else 0.0
      __m256d output_vals = _mm256_blendv_pd(vec_zero, input_vals, mask_keep);

      // Store the results
      _mm256_storeu_pd(&output[idx], output_vals);
    }

    // Handle remaining pixels (tail processing)
    for (; j < width - padd; j++) {
      int idx = i * width + j;

      // Convert angle from radians to degrees
      double angle = theta[idx] * radianToDegree;

      // Normalize the angle to be in the range [0, 180)
      if (angle < 0) {
        angle += 180;
      }

      double q, r;
      // Determine which pixels to compare (q and r) based on the angle range
      if ((0 <= angle && angle < 22.5) || (157.5 <= angle && angle <= 180)) {
        // Horizontal direction
        q = input[idx + 1];
        r = input[idx - 1];
      } else if (22.5 <= angle && angle < 67.5) {
        // Diagonal (top-left to bottom-right)
        q = input[(i + 1) * width + (j - 1)];
        r = input[(i - 1) * width + (j + 1)];
      } else if (67.5 <= angle && angle < 112.5) {
        // Vertical direction
        q = input[(i + 1) * width + j];
        r = input[(i - 1) * width + j];
      } else if (112.5 <= angle && angle < 157.5) {
        // Diagonal (bottom-left to top-right)
        q = input[(i - 1) * width + (j - 1)];
        r = input[(i + 1) * width + (j + 1)];
      } else {
        // std::cerr << "Error: Invalid angle value: " << angle << std::endl;
        q = r = 0.0;
      }

      // Keep the pixel value if it is a local maximum; otherwise, suppress it
      output[idx] = (input[idx] >= q && input[idx] >= r) ? input[idx] : 0.0;
    }
  }
}
