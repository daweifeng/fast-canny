#include <cmath>
#include <immintrin.h>

void NonMaxSuppressionSlow(double *input, double *output, double *theta,
                           int kernalSize, int width, int height) {
  int padd = kernalSize / 2;
  double radianToDegree = 180.0 / M_PI;

  for (int i = padd; i < height - padd; i++) {
    for (int j = padd; j < width - padd; j++) {
      int idx = i * width + j;

      // Convert angle from radians to degrees
      double angle = theta[idx] * radianToDegree;

      // Normalize angle to [0,360)
      // After computing gradients with atan2 and adjusting them to [0,2π),
      // angle in degrees will be [0,360).
      // Canny uses angles in [0,180), so reduce any angle >180 by subtracting
      // 180.
      if (angle >= 180) {
        angle -= 180;
      }

      double q = 255.0;
      double r = 255.0;

      // Determine which pixels to compare (q and r) based on the angle range
      if ((0 <= angle && angle < 22.5) || (157.5 <= angle && angle < 180)) {
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
  double radianToDegree = 180 / M_PI;

  for (int i = padd; i < height - padd; i++) {
    for (int j = padd; j < width - padd; j += 4) {
      int idx = i * width + j;

      __m256d angle = _mm256_loadu_pd(&theta[idx]);
      angle = _mm256_mul_pd(angle, _mm256_set1_pd(radianToDegree));

      __m256d mask = _mm256_cmp_pd(angle, _mm256_setzero_pd(), _CMP_LT_OS);
      angle = _mm256_add_pd(angle, _mm256_and_pd(mask, _mm256_set1_pd(180.0)));

      __m256d q, r;

      q = _mm256_set1_pd(255.0);
      r = _mm256_set1_pd(255.0);

      __m256d mask1 = _mm256_and_pd(
          _mm256_cmp_pd(angle, _mm256_set1_pd(22.5), _CMP_GE_OS),
          _mm256_cmp_pd(angle, _mm256_set1_pd(157.5), _CMP_LT_OS));
      q = _mm256_blendv_pd(q, _mm256_loadu_pd(&input[idx + 1]), mask1);
      r = _mm256_blendv_pd(r, _mm256_loadu_pd(&input[idx - 1]), mask1);

      __m256d input_val = _mm256_loadu_pd(&input[idx]);
      __m256d result = _mm256_max_pd(_mm256_max_pd(input_val, q), r);

      __m256d output_val =
          _mm256_blendv_pd(_mm256_setzero_pd(), result,
                           _mm256_cmp_pd(input_val, result, _CMP_GE_OS));

      _mm256_storeu_pd(&output[idx], output_val);
    }
  }
}
