#include <algorithm>
#include <cmath>
#include <cstdio>
#include <immintrin.h>

void double_threshold_slow(double *input, double *output, int width, int height,
                           double low_thres = 50, double high_thres = 100) {
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      int idx = i * width + j;
      if (input[idx] >= high_thres) {
        output[idx] = high_thres;
      } else if (input[idx] >= low_thres && input[idx] <= high_thres) {
        output[idx] = low_thres;
      } else {
        output[idx] = 0;
      }
    }
  }
}

void double_threshold(double *input, double *output, int width, int height,
                      double low_thres = 50, double high_thres = 100) {
  int size = width * height;

  int i = 0;

  for (; i <= size - 4; i += 4) {
    __m256d input_vals = _mm256_loadu_pd(&input[i]);

    __m256d low_vals = _mm256_set1_pd(low_thres);
    __m256d high_vals = _mm256_set1_pd(high_thres);

    __m256d high_mask = _mm256_cmp_pd(input_vals, high_vals, _CMP_GE_OS);
    __m256d low_mask = _mm256_andnot_pd(
        high_mask, _mm256_cmp_pd(input_vals, low_vals, _CMP_GE_OS));

    __m256d result =
        _mm256_blendv_pd(_mm256_setzero_pd(), high_vals, high_mask);

    result = _mm256_blendv_pd(result, low_vals, low_mask);

    _mm256_storeu_pd(&output[i], result);
  }

  for (; i < size; i++) {
    if (input[i] >= high_thres) {
      output[i] = high_thres;
    } else if (input[i] >= low_thres) {
      output[i] = low_thres;
    } else {
      output[i] = 0;
    }
  }
}
