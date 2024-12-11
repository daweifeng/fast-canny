#include "gradient.h"
#include "padding.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <immintrin.h>

#define GAUSSIAN_KERNEL_SIZE 3

using namespace std;

const double sobel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const double sobel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

__m256d simd_approx_atan(__m256d x) {
  // Constants for approximation (coefficients would be based on a polynomial
  // fit)
  const __m256d c1 = _mm256_set1_pd(0.9997878412794807); // Example coefficient
  const __m256d c2 = _mm256_set1_pd(-0.3258083974640975);
  const __m256d c3 = _mm256_set1_pd(0.1555786518463281);
  const __m256d c4 = _mm256_set1_pd(-0.04432655554792128);

  __m256d x2 = _mm256_mul_pd(x, x);
  __m256d result = _mm256_fmadd_pd(c4, x2, c3);
  result = _mm256_fmadd_pd(result, x2, c2);
  result = _mm256_fmadd_pd(result, x2, c1);
  result = _mm256_mul_pd(result, x);

  return result;
}

__m256d simd_atan2(__m256d y, __m256d x) {
  // Constants for adjusting results based on the quadrant
  const __m256d pi = _mm256_set1_pd(M_PI);
  const __m256d half_pi = _mm256_set1_pd(M_PI_2);
  const __m256d zero = _mm256_setzero_pd();

  // Calculate y / x
  __m256d tangent = _mm256_div_pd(y, x);

  // Approximate atan using polynomial expansion or similar methods
  __m256d atan_result =
      simd_approx_atan(tangent); // Substitute with approximate atan

  // Check for quadrant adjustments
  __m256d x_lt_zero = _mm256_cmp_pd(x, zero, _CMP_LT_OS); // x < 0
  __m256d y_ge_zero = _mm256_cmp_pd(y, zero, _CMP_GE_OS); // y >= 0

  // Adjust atan_result based on the quadrant of (x, y)
  __m256d angle_offset =
      _mm256_blendv_pd(pi, -pi, y_ge_zero); // +/- pi based on y sign
  __m256d adjusted_result =
      _mm256_add_pd(atan_result, _mm256_and_pd(x_lt_zero, angle_offset));

  return adjusted_result;
};

/**
 * @brief Apply a Sobel filter to an image using SIMD
 */
