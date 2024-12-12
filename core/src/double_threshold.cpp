#include <algorithm>
#include <cmath>
#include <cstdio>
#include <immintrin.h>

void DoubleThresholdSlow(double *input, double *output, int width, int height,
                         double low_thres = 50, double high_thres = 100) {
  const double STRONG_EDGE = high_thres;
  const double WEAK_EDGE = low_thres;
  const double NON_EDGE = 0.0;

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      int idx = i * width + j;
      if (input[idx] >= high_thres) {
        output[idx] = STRONG_EDGE;
      } else if (input[idx] >= low_thres && input[idx] <= high_thres) {
        output[idx] = WEAK_EDGE;
      } else {
        output[idx] = 0;
      }
    }
  }
}

void DoubleThreshold(double *input, double *output, int width, int height,
                     double low_thres = 50, double high_thres = 100) {
  int size = width * height;

  __m256d low_vals = _mm256_set1_pd(low_thres);
  __m256d high_vals = _mm256_set1_pd(high_thres);
  
  #pragma omp parallel for schedule(static)
  for (int i = 0; i <= size - 12; i += 12) {
    __m256d input_vals_0 = _mm256_loadu_pd(&input[i]);
    __m256d input_vals_1 = _mm256_loadu_pd(&input[i + 4]);
    __m256d input_vals_2 = _mm256_loadu_pd(&input[i + 8]);

    __m256d high_mask_0 = _mm256_cmp_pd(input_vals_0, high_vals, _CMP_GE_OS);
    __m256d high_mask_1 = _mm256_cmp_pd(input_vals_1, high_vals, _CMP_GE_OS);
    __m256d high_mask_2 = _mm256_cmp_pd(input_vals_2, high_vals, _CMP_GE_OS);
    __m256d low_mask_0 = _mm256_andnot_pd(
        high_mask_0, _mm256_cmp_pd(input_vals_0, low_vals, _CMP_GE_OS));
    __m256d low_mask_1 = _mm256_andnot_pd(
        high_mask_1, _mm256_cmp_pd(input_vals_1, low_vals, _CMP_GE_OS));
    __m256d low_mask_2 = _mm256_andnot_pd(
        high_mask_2, _mm256_cmp_pd(input_vals_2, low_vals, _CMP_GE_OS));

    __m256d result_0 =
        _mm256_blendv_pd(_mm256_setzero_pd(), high_vals, high_mask_0);
    __m256d result_1 =
        _mm256_blendv_pd(_mm256_setzero_pd(), high_vals, high_mask_1);
    __m256d result_2 =
        _mm256_blendv_pd(_mm256_setzero_pd(), high_vals, high_mask_2);

    result_0 = _mm256_blendv_pd(result_0, low_vals, low_mask_0);
    result_1 = _mm256_blendv_pd(result_1, low_vals, low_mask_1);
    result_2 = _mm256_blendv_pd(result_2, low_vals, low_mask_2);

    _mm256_storeu_pd(&output[i], result_0);
    _mm256_storeu_pd(&output[i + 4], result_1);
    _mm256_storeu_pd(&output[i + 8], result_2);
  }

  #pragma omp parallel for schedule(static)
  for (int i = (size / 12) * 12; i < size; ++i) {
    if (input[i] >= high_thres) {
      output[i] = high_thres;
    } else if (input[i] >= low_thres) {
      output[i] = low_thres;
    } else {
      output[i] = 0;
    }
  }
}