void Gradient(const double *input, double *output, double *theta, int width,
              int height) {
  // Assuming width and height are power of 2
  assert(width > 0 && (width & (width - 1)) == 0);
  assert(height > 0 && (height & (height - 1)) == 0);

  const int padd = 1;
  int halfSize = (width * height) / 2;

  double *paddedInput =
      new double[(width + 2 * halfSize) * (height + 2 * halfSize)];
  // Add 0 padding to the input matrix
  PadMatrix(input, paddedInput, width, height, halfSize, 0);
  int paddedWidth = width + 2 * halfSize;
  int paddedHeight = height + 2 * halfSize;

  __m256d sum1_x, sum2_x, sum3_x, sum4_x, sum5_x, sum6_x, sum7_x, sum8_x,
      sum9_x, sum10_x;
  __m256d sum1_y, sum2_y, sum3_y, sum4_y, sum5_y, sum6_y, sum7_y, sum8_y,
      sum9_y, sum10_y;
  __m256d pixels1, pixels2, pixels3, pixels4, pixels5;
  __m256d grad1, grad2, grad3, grad4, grad5, grad6, grad7, grad8, grad9, grad10;
  __m256d dir1, dir2, dir3, dir4, dir5, dir6, dir7, dir8, dir9, dir10;

  // Loop over the image
  for (int idx = 0; idx <= width * height - 40; idx += 40) {
    // Process 4 elements at a time (AVX2 for double)
    sum1_x = _mm256_setzero_pd();
    sum2_x = _mm256_setzero_pd();
    sum3_x = _mm256_setzero_pd();
    sum4_x = _mm256_setzero_pd();
    sum5_x = _mm256_setzero_pd();
    sum6_x = _mm256_setzero_pd();
    sum7_x = _mm256_setzero_pd();
    sum8_x = _mm256_setzero_pd();
    sum9_x = _mm256_setzero_pd();
    sum10_x = _mm256_setzero_pd();
    sum1_y = _mm256_setzero_pd();
    sum2_y = _mm256_setzero_pd();
    sum3_y = _mm256_setzero_pd();
    sum4_y = _mm256_setzero_pd();
    sum5_y = _mm256_setzero_pd();
    sum6_y = _mm256_setzero_pd();
    sum7_y = _mm256_setzero_pd();
    sum8_y = _mm256_setzero_pd();
    sum9_y = _mm256_setzero_pd();
    sum10_y = _mm256_setzero_pd();

    pixels1 = _mm256_setzero_pd();
    pixels2 = _mm256_setzero_pd();
    pixels3 = _mm256_setzero_pd();
    pixels4 = _mm256_setzero_pd();
    pixels5 = _mm256_setzero_pd();

    // Apply Sobel kernels
    for (int k = -padd; k <= padd; k++) {
      for (int l = -padd; l <= padd; l++) {

        __m256d kernel_x_value = _mm256_set1_pd(sobel_x[k + padd][l + padd]);
        __m256d kernel_y_value = _mm256_set1_pd(sobel_y[k + padd][l + padd]);

        pixels1 = _mm256_loadu_pd(
            &paddedInput[((idx) / width + halfSize + k) * paddedWidth +
                         ((idx) % width + halfSize + l)]);
        pixels2 = _mm256_loadu_pd(
            &paddedInput[((idx + 4) / width + halfSize + k) * paddedWidth +
                         ((idx + 4) % width + halfSize + l)]);
        pixels3 = _mm256_loadu_pd(
            &paddedInput[((idx + 8) / width + halfSize + k) * paddedWidth +
                         ((idx + 8) % width + halfSize + l)]);
        pixels4 = _mm256_loadu_pd(
            &paddedInput[((idx + 12) / width + halfSize + k) * paddedWidth +
                         ((idx + 12) % width + halfSize + l)]);
        pixels5 = _mm256_loadu_pd(
            &paddedInput[((idx + 16) / width + halfSize + k) * paddedWidth +
                         ((idx + 16) % width + halfSize + l)]);

        sum1_x = _mm256_fmadd_pd(pixels1, kernel_x_value, sum1_x);
        sum1_y = _mm256_fmadd_pd(pixels1, kernel_y_value, sum1_y);

        sum2_x = _mm256_fmadd_pd(pixels2, kernel_x_value, sum2_x);
        sum2_y = _mm256_fmadd_pd(pixels2, kernel_y_value, sum2_y);

        sum3_x = _mm256_fmadd_pd(pixels3, kernel_x_value, sum3_x);
        sum3_y = _mm256_fmadd_pd(pixels3, kernel_y_value, sum3_y);

        sum4_x = _mm256_fmadd_pd(pixels4, kernel_x_value, sum4_x);
        sum4_y = _mm256_fmadd_pd(pixels4, kernel_y_value, sum4_y);

        sum5_x = _mm256_fmadd_pd(pixels5, kernel_x_value, sum5_x);
        sum5_y = _mm256_fmadd_pd(pixels5, kernel_y_value, sum5_y);

        pixels1 = _mm256_loadu_pd(
            &paddedInput[((idx + 20) / width + halfSize + k) * paddedWidth +
                         ((idx + 20) % width + halfSize + l)]);
        pixels2 = _mm256_loadu_pd(
            &paddedInput[((idx + 24) / width + halfSize + k) * paddedWidth +
                         ((idx + 24) % width + halfSize + l)]);
        pixels3 = _mm256_loadu_pd(
            &paddedInput[((idx + 28) / width + halfSize + k) * paddedWidth +
                         ((idx + 28) % width + halfSize + l)]);
        pixels4 = _mm256_loadu_pd(
            &paddedInput[((idx + 32) / width + halfSize + k) * paddedWidth +
                         ((idx + 32) % width + halfSize + l)]);
        pixels5 = _mm256_loadu_pd(
            &paddedInput[((idx + 36) / width + halfSize + k) * paddedWidth +
                         ((idx + 36) % width + halfSize + l)]);

        sum6_x = _mm256_fmadd_pd(pixels1, kernel_x_value, sum6_x);
        sum6_y = _mm256_fmadd_pd(pixels1, kernel_y_value, sum6_y);

        sum7_x = _mm256_fmadd_pd(pixels2, kernel_x_value, sum7_x);
        sum7_y = _mm256_fmadd_pd(pixels2, kernel_y_value, sum7_y);

        sum8_x = _mm256_fmadd_pd(pixels3, kernel_x_value, sum8_x);
        sum8_y = _mm256_fmadd_pd(pixels3, kernel_y_value, sum8_y);

        sum9_x = _mm256_fmadd_pd(pixels4, kernel_x_value, sum9_x);
        sum9_y = _mm256_fmadd_pd(pixels4, kernel_y_value, sum9_y);

        sum10_x = _mm256_fmadd_pd(pixels5, kernel_x_value, sum10_x);
        sum10_y = _mm256_fmadd_pd(pixels5, kernel_y_value, sum10_y);
      }
    }

    // Compute magnitude and direction (grad_x/2 + grad_y/2)
    grad1 = _mm256_add_pd(_mm256_mul_pd(sum1_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum1_y, _mm256_set1_pd(0.5)));
    grad2 = _mm256_add_pd(_mm256_mul_pd(sum2_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum2_y, _mm256_set1_pd(0.5)));
    grad3 = _mm256_add_pd(_mm256_mul_pd(sum3_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum3_y, _mm256_set1_pd(0.5)));
    grad4 = _mm256_add_pd(_mm256_mul_pd(sum4_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum4_y, _mm256_set1_pd(0.5)));
    grad5 = _mm256_add_pd(_mm256_mul_pd(sum5_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum5_y, _mm256_set1_pd(0.5)));
    grad6 = _mm256_add_pd(_mm256_mul_pd(sum6_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum6_y, _mm256_set1_pd(0.5)));
    grad7 = _mm256_add_pd(_mm256_mul_pd(sum7_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum7_y, _mm256_set1_pd(0.5)));
    grad8 = _mm256_add_pd(_mm256_mul_pd(sum8_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum8_y, _mm256_set1_pd(0.5)));
    grad9 = _mm256_add_pd(_mm256_mul_pd(sum9_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum9_y, _mm256_set1_pd(0.5)));
    grad10 = _mm256_add_pd(_mm256_mul_pd(sum10_x, _mm256_set1_pd(0.5)),
                           _mm256_mul_pd(sum10_y, _mm256_set1_pd(0.5)));

    dir1 = simd_atan2(sum1_x, sum1_y);
    dir2 = simd_atan2(sum2_x, sum2_y);
    dir3 = simd_atan2(sum3_x, sum3_y);
    dir4 = simd_atan2(sum4_x, sum4_y);
    dir5 = simd_atan2(sum5_x, sum5_y);
    dir6 = simd_atan2(sum6_x, sum6_y);
    dir7 = simd_atan2(sum7_x, sum7_y);
    dir8 = simd_atan2(sum8_x, sum8_y);
    dir9 = simd_atan2(sum9_x, sum9_y);
    dir10 = simd_atan2(sum10_x, sum10_y);

    // Store results (for grad_magnitude and theta)
    _mm256_storeu_pd(&output[((idx) / width) * width + ((idx) % width)], grad1);
    _mm256_storeu_pd(&output[((idx + 4) / width) * width + ((idx + 4) % width)],
                     grad2);
    _mm256_storeu_pd(&output[((idx + 8) / width) * width + ((idx + 8) % width)],
                     grad3);
    _mm256_storeu_pd(
        &output[((idx + 12) / width) * width + ((idx + 12) % width)], grad4);
    _mm256_storeu_pd(
        &output[((idx + 16) / width) * width + ((idx + 16) % width)], grad5);
    _mm256_storeu_pd(
        &output[((idx + 20) / width) * width + ((idx + 20) % width)], grad6);
    _mm256_storeu_pd(
        &output[((idx + 24) / width) * width + ((idx + 24) % width)], grad7);
    _mm256_storeu_pd(
        &output[((idx + 28) / width) * width + ((idx + 28) % width)], grad8);
    _mm256_storeu_pd(
        &output[((idx + 32) / width) * width + ((idx + 32) % width)], grad9);
    _mm256_storeu_pd(
        &output[((idx + 36) / width) * width + ((idx + 36) % width)], grad10);
    _mm256_storeu_pd(&theta[((idx) / width) * width + ((idx) % width)], dir1);
    _mm256_storeu_pd(&theta[((idx + 4) / width) * width + ((idx + 4) % width)],
                     dir2);
    _mm256_storeu_pd(&theta[((idx + 8) / width) * width + ((idx + 8) % width)],
                     dir3);
    _mm256_storeu_pd(
        &theta[((idx + 12) / width) * width + ((idx + 12) % width)], dir4);
    _mm256_storeu_pd(
        &theta[((idx + 16) / width) * width + ((idx + 16) % width)], dir5);
    _mm256_storeu_pd(
        &theta[((idx + 20) / width) * width + ((idx + 20) % width)], dir6);
    _mm256_storeu_pd(
        &theta[((idx + 24) / width) * width + ((idx + 24) % width)], dir7);
    _mm256_storeu_pd(
        &theta[((idx + 28) / width) * width + ((idx + 28) % width)], dir8);
    _mm256_storeu_pd(
        &theta[((idx + 32) / width) * width + ((idx + 32) % width)], dir9);
    _mm256_storeu_pd(
        &theta[((idx + 36) / width) * width + ((idx + 36) % width)], dir10);
  }
  for (int idx = (width * height / 40) * 40; idx < width * height; idx += 4) {
    sum1_x = _mm256_setzero_pd();
    sum1_y = _mm256_setzero_pd();

    for (int k = -padd; k <= padd; k++) {
      for (int l = -padd; l <= padd; l++) {
        __m256d kernel_x_value = _mm256_set1_pd(sobel_x[k + padd][l + padd]);
        __m256d kernel_y_value = _mm256_set1_pd(sobel_y[k + padd][l + padd]);

        pixels1 = _mm256_loadu_pd(
            &paddedInput[((idx) / width + halfSize + k) * paddedWidth +
                         ((idx) % width + halfSize + l)]);

        sum1_x = _mm256_fmadd_pd(pixels1, kernel_x_value, sum1_x);
        sum1_y = _mm256_fmadd_pd(pixels1, kernel_y_value, sum1_y);
      }
    }
    grad1 = _mm256_add_pd(_mm256_mul_pd(sum1_x, _mm256_set1_pd(0.5)),
                          _mm256_mul_pd(sum1_y, _mm256_set1_pd(0.5)));
    dir1 = simd_atan2(sum1_x, sum1_y);

    _mm256_storeu_pd(&output[((idx) / width) * width + ((idx) % width)], grad1);
    _mm256_storeu_pd(&theta[((idx) / width) * width + ((idx) % width)], dir1);
  }
  delete[] paddedInput;
}

/**
 * @brief Apply a Sobel filter to an image. This function is a slow
 * implementation of the Sobel filter. It is used to compare the performance
 */
void GradientSlow(const double *input, double *output, double *theta, int width,
                  int height) {
  int padd = 1;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      double grad_x = 0.0;
      double grad_y = 0.0;

      for (int k_i = -padd; k_i <= padd; k_i++) {
        for (int k_j = -padd; k_j <= padd; k_j++) {
          int ni = i + k_i;
          int nj = j + k_j;

          if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
            int img_index = ni * width + nj;
            grad_x += input[img_index] * sobel_x[k_i + padd][k_j + padd];
            grad_y += input[img_index] * sobel_y[k_i + padd][k_j + padd];
          }
        }
      }

      int sobel_index = i * width + j;
      double magnitude = sqrt(grad_x * grad_x + grad_y * grad_y);
      double angle = atan2(grad_y, grad_x);

      if (angle < 0) {
        angle += 2 * M_PI; // Adjust angle to be in [0, 2π)
      }

      output[sobel_index] = magnitude;
      theta[sobel_index] = angle;
    }
  }
};
